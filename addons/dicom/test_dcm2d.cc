/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <mia/internal/autotest.hh>
#include <dicom/dcm2d.hh>
#include <dicom/dicom4mia.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace IMAGEIO_2D_DICOM;

struct DicomLoaderFixture {

	DicomLoaderFixture();

	template <typename T>
	void check_attribute(const char *name, const T& value)	{
		TRACE_FUNCTION;

		cvdebug() << "check if attribute '" << name << "' has value '"
			  << value << "' of type '"
			  << typeid(T).name() << "'\n";
		PAttribute attr = pimage->get_attribute(name);
		BOOST_REQUIRE(attr);

		cvdebug() << "Got attribute of type '" << attr->typedescr() << "'\n";

		cvdebug() << "will cast from'" << typeid(attr.get()).name()
			  <<"' to '" << typeid(TAttribute<T> *).name() << "'\n";


		const TAttribute<T>& pattr = dynamic_cast<const TAttribute<T>&>(*attr);
		T attr_val = pattr;
		BOOST_CHECK_EQUAL(attr_val, value);
	}

	void check_attribute(const char *name, const char *value);
	CDicom2DImageIOPlugin plugin;
	P2DImage pimage;
};

DicomLoaderFixture::DicomLoaderFixture()
{
        string filename(MIA_SOURCE_ROOT"/testdata/test.dcm");
        cvdebug() << "open: " << filename << "\n";

        CDicom2DImageIOPlugin::PData images = plugin.load(filename.c_str());
	BOOST_REQUIRE(images);

	BOOST_REQUIRE(images->size() == 1);
	pimage = *images->begin();

	BOOST_REQUIRE(pimage);
}


void DicomLoaderFixture::check_attribute(const char *name, const char *value)
{
	TRACE_FUNCTION;

	cvdebug() << "check if attribute '" << name << "' has value '"
		  << value << "' of type 'string'\n";

	string svalue(value);
	PAttribute attr = pimage->get_attribute(name);
 	BOOST_REQUIRE(attr);
	const TAttribute<string> *pattr = dynamic_cast<TAttribute<string> *>(attr.get());
	BOOST_REQUIRE(pattr);
	string attr_val = *pattr;
	BOOST_CHECK_EQUAL(attr_val, svalue);
}



BOOST_FIXTURE_TEST_CASE( test_dicom_load, DicomLoaderFixture )
{
	check_attribute("StudyDescription", "Cardiovascular^Heart-Cardiac Function");
	check_attribute("SeriesDescription","cine_retro_aortic arch");
	check_attribute("Modality", "MR");
	check_attribute("SeriesNumber", 24);
	check_attribute("PatientPosition", "HFS");
	check_attribute("AcquisitionDate", "20041116");
	check_attribute("AcquisitionNumber", 1);
	check_attribute("InstanceNumber", 6);
	check_attribute("StudyID", "4273329");
	check_attribute("ImageType", "ORIGINAL\\PRIMARY\\M\\ND\\RETRO");
	check_attribute("SliceLocation", 15.088232007086f);
	check_attribute("MediaStorageSOPClassUID","1.2.840.10008.5.1.4.1.1.4");

	C2DFVector pixel_size = pimage->get_pixel_size();

	BOOST_CHECK_EQUAL(pixel_size, C2DFVector(1.484375,1.484375));

	// test
	const C2DUSImage& image = dynamic_cast<const C2DUSImage&>(*pimage);

	BOOST_CHECK_EQUAL(image.get_size(), C2DBounds(224,256));
	BOOST_CHECK_EQUAL(image(0,0), 0);

	// the next test is not necessarily right
	BOOST_CHECK_EQUAL(image(128,114), 135);
}

struct DicomSaveLoadFixture {
	DicomSaveLoadFixture();
	CDicom2DImageIOPlugin plugin;
protected:
	void fill_attributes();
	C2DBounds size;
	C2DUSImage *org_image;
	P2DImage porg_image;
};

void DicomSaveLoadFixture::fill_attributes()
{
	org_image->set_attribute(IDMediaStorageSOPClassUID,  "somevalue");
	org_image->set_attribute(IDSOPClassUID,  "othervalue");
	org_image->set_pixel_size(C2DFVector(1.45, 2.34));


	org_image->set_attribute("Modality", "MR");
	org_image->set_attribute("SeriesNumber", "12");
	org_image->set_attribute("AcquisitionNumber", "23");
	org_image->set_attribute("InstanceNumber", "7");
	org_image->set_attribute("SeriesDescription", "T2-pla");
	org_image->set_attribute("StudyDescription", "PROST");
	org_image->set_attribute("AcquisitionDate","20090909");
	org_image->set_attribute("PatientPosition", "HFP");
	org_image->set_attribute("StudyID", "786755");
	org_image->set_attribute("SliceLocation", "12.6755");
	org_image->set_attribute("ImageType", "ORIGINAL\\PRIMARY\\M\\ND\\RETRO");

	org_image->set_attribute(IDSmallestImagePixelValue,"0");
	org_image->set_attribute(IDLargestImagePixelValue,"119");

}

DicomSaveLoadFixture::DicomSaveLoadFixture():
	size(10,12)
{
	org_image = new C2DUSImage(size);
	short k = 0;
	for (C2DUSImage::iterator i = org_image->begin(); i != org_image->end(); ++i, ++k)
		*i = k;

	porg_image.reset(org_image);
}


BOOST_FIXTURE_TEST_CASE( test_dicom_save_load, DicomSaveLoadFixture )
{

	fill_attributes();

	CDicom2DImageIOPlugin::Data imagelist;
	imagelist.push_back(porg_image);
	BOOST_REQUIRE(plugin.save("testsave.dcm", imagelist));

	CDicom2DImageIOPlugin::PData images = plugin.load("testsave.dcm");
	BOOST_REQUIRE(images);
	BOOST_REQUIRE(images->size() == 1);
	P2DImage pimage = *images->begin();
	BOOST_REQUIRE(pimage);

	BOOST_CHECK(*pimage ==  *org_image);

	const C2DUSImage& load_image = dynamic_cast<const C2DUSImage&>(*pimage);

	BOOST_CHECK_EQUAL(load_image.get_size(), size);

	BOOST_CHECK(equal(load_image.begin(), load_image.end(), org_image->begin()));

}

BOOST_FIXTURE_TEST_CASE( test_dicom_load_nothing, DicomSaveLoadFixture )
{
	CDicom2DImageIOPlugin plugin;
	CDicom2DImageIOPlugin::PData images = plugin.load("nonexistence.dcm");
	BOOST_CHECK(!images.get());
}

