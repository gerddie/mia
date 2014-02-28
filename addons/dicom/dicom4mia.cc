/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/core/errormacro.hh>
#include <map>
#include <algorithm>

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

#include <mia/2d/imageio.hh>

NS_MIA_BEGIN

using namespace std;

enum ETagRequirement {
	tr_no, 
	tr_yes, 
	tr_yes_defaulted
}; 

typedef struct {
	const char *const skey;
	DcmTagKey key;
	bool ismetadata;
	ETagRequirement required;
	const char *const default_value;
} SLookupInit;

const SLookupInit lookup_init[] = {
	{IDStudyDescription, DCM_StudyDescription, false, tr_no, NULL},
	{IDSeriesDescription, DCM_SeriesDescription, false, tr_no, NULL},
	{IDModality, DCM_Modality, false, tr_yes, NULL},
	{IDSeriesNumber, DCM_SeriesNumber, false, tr_yes_defaulted, "-1"},
	{IDPatientPosition, DCM_PatientPosition, false, tr_no, NULL},
	{IDAcquisitionDate, DCM_AcquisitionDate, false, tr_no, NULL},
	{IDAcquisitionNumber, DCM_AcquisitionNumber, false, tr_yes_defaulted, "-1"},
	{IDSmallestImagePixelValue, DCM_SmallestImagePixelValue, false, tr_no, NULL},
	{IDLargestImagePixelValue, DCM_LargestImagePixelValue, false, tr_no, NULL},
	{IDInstanceNumber, DCM_InstanceNumber, false, tr_yes_defaulted, "-1"},
	{IDStudyID, DCM_StudyID, false, tr_no, NULL},
	{IDImageType, DCM_ImageType, false, tr_no, NULL},
	{IDSliceLocation, DCM_SliceLocation, false, tr_no, NULL},
	{IDSliceThickness, DCM_SliceThickness, false, tr_no, NULL},
	{IDPatientOrientation, DCM_PatientOrientation, false, tr_no, NULL},
	{IDMediaStorageSOPClassUID, DCM_MediaStorageSOPClassUID, true, tr_no, NULL},
	{IDSOPClassUID, DCM_SOPClassUID, false, tr_no, NULL},
	{IDProtocolName, DCM_ProtocolName, false, tr_no, NULL},
	{IDTestValue, DcmTagKey(), false, tr_no, NULL},
	{IDPatientPosition, DCM_PatientPosition, false, tr_no, NULL}, 
	{IDAcquisitionTime, DCM_AcquisitionTime, false, tr_no, NULL}, 

	{IDPositionerPrimaryAngle, DCM_PositionerPrimaryAngle, false, tr_no, NULL}, 
	{IDPositionerSecondaryAngle, DCM_PositionerSecondaryAngle, false, tr_no, NULL}, 
	{IDImagerPixelSpacing, DCM_ImagerPixelSpacing, false, tr_no, NULL},
	{IDDistanceSourceToDetector, DCM_DistanceSourceToDetector, false, tr_no, NULL},
	{IDDistanceSourceToPatient, DCM_DistanceSourceToPatient, false, tr_no, NULL},
	{IDPixelIntensityRelationship, DCM_PixelIntensityRelationship, false, tr_no, NULL},
	{IDPositionerPrimaryAngleIncrement, DCM_PositionerPrimaryAngleIncrement, false, tr_no, NULL},
	{IDPositionerSecondaryAngleIncrement, DCM_PositionerSecondaryAngleIncrement, false, tr_no, NULL},
	
	{NULL, DcmTagKey(), false, tr_no, NULL}
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

	Uint16 getUint16(const DcmTagKey &tagKey, bool required, Uint16 default_value = 0);
	Sint32 getSint32(const DcmTagKey &tagKey, bool required, Sint32 default_value = 0);
	float getFloat32(const DcmTagKey &tagKey, bool required, float default_value = 0);
	float getFloat64(const DcmTagKey &tagKey, bool required, double default_value = 0);

	string getAttribute(const string& key, bool required, const char *default_value = "");
	string getAttribute(const DcmTagKey &tagKey, bool required, const char *default_value = "");

	void add_attribute(CAttributedData& image, const char *key, ETagRequirement required, const char *default_value); 

	template <typename I>
	void getPixelData(I out_begin, size_t size);

	template <typename I>
	void getPixelData_LittleEndianExplicitTransfer(I out_begin, size_t size);


	C2DFVector getPixelSize();

	C2DFVector getImagePixelSpacing(); 


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

bool CDicomReader::has_3dimage() const
{
	TRACE_FUNCTION; 
	return get_number_of_frames() > 1; 
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

C3DFVector CDicomReader::get_voxel_size(bool warn) const
{
	auto size2d = impl->getPixelSize();
	auto thinkness = impl->getFloat64(DCM_SpacingBetweenSlices, false,0); 
	if (thinkness <= 0.0 && warn) 
		cvwarn() << "DICOM: 3D multiframe image doesn't provide a spacing between slices, most likely "
			 << "the input file does not constitute a volume"; 

	return C3DFVector(size2d.x, size2d.y, thinkness); 
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



template <typename T>
P2DImage CDicomReader::load_image()const
{
	C2DBounds size = this->image_size();
	cvdebug() << "load slice of size " << size << "\n";
	T2DImage<T> *result = new T2DImage<T>(size);
	P2DImage presult(result);

	// get pixel data
	impl->getPixelData(result->begin(), result->size());

	result->set_pixel_size(get_pixel_size());


	const SLookupInit *attr_table = lookup_init;
	while (attr_table->skey) {
		// copy some attributes
		impl->add_attribute(*result, attr_table->skey, attr_table->required, attr_table->default_value);
		++attr_table;
	}
	
	return presult;
}

int CDicomReader::get_number_of_frames() const
{
	TRACE_FUNCTION; 
	return impl->getSint32(DCM_NumberOfFrames, false, 1);
}

template <typename T>
P3DImage CDicomReader::load_image3d()const
{
	C3DBounds size(this->image_size().x, this->image_size().y, this->get_number_of_frames());  
	cvdebug() << "load slice of size " << size << "\n";
	T3DImage<T> *result = new T3DImage<T>(size);
	P3DImage presult(result);

	// get pixel data
	impl->getPixelData(result->begin(), result->size());

	result->set_voxel_size(this->get_voxel_size(true));


	const SLookupInit *attr_table = lookup_init;
	while (attr_table->skey) {
		// copy some attributes
		impl->add_attribute(*result, attr_table->skey, attr_table->required, attr_table->default_value);
		++attr_table;
	}
	

	return presult;
}

P2DImage CDicomReader::get_image() const
{
	if (samples_per_pixel() != 1) {
		throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
						"' Image has more then one color channel");
	}
	int bbpa = bits_allocated();
	int bbp  = bits_used();
	if (bbp > bbpa) {
		throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
						"' Bogus image - more bits per pixel used then allocated");
	}

	auto pixel_signed = impl->getUint16(DCM_PixelRepresentation, false, 0);


	if (bbpa == 16) {
		if (pixel_signed) 
			return load_image<signed short>();
		else
			return load_image<unsigned short>();
	}

	throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
					"' doesn't support ", bbp,  " bits per pixel.");
}

P3DImage CDicomReader::get_3dimage() const
{
	if (samples_per_pixel() != 1) {
		throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
						"' Image has more then one color channel");
	}
	int bbpa = bits_allocated();
	int bbp  = bits_used();
	if (bbp > bbpa) {
		throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
						"' Bogus image - more bits per pixel used then allocated");
	}

	auto pixel_signed = impl->getUint16(DCM_PixelRepresentation, false, 0);


	if (bbpa == 16) {
		if (pixel_signed) 
			return load_image3d<signed short>();
		else
			return load_image3d<unsigned short>();
	}

	throw create_exception<invalid_argument>( "CDicomReader: '", m_filename, 
					"' doesn't support ", bbp,  " bits per pixel.");
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


Sint32 CDicomReaderData::getSint32(const DcmTagKey &tagKey, bool required, Sint32 default_value)
{
	Sint32 value = default_value;
	OFCondition success = dcm.getDataset()->findAndGetSint32(tagKey, value);

	if (success.bad() && required){
		throw create_exception<runtime_error>( "CDicom2DImageIOPlugin: unable to get value for '", 
					     tagKey, " ':", status.text());
	}
	if (success.bad()) 
		cvdebug() << "Reading " << tagKey << ": use default " << value << " because '"<< status.text() << "'\n"; 
	else 
		cvdebug() << "Reading " << tagKey << ":" << value << "\n"; 
	return value;
}

Uint16 CDicomReaderData::getUint16(const DcmTagKey &tagKey, bool required, Uint16 default_value)
{
	Uint16 value = default_value;
	OFCondition success = dcm.getDataset()->findAndGetUint16(tagKey, value);

	if (success.bad() && required){
		throw create_exception<runtime_error>( "CDicom2DImageIOPlugin: unable to get value for '", 
					     tagKey, " ':", status.text());
	}
	if (success.bad()) 
		cvdebug() << "Reading " << tagKey << ": use default " << value << " because '"<< status.text() << "'\n"; 
	else 
		cvdebug() << "Reading " << tagKey << ":" << value << "\n"; 
	return value;
}

float CDicomReaderData::getFloat32(const DcmTagKey &tagKey, bool required, float default_value)
{
	Float32 value = default_value;
	OFCondition success = dcm.getDataset()->findAndGetFloat32(tagKey, value);

	if (success.bad() && required){
		throw create_exception<runtime_error>( "CDicom2DImageIOPlugin: unable to get value for '", 
					     tagKey, " ':", status.text());
	}
	return value;
}

float CDicomReaderData::getFloat64(const DcmTagKey &tagKey, bool required, double default_value)
{
	Float64 value = default_value;
	OFCondition success = dcm.getDataset()->findAndGetFloat64(tagKey, value);

	if (success.bad() && required){
		throw create_exception<runtime_error>( "CDicom2DImageIOPlugin: unable to get value for '", 
					     tagKey, " ':", status.text());
	}
	return value;
}


string CDicomReaderData::getAttribute(const DcmTagKey &tagKey, bool required, const char *default_value)
{
	OFString value;
	OFCondition success = dcm.getDataset()->findAndGetOFString(tagKey, value);
	if (success.good()) 
	    return string(value.data());
	
	if (required) {
		throw create_exception<runtime_error>( "DICOM read: Required value '", tagKey, "' not found");
	}
	return string(default_value); 
}

string CDicomReaderData::getAttribute(const string& key, bool required, const char *default_value)
{
	LookupMap::Key k = LookupMap::instance().find(key);

	OFString value;
	OFCondition success = k.second  ?
		dcm.getMetaInfo()->findAndGetOFStringArray(k.first, value):
		dcm.getDataset()->findAndGetOFStringArray(k.first, value);

	if (success.good()) 
	    return string(value.data());

	if (required) {
		throw create_exception<runtime_error>( "DICOM read: Required value '", key, "' not found");
	}
	return string(default_value); 
}

void CDicomReaderData::add_attribute(CAttributedData& image, const char *key, ETagRequirement  required, const char *default_value)
{
	string value = getAttribute(key, required == tr_yes);
	if (!value.empty())
		image.set_attribute(key, value);
	else if (required == tr_yes_defaulted) 
		image.set_attribute(key, string(default_value));
}


template <typename I>
void CDicomReaderData::getPixelData_LittleEndianExplicitTransfer(I out_begin, size_t size)
{
	OFCondition status = dcm.loadAllDataIntoMemory();
	if (status.bad()) {
		throw create_exception<runtime_error>( "DICOM: error loading pixel data:", status.text());
	}

	const Uint16 *values;
	long unsigned int count;
	OFCondition cnd = dcm.getDataset()->findAndGetUint16Array(DCM_PixelData, values, &count, false);
	if (cnd.good()) {
		if (size != count) {
			throw create_exception<runtime_error>( "bogus file, expect ", size, " pixels, ", 
						     "but got data for ", count, " pixels");
		}
		copy(values, values+count, out_begin);
	}else {
		throw create_exception<runtime_error>( "DICOM: required value PixelData:", status.text());
	}
}

template <typename I>
void CDicomReaderData::getPixelData(I out_begin, size_t size)
{
	dcm.getDataset()->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
	
	if (!dcm.getDataset()->canWriteXfer(EXS_LittleEndianExplicit)) {
		OFString of_transfer_syntax;
		OFCondition success = dcm.getMetaInfo()->findAndGetOFString(DCM_TransferSyntaxUID, of_transfer_syntax);
		if (success.bad()) {
			throw create_exception<runtime_error>( "DICOM: Unsupported data encoding: ", success.text());
		}
		string transfer_syntax(of_transfer_syntax.data());
		throw create_exception<runtime_error>( "DICOM: Unsupported data encoding '", transfer_syntax, "'");
	}
	getPixelData_LittleEndianExplicitTransfer(out_begin, size);
}

C2DFVector CDicomReaderData::getImagePixelSpacing()
{
	OFString help;
	OFCondition success = dcm.getDataset()->findAndGetOFString(DCM_ImagerPixelSpacing, help, 0);
	if (success.bad()) {
		throw create_exception<runtime_error>( "Neither 'PixelSpacing' nor 'ImagerPixelSpacing' found!"
						       " As a workaround to can add either of them by using 'dcmodify'");
	}
	C2DFVector result;

	istringstream swidth(help.data());
	swidth >> result.x;

	success = dcm.getDataset()->findAndGetOFString(DCM_ImagerPixelSpacing, help, 1);
	if (success.bad()) {
		throw create_exception<runtime_error>( "Second value in 'ImagerPixelSpacing' not found!"
						       " As a workaround to can add either of them by using 'dcmodify'");
	}
	istringstream sheight(help.data());
	sheight >> result.y;

	cvwarn() << "The image contained no 'PixelSpacing' tag, only 'ImagerPixelSpacing' that corresponds to the " 
		 << "detector pixel spacing. If you can not assume that the rays are parallel, then your object "
		 << "pixel spacing might have to be scaled."; 

	return result;
	
}

C2DFVector CDicomReaderData::getPixelSize()
{
	OFString help;
	OFCondition success = dcm.getDataset()->findAndGetOFString(DCM_PixelSpacing, help, 0);
	if (success.bad()) {
		return getImagePixelSpacing(); 
	}
	C2DFVector result;

	istringstream swidth(help.data());
	swidth >> result.x;

	success = dcm.getDataset()->findAndGetOFString(DCM_PixelSpacing, help, 1);
	if (success.bad()) {
		throw create_exception<runtime_error>( "Required attribute 'PixelSpacing' not found");
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
	if (!pixel_trait<T>::supported)
		throw create_exception<invalid_argument>( "DICOM: unsupported pixel type '", __type_descr<T>::value, "'"); 

	parent->setValueUint16(DCM_BitsAllocated, pixel_trait<T>::AllocSize);
	parent->setValueUint16(DCM_BitsStored, pixel_trait<T>::UseSize);

	auto  minmax = minmax_element(image.begin(), image.end());


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

	for(auto i = image.begin_attributes(); i != image.end_attributes(); ++i)
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

NS_MIA_END
