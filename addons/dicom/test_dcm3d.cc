/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/internal/autotest.hh>
#include <mia/2d/imageio.hh>
#include <dicom/dcm3d.hh>
#include <dicom/dicom4mia.hh>


using namespace std;
NS_MIA_USE;
using namespace IMAGEIO_3D_DICOM;

struct DicomLoaderFixture {

	DicomLoaderFixture();

	template <typename T>
	void check_attribute(const C3DImage& image, const char *name, const T& value)	{
		TRACE_FUNCTION;

		cvdebug() << "check if attribute '" << name << "' has value '"
			  << value << "' of type '"
			  << typeid(T).name() << "'\n";
		PAttribute attr = image.get_attribute(name);
		BOOST_REQUIRE(attr);

		cvdebug() << "Got attribute of type '" << attr->typedescr() << "'\n";

		cvdebug() << "will cast from'" << typeid(attr.get()).name()
			  <<"' to '" << typeid(TAttribute<T> *).name() << "'\n";


		const TAttribute<T>& pattr = dynamic_cast<const TAttribute<T>&>(*attr);
		T attr_val = pattr;
		BOOST_CHECK_EQUAL(attr_val, value);
	}

	void check_attribute(const C3DImage& image, const char *name, const char *value);
	CDicom3DImageIOPlugin plugin;
};

template <typename T> 
struct DicomSaveLoadFixture : public DicomLoaderFixture {
	DicomSaveLoadFixture();
	void fill_attributes();

	C3DBounds size;
	T3DImage<T> *org_image;
	P3DImage porg_image;
};

DicomLoaderFixture::DicomLoaderFixture()
{

}

BOOST_FIXTURE_TEST_CASE( test_dicom_load, DicomLoaderFixture )
{
	CDicom3DImageIOPlugin::PData pimages = plugin.load(MIA_SOURCE_ROOT"/testdata/IM-0001-0001.dcm");
	BOOST_REQUIRE(pimages);

	BOOST_CHECK_EQUAL(pimages->size(),2u);

	P3DImage i1 = (*pimages)[0];
	BOOST_CHECK_EQUAL(i1->get_size(), C3DBounds(256,256,8));
	BOOST_CHECK_EQUAL(i1->get_voxel_size(), C3DFVector(0.8594, 0.8594, 10));

	P3DImage i2 = (*pimages)[1];
	BOOST_CHECK_EQUAL(i2->get_size(), C3DBounds(512,512,10));
	BOOST_CHECK_EQUAL(i2->get_voxel_size(), C3DFVector(0.293, 0.293, 4.5));
}





void DicomLoaderFixture::check_attribute(const C3DImage& image, const char *name, const char *value)
{
	TRACE_FUNCTION;

	cvdebug() << "check if attribute '" << name << "' has value '"
		  << value << "' of type 'string'\n";

	string svalue(value);
	PAttribute attr = image.get_attribute(name);
 	BOOST_REQUIRE(attr);
	const TAttribute<string> *pattr = dynamic_cast<TAttribute<string> *>(attr.get());
	BOOST_REQUIRE(pattr);
	string attr_val = *pattr;
	BOOST_CHECK_EQUAL(attr_val, svalue);
}

template <typename T> 
void DicomSaveLoadFixture<T>::fill_attributes()
{
	org_image->set_attribute("MediaStorageSOPClassUID",  "somevalue");
	org_image->set_attribute(IDSOPClassUID,  "othervalue");
	org_image->set_voxel_size(C3DFVector(1.45, 2.34, 3));
	org_image->set_origin(C3DFVector(2.45, 2.0, 3.1));
	org_image->set_rotation(Quaternion(0.1, 0.7, 0.5, 0.5).get_rotation_matrix());


	org_image->set_attribute("Modality", "MR");
	org_image->set_attribute("SeriesNumber", "12");
	org_image->set_attribute("AcquisitionNumber", "23");
	org_image->set_attribute("SeriesDescription", "T2-pla");
	org_image->set_attribute("StudyDescription", "PROST");
	org_image->set_attribute("AcquisitionDate","20090909");
	org_image->set_attribute("PatientPosition", "HFP");
	org_image->set_attribute("StudyID", typeid(T).name());
	org_image->set_attribute("ImageType", "ORIGINAL\\PRIMARY\\M\\ND\\RETRO");
	org_image->set_attribute(IDSmallestImagePixelValue,"1");
	org_image->set_attribute(IDLargestImagePixelValue,"20");
	org_image->set_attribute(IDPhotometricInterpretation,"MONOCHROME2");

}


BOOST_FIXTURE_TEST_CASE( test_dicom_us_save_load, DicomSaveLoadFixture<unsigned short> )
{
	CDicom3DImageIOPlugin::Data imagelist;
	imagelist.push_back(porg_image);
	BOOST_REQUIRE(plugin.save("testsave3d-us.dcm", imagelist));

	CDicom3DImageIOPlugin::PData images = plugin.load("testsave3d-us0001_0001.dcm");
	BOOST_REQUIRE(images);
	BOOST_REQUIRE(images->size() == 1);
	P3DImage pimage = *images->begin();
	BOOST_REQUIRE(pimage);

	const C3DUSImage& load_image = dynamic_cast<const C3DUSImage&>(*pimage);

	BOOST_CHECK_EQUAL(load_image.get_size(), size);

	if (!equal(load_image.begin(), load_image.end(), org_image->begin()))  {
		for(size_t z = 0; z < load_image.get_size().z; ++z)
			for(size_t y = 0; y < load_image.get_size().y; ++y)
				for(size_t x = 0; x < load_image.get_size().x; ++x) {
					BOOST_CHECK_EQUAL(load_image(x,y,z),
							  (*org_image)(x,y,z));
				}
	}

	BOOST_CHECK_EQUAL(load_image.get_voxel_size(), org_image->get_voxel_size());
	BOOST_CHECK(load_image ==  *org_image);

}

BOOST_FIXTURE_TEST_CASE( test_dicom_ss_save_load, DicomSaveLoadFixture<signed short> )
{
	CDicom3DImageIOPlugin::Data imagelist;
	imagelist.push_back(porg_image);
	BOOST_REQUIRE(plugin.save("testsave3d-ss.dcm", imagelist));

	CDicom3DImageIOPlugin::PData images = plugin.load("testsave3d-ss0001_0001.dcm");
	BOOST_REQUIRE(images);
	BOOST_REQUIRE(images->size() == 1);
	P3DImage pimage = *images->begin();
	BOOST_REQUIRE(pimage);

	const C3DSSImage& load_image = dynamic_cast<const C3DSSImage&>(*pimage);

	BOOST_CHECK_EQUAL(load_image.get_size(), size);

	if (!equal(load_image.begin(), load_image.end(), org_image->begin()))  {
		for(size_t z = 0; z < load_image.get_size().z; ++z)
			for(size_t y = 0; y < load_image.get_size().y; ++y)
				for(size_t x = 0; x < load_image.get_size().x; ++x) {
					BOOST_CHECK_EQUAL(load_image(x,y,z),
							  (*org_image)(x,y,z));
				}
	}

	BOOST_CHECK_EQUAL(load_image.get_voxel_size(), org_image->get_voxel_size());
	BOOST_CHECK(load_image ==  *org_image);

}


template <typename T> 
DicomSaveLoadFixture<T>::DicomSaveLoadFixture():
	size(4,5,6)
{
	org_image = new T3DImage<T>(size);
	porg_image.reset( org_image );

	short k = 1;
	for (auto i = org_image->begin(); i != org_image->end(); ++i, ++k)
		*i = k;
	fill_attributes();
}




