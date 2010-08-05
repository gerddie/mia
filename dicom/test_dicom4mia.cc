/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#define BOOST_TEST_DYN_LINK

#include <climits>
#include <mia/internal/autotest.hh>
#include <stdexcept>

#define ENABLE_TEST_HACKS

#include <dicom/dicom4mia.hh>

using namespace std;
BOOST_AUTO_TEST_CASE(test_read_dicom_attributes)
{
        string filename(MIA_SOURCE_ROOT"/testdata/test.dcm");
        cvdebug() << "open: " << filename << "\n";
        CDicomReader reader(filename.c_str());
	BOOST_CHECK(reader.good());

	BOOST_CHECK_EQUAL(C2DBounds(224, 256), reader.image_size());

	BOOST_CHECK_EQUAL(reader.rows(), 256);
	BOOST_CHECK_EQUAL(reader.cols(), 224);
	BOOST_CHECK_EQUAL(reader.samples_per_pixel(), 1);
	BOOST_CHECK_EQUAL(reader.bits_allocated(), 16);
	BOOST_CHECK_EQUAL(reader.bits_used(),12);

	BOOST_CHECK_EQUAL(reader.get_attribute(IDStudyDescription, true),
			  "Cardiovascular^Heart-Cardiac Function");

	BOOST_CHECK_THROW(reader.get_attribute(IDTestValue, true), runtime_error);

	BOOST_CHECK(reader.get_attribute(IDTestValue, false).empty());

	BOOST_CHECK_EQUAL(reader.get_attribute(IDSeriesDescription, true),"cine_retro_aortic arch");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDModality, true), "MR");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDSeriesNumber, true), "24");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDPatientPosition, true), "HFS");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDAcquisitionDate, true), "20041116");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDAcquisitionNumber, true), "1");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDInstanceNumber, true), "6");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDStudyID, true), "4273329");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDImageType, true), "ORIGINAL\\PRIMARY\\M\\ND\\RETRO");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDSliceLocation, true), "15.088232007086");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDMediaStorageSOPClassUID, true),"1.2.840.10008.5.1.4.1.1.4");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDSmallestImagePixelValue, true),"7");
	BOOST_CHECK_EQUAL(reader.get_attribute(IDLargestImagePixelValue, true),"1548");

	BOOST_CHECK_EQUAL(reader.get_pixel_size(), C2DFVector(1.484375,1.484375));

}

struct DicomFixture {

	DicomFixture();

	template <typename T>
	void check_attribute(const C2DImage& image, const char *name, const T& value)	{
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

	void check_attribute(const C2DImage& image,const char *name, const char *value);
};

BOOST_FIXTURE_TEST_CASE(test_read_dicom_pixels, DicomFixture)
{
        string filename(MIA_SOURCE_ROOT"/testdata/test.dcm");
        cvdebug() << "open: " << filename << "\n";

        CDicomReader reader(filename.c_str());
	BOOST_REQUIRE(reader.good());

	P2DImage image = reader.get_image();

	BOOST_REQUIRE(image);

	BOOST_CHECK_EQUAL(image->get_size(), C2DBounds(224, 256));
	BOOST_CHECK_EQUAL(image->get_pixel_size(), C2DFVector(1.484375,1.484375));

	check_attribute(*image, IDStudyDescription, "Cardiovascular^Heart-Cardiac Function");
	check_attribute(*image, IDSeriesDescription,"cine_retro_aortic arch");
	check_attribute(*image, IDModality, "MR");
	check_attribute(*image, IDSeriesNumber, 24);
	check_attribute(*image, IDPatientPosition, "HFS");
	check_attribute(*image, IDAcquisitionDate, "20041116");
	check_attribute(*image, IDAcquisitionNumber, 1);
	check_attribute(*image, IDInstanceNumber, 6);
	check_attribute(*image, IDStudyID, "4273329");
	check_attribute(*image, IDImageType, "ORIGINAL\\PRIMARY\\M\\ND\\RETRO");
	check_attribute(*image, IDSliceLocation, 15.088232007086f);
	check_attribute(*image, IDMediaStorageSOPClassUID,"1.2.840.10008.5.1.4.1.1.4");

	const C2DUSImage& img = dynamic_cast<const C2DUSImage&>(*image);


	BOOST_CHECK_EQUAL(img(0,0), 0);

	// the next test is not necessarily right
	BOOST_CHECK_EQUAL(img(128,114), 135);


}

BOOST_AUTO_TEST_CASE(test_read_dicom_fail)
{
	BOOST_CHECK(!CDicomReader("nothere.dcm").good());
}

BOOST_FIXTURE_TEST_CASE(test_create_dicom, DicomFixture)
{
	C2DBounds size(20,21);
	C2DFVector pixel_size(1.4, 2.3);
	C2DUSImage image(size);
	image.set_pixel_size(pixel_size);

	fill(image.begin(), image.end(), 200);
	image(10,10) = 10;
	image(10,11) = 2000;

	image.set_attribute(IDSliceLocation, "100.21");
	image.set_attribute(IDMediaStorageSOPClassUID, "0");
	image.set_attribute(IDSOPClassUID, "lala");
	image.set_attribute(IDStudyDescription, "study");
	image.set_attribute(IDSeriesDescription, "series");

	image.set_attribute(IDModality, "MR");
	image.set_attribute(IDSeriesNumber, "1");
	image.set_attribute(IDAcquisitionDate, "991012");
	image.set_attribute(IDAcquisitionNumber, "12");
	image.set_attribute(IDInstanceNumber, "7");
	image.set_attribute(IDStudyID, "009900");
	image.set_attribute(IDImageType, "imagetype");
	image.set_attribute(IDSliceLocation, "1.231");
	image.set_attribute(IDSmallestImagePixelValue, "10");
	image.set_attribute(IDLargestImagePixelValue, "2000");

	CDicomWriter writer(image);

	CDicomReader reader = ugly_trick_writer_dcm_to_reader_dcm(writer);

	BOOST_CHECK_EQUAL(reader.image_size(), size);
	BOOST_CHECK_EQUAL(reader.get_pixel_size(), pixel_size);

	P2DImage read_img = reader.get_image();
	BOOST_CHECK_EQUAL(*read_img->get_attribute_list(), *image.get_attribute_list());
}

DicomFixture::DicomFixture()
{
	TTranslator<float>::register_for(IDSliceLocation);
	TTranslator<int>::register_for(IDSeriesNumber);
	TTranslator<int>::register_for(IDAcquisitionNumber);
	TTranslator<int>::register_for(IDInstanceNumber);
}

void DicomFixture::check_attribute(const C2DImage& image, const char *name, const char *value)
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

BOOST_AUTO_TEST_CASE(test_read_dicom_pixel_data_jpeg)
{
	CDicomReader reader(MIA_SOURCE_ROOT"/testdata/IM-0001-0001.dcm");
	BOOST_REQUIRE(reader.good());
	P2DImage image = reader.get_image();
}
