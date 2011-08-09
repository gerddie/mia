/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/errormacro.hh>
#include <map>
#include <boost/algorithm/minmax_element.hpp>

#include <dicom/dicom4mia.hh>

#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmdata/dcdict.h>
#include <dcmtk/dcmdata/dcdicent.h>
#include <dcmtk/dcmnet/dimse.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmjpeg/djdecode.h>

#include <mia/2d/2dimageio.hh>

NS_MIA_BEGIN

using namespace std;

typedef struct {
	const char *const skey;
	DcmTagKey key;
	bool ismetadata;
	bool required;
} SLookupInit;

const SLookupInit lookup_init[] = {
	{IDStudyDescription, DCM_StudyDescription, false, true},
	{IDSeriesDescription, DCM_SeriesDescription, false, true},
	{IDModality, DCM_Modality, false, true},
	{IDSeriesNumber, DCM_SeriesNumber, false, true},
	{IDPatientPosition, DCM_PatientPosition, false, false},
	{IDAcquisitionDate, DCM_AcquisitionDate, false, true},
	{IDAcquisitionNumber, DCM_AcquisitionNumber, false, true},
	{IDSmallestImagePixelValue, DCM_SmallestImagePixelValue, false, false},
	{IDLargestImagePixelValue, DCM_LargestImagePixelValue, false, false},
	{IDInstanceNumber, DCM_InstanceNumber, false, true},
	{IDStudyID, DCM_StudyID, false, true},
	{IDImageType, DCM_ImageType, false, true},
	{IDSliceLocation, DCM_SliceLocation, false, true},
	{IDPatientOrientation, DCM_PatientOrientation, false, false},
	{IDMediaStorageSOPClassUID, DCM_MediaStorageSOPClassUID, true, true},
	{IDSOPClassUID, DCM_SOPClassUID, false, false},
	{IDProtocolName, DCM_ProtocolName, false, false},
	{IDTestValue, DcmTagKey(), false, false},
	{NULL, DcmTagKey(), false, false}
};

class LookupMap {
public:
	typedef pair<DcmTagKey, bool> Key;

	LookupMap();
	Key find(const string& k) const;
	bool has_key(const string& k) const;

	static const LookupMap& instance();

private:
	typedef map<string, Key > Map;
	Map m_keylookup;
};


struct CDicomReaderData {
	DcmFileFormat dcm;
	OFCondition status;

	CDicomReaderData();
	CDicomReaderData(const DcmFileFormat& dcm); 
	~CDicomReaderData(); 

	Uint16 getUint16(const DcmTagKey &tagKey, bool required);

	string getAttribute(const string& key, bool required);

	template <typename T>
	void getPixelData(T2DImage<T>& image);

	template <typename T>
	void getPixelData_LittleEndianExplicitTransfer(T2DImage<T>& image);


	C2DFVector getPixelSize();
};


CDicomReader::CDicomReader(const char *filename):
	impl(new CDicomReaderData()),
	m_filename(filename)
{
	impl->status = impl->dcm.loadFile( filename );
}

CDicomReader::~CDicomReader()
{
	delete impl;
}

bool CDicomReader::good() const
{
	return impl->status.good();
}

int CDicomReader::rows() const
{
	return impl->getUint16(DCM_Rows, true);
}

int CDicomReader::cols() const
{
	return impl->getUint16(DCM_Columns, true);
}

C2DBounds CDicomReader::image_size()const
{
	return C2DBounds(impl->getUint16(DCM_Columns, true),
			 impl->getUint16(DCM_Rows, true));

}

C2DFVector CDicomReader::get_pixel_size() const
{
	return impl->getPixelSize();
}

int CDicomReader::samples_per_pixel() const
{
	return impl->getUint16(DCM_SamplesPerPixel, true);
}

int CDicomReader::bits_allocated() const
{
	return impl->getUint16(DCM_BitsAllocated, true);
}

int CDicomReader::bits_used() const
{
	return impl->getUint16(DCM_BitsStored, true);
}

string CDicomReader::get_attribute(const std::string& name, bool required)const
{
	return impl->getAttribute(name, required);
}


void CDicomReader::add_attribute(C2DImage& image, const char *key, bool required) const
{
	string value = get_attribute(key, required);
	if (!value.empty())
		image.set_attribute(key, value);
}

template <typename T>
P2DImage CDicomReader::load_image()const
{
	C2DBounds size = this->image_size();
	cvdebug() << "load slice of size " << size << "\n";
	T2DImage<T> *result = new T2DImage<T>(size);
	P2DImage presult(result);

	// get pixel data
	impl->getPixelData(*result);

	result->set_pixel_size(get_pixel_size());


	const SLookupInit *attr_table = lookup_init;
	while (attr_table->skey) {
		// copy some attributes
		this->add_attribute(*result, attr_table->skey, attr_table->required);
		++attr_table;
	}

	return presult;
}

P2DImage CDicomReader::get_image() const
{
	if (samples_per_pixel() != 1) {
		THROW(invalid_argument, "CDicomReader: '"<< m_filename
		      <<"' Image has more then one color channel");
	}
	int bbpa = bits_allocated();
	int bbp  = bits_used();
	if (bbp > bbpa) {
		THROW(invalid_argument, "CDicomReader: '"<< m_filename
		      <<"' Bogus image - more bits per pixel used then allocated");
	}

	if (bbpa == 16)
		return load_image<unsigned short>();

	THROW(invalid_argument, "CDicomReader: '"<< m_filename
	      << "' don't support " << bbp << " bits per pixel (yet)");
}


CDicomReaderData::CDicomReaderData()
{
	DJDecoderRegistration::registerCodecs();
}

CDicomReaderData::CDicomReaderData(const DcmFileFormat& _dcm):
	dcm(_dcm)
{
	DJDecoderRegistration::registerCodecs();
}

CDicomReaderData::~CDicomReaderData()
{
	DJDecoderRegistration::cleanup();
}


Uint16 CDicomReaderData::getUint16(const DcmTagKey &tagKey, bool required)
{
	Uint16 value;
	OFCondition success = dcm.getDataset()->findAndGetUint16(tagKey, value);

	if (success.bad() && required){
		THROW(runtime_error, "CDicom2DImageIOPlugin: unable to get value for '"
		      << tagKey << " ':" << status.text());
	}
	return value;
}

string CDicomReaderData::getAttribute(const string& key, bool required)
{
	LookupMap::Key k = LookupMap::instance().find(key);

	OFString value;
	OFCondition success = k.second  ?
		dcm.getMetaInfo()->findAndGetOFStringArray(k.first, value):
		dcm.getDataset()->findAndGetOFStringArray(k.first, value);

	if (success.bad() && required) {
		THROW(runtime_error, "DICOM read: Required value '" << key << "' not found");
	}

	return string(value.data());
}

template <typename T>
void CDicomReaderData::getPixelData_LittleEndianExplicitTransfer(T2DImage<T>& image)
{
	OFCondition status = dcm.loadAllDataIntoMemory();
	if (status.bad()) {
		THROW(runtime_error, "DICOM: error loading pixel data:"<< status.text());
	}

	const Uint16 *values;
	long unsigned int count;
	OFCondition cnd = dcm.getDataset()->findAndGetUint16Array(DCM_PixelData, values, &count, false);
	if (cnd.good()) {
		if (image.size() != count) {
			THROW(runtime_error, "bogus file, expect " << image.size() << " pixels, "
			      << "but got data for " << count << " pixels");
		}
		copy(values, values+count, image.begin());
	}else {
		THROW(runtime_error, "DICOM: required value PixelData:" << status.text());
	}
}

template <typename T>
void CDicomReaderData::getPixelData(T2DImage<T>& image)
{
	OFString of_transfer_syntax;
	OFCondition success = dcm.getMetaInfo()->findAndGetOFString(DCM_TransferSyntaxUID, of_transfer_syntax);
	if (success.bad()) {
		THROW(runtime_error, "Unable to determine transfer syntax");
	}
	dcm.getDataset()->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
	
	if (!dcm.getDataset()->canWriteXfer(EXS_LittleEndianExplicit)) {
		string transfer_syntax(of_transfer_syntax.data());
		THROW(runtime_error, "DICOM: Unsupported data encoding '" <<  transfer_syntax << "'");
	}

	getPixelData_LittleEndianExplicitTransfer(image);
}

C2DFVector CDicomReaderData::getPixelSize()
{
	OFString help;
	OFCondition success = dcm.getDataset()->findAndGetOFString(DCM_PixelSpacing, help, 0);
	if (success.bad()) {
		THROW(runtime_error, "Required attribute 'PixelSpacing' not found");
	}
	C2DFVector result;

	istringstream swidth(help.data());
	swidth >> result.x;

	success = dcm.getDataset()->findAndGetOFString(DCM_PixelSpacing, help, 1);
	if (success.bad()) {
		THROW(runtime_error, "Required attribute 'PixelSpacing' not found");
	}
	istringstream sheight(help.data());
	sheight >> result.y;

	return result;
}

struct CDicomWriterData {
	CDicomWriterData(const C2DImage& image);
	bool write(const char *filename);
	DcmFileFormat dcm;
private:
	void setValueUint16(const DcmTagKey& key, int value);
	void setValueString(const string& key, const string& value);
	void setValueString(const DcmTagKey& key, const string& value, bool meta);
	void setSize(const C2DBounds& size);
	void setPixelSpacing(const C2DFVector& value);
	void setValueStringIfKeyExists(const CAttributeMap::value_type& value);
	friend struct CDicomImageSaver;
};

struct CDicomImageSaver: public TFilter<bool> {
	CDicomImageSaver(CDicomWriterData *_parent):
		parent(_parent)
	{
	}

	template <typename T>
	CDicomImageSaver::result_type operator ()(const T2DImage<T>& image)const;
private:

	CDicomWriterData *parent;
};


//add lowest and higest value

template <typename T>
struct pixel_trait {
	enum { AllocSize = 8 * sizeof(T)};
	enum { UseSize = 8 * sizeof(T)};
	enum { supported = 0 };
	static void copy_pixel_data(DcmDataset& /*dataset*/, const T2DImage<T>& /*image*/) {
		assert(!"There is no code here");
	}
};

template <>
struct pixel_trait<unsigned short> {
	enum { AllocSize = 16};
	enum { UseSize = 16};
	enum { supported = 1};
	static void copy_pixel_data(DcmDataset& dataset, const C2DUSImage& image) {
		dataset.putAndInsertUint16Array(DCM_PixelData, (const Uint16*)&image(0,0), image.size());
	}
};

template <typename T>
CDicomImageSaver::result_type CDicomImageSaver::operator ()(const T2DImage<T>& image)const
{
	if (!pixel_trait<T>::supported) {
		THROW(invalid_argument, "DICOM: unsupporrted pixel type");
	}

	parent->setValueUint16(DCM_BitsAllocated, pixel_trait<T>::AllocSize);
	parent->setValueUint16(DCM_BitsStored, pixel_trait<T>::UseSize);

	pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator> minmax =
		boost::minmax_element(image.begin(), image.end());


	parent->setValueUint16(DCM_SmallestImagePixelValue, *minmax.first);
	parent->setValueUint16(DCM_LargestImagePixelValue, *minmax.second);

	pixel_trait<T>::copy_pixel_data(*parent->dcm.getDataset(), image);
	return true;
}

CDicomWriterData::CDicomWriterData(const C2DImage& image)
{
	setValueUint16(DCM_SamplesPerPixel, 1);
	setSize(image.get_size());
	setPixelSpacing(image.get_pixel_size());

	for(CAttributeMap::const_iterator i = image.get_attribute_list()->begin();
	    i != image.get_attribute_list()->end(); ++i)
		setValueStringIfKeyExists(*i);

	if (!image.has_attribute(IDMediaStorageSOPClassUID))
		setValueString(IDMediaStorageSOPClassUID, "1.2.840.10008.5.1.4.1.1.4");

	dcm.getMetaInfo()->putAndInsertString(DCM_TransferSyntaxUID,
					      UID_LittleEndianImplicitTransferSyntax);

	CDicomImageSaver saver(this);
	mia::filter(saver, image);

}

void CDicomWriterData::setValueUint16(const DcmTagKey& key, int value)
{
	dcm.getDataset()->putAndInsertUint16(key, value);
}

void CDicomWriterData::setValueString(const DcmTagKey& key, const string& value, bool meta)
{
	if (meta)
		dcm.getMetaInfo()->putAndInsertString(key, value.c_str());
	else
		dcm.getDataset()->putAndInsertString(key, value.c_str());
}

void CDicomWriterData::setValueString(const string& key, const string& value)
{
	LookupMap::Key k = LookupMap::instance().find(key);
	setValueString(k.first, value, k.second);
}

void CDicomWriterData::setSize(const C2DBounds& size)
{
	setValueUint16(DCM_Rows, size.y);
	setValueUint16(DCM_Columns, size.x);
}

void CDicomWriterData::setPixelSpacing(const C2DFVector& value)
{
	stringstream pixelspacing;
	pixelspacing << value.x << "\\" << value.y;
	setValueString(DCM_PixelSpacing, pixelspacing.str(), false);
}

void CDicomWriterData::setValueStringIfKeyExists(const CAttributeMap::value_type& value)
{
	if (LookupMap::instance().has_key(value.first))
		setValueString(value.first, value.second->as_string());
}

bool CDicomWriterData::write(const char *filename)
{
	OFCondition status = dcm.saveFile(filename, EXS_LittleEndianImplicit,
					  EET_UndefinedLength,EGL_withoutGL);
	return status.good();
}


CDicomWriter::CDicomWriter(const C2DImage& image):
	impl(new CDicomWriterData(image))
{
}

CDicomWriter::~CDicomWriter()
{
	delete impl;
}


bool CDicomWriter::write(const char *filename) const
{
	return impl->write(filename);
}


LookupMap::LookupMap()
{
	const SLookupInit *s = lookup_init;
	while (s->skey) {
		m_keylookup[s->skey] = pair<DcmTagKey,bool>(s->key,s->ismetadata);
		++s;
	}
}

LookupMap::Key LookupMap::find(const string& k) const
{
	Map::const_iterator i = m_keylookup.find(k);
	// keys _must_ be in thelist, otherwise we have a problem
	assert(i != m_keylookup.end());
	return i->second;
}

bool LookupMap::has_key(const string& k) const
{
	return m_keylookup.find(k) != m_keylookup.end();
}

const LookupMap& LookupMap::instance()
{
	static LookupMap me;
	return me;
}

	// this is here only for testing
CDicomReader::CDicomReader(struct CDicomReaderData *yeah):
	impl(yeah),
	m_filename("internal")
{
}

CDicomReader EXPORT_DICOM ugly_trick_writer_dcm_to_reader_dcm(CDicomWriter& writer)
{
	return CDicomReader(new CDicomReaderData(writer.impl->dcm));
}

EXPORT_DICOM const char * IDMediaStorageSOPClassUID= "MediaStorageSOPClassUID";
EXPORT_DICOM const char * IDStudyDescription = "StudyDescription";
EXPORT_DICOM const char * IDSamplesPerPixel = "IDSamplesPerPixel";
EXPORT_DICOM const char * IDSeriesDescription = "SeriesDescription";

EXPORT_DICOM const char * IDTestValue = "TestValue";
EXPORT_DICOM const char * IDTransferSyntaxUID = "TransferSyntaxUID";
EXPORT_DICOM const char * IDSOPClassUID = "SOPClassUID";


NS_MIA_END
