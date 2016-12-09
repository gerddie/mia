/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#define VSTREAM_DOMAIN "test-HDF5MIA"

#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/insert_range.hpp>

#include <addons/hdf5/hdf5a_mia.hh> 

NS_MIA_USE
using namespace std;
namespace bmpl=boost::mpl;

typedef bmpl::vector<int8_t,
		     uint8_t,
		     int16_t,
		     uint16_t,
		     int32_t,
		     uint32_t,
		     int64_t,
		     uint64_t,
		     float,
		     double
		     > test_types;

class HDF5CoreFileFixture {
	
protected: 
	HDF5CoreFileFixture(); 
	~HDF5CoreFileFixture(); 

	const H5File& get_file() const;
private: 
	H5Property access_plist;
	H5File  m_core_file; 

}; 



HDF5CoreFileFixture::HDF5CoreFileFixture()
{
	access_plist = H5Property::create(H5P_FILE_ACCESS);
	H5Pset_fapl_core (access_plist, 1024, 0); 

	m_core_file = H5File::create("core.h5", H5F_ACC_TRUNC, H5P_DEFAULT, access_plist); 
}

HDF5CoreFileFixture::~HDF5CoreFileFixture()
{
	
	
}


const H5File& HDF5CoreFileFixture::get_file() const
{
	return m_core_file; 
}




BOOST_FIXTURE_TEST_CASE(test_core_hdf5_io_driver,  HDF5CoreFileFixture)
{
	
}

BOOST_FIXTURE_TEST_CASE(test_simple_dataset,  HDF5CoreFileFixture)
{
	hsize_t dims[2] = {2,3}; 
	vector<int32_t> data = {1,2,3,4,5,6}; 
	auto mem_type_in = Mia_to_h5_types<int32_t>::mem_datatype(); 

	// write the data set 
	{
		
		auto file_type = Mia_to_h5_types<int32_t>::file_datatype(); 

		
		auto space = H5Space::create(2, dims); 
		auto dataset = H5Dataset::create(get_file(), "/testset", file_type, space);
		
		dataset.write_data(data, int32_t());


		cvdebug() << "Get the data set back (first)\n"; 
		
		auto dataset2 = H5Dataset::open(get_file(), "/testset");
		auto size = dataset2.get_size();
				
		BOOST_CHECK_EQUAL(size.size(), 2u); 
		BOOST_CHECK_EQUAL(size[0],2u);
		BOOST_CHECK_EQUAL(size[1],3u); 

	}
	// close data set automatically, and now reopen it
	cvdebug() << "second part\n"; 
	{
		auto dataset = H5Dataset::open(get_file(), "/testset");
		
		auto size = dataset.get_size();
		
		BOOST_CHECK_EQUAL(size.size(), 2u); 
		BOOST_CHECK_EQUAL(size[0],2u);
		BOOST_CHECK_EQUAL(size[1],3u); 

		H5Type file_type(H5Dget_type(dataset)); 
		H5Type mem_type = file_type.get_native_type(); 

		BOOST_CHECK(H5Tequal( mem_type, mem_type_in ) > 0); 

		vector<int32_t> read_data(6);
		
		dataset.read_data(read_data, int32_t()); 

		for (int i = 0; i < 6; ++i)
			BOOST_CHECK_EQUAL(read_data[i], data[i]); 
	}
	
}


BOOST_FIXTURE_TEST_CASE(test_bool_dataset,  HDF5CoreFileFixture)
{
	hsize_t dims[2] = {2,3}; 
	vector<bool> data {false, true, true, false, false, true}; 
	auto mem_type_in = Mia_to_h5_types<bool>::mem_datatype(); 

	// write the data set 
	{
		
		auto file_type = Mia_to_h5_types<bool>::file_datatype(); 

		
		auto space = H5Space::create(2, dims); 
		auto dataset = H5Dataset::create(get_file(), "/testset", file_type, space);
		
		dataset.write_data(data, true);
	}
	// close data set automatically, and now reopen it 
	{
		auto dataset = H5Dataset::open(get_file(), "/testset");
		auto size = dataset.get_size(); 
		BOOST_CHECK_EQUAL(size.size(), 2u); 
		BOOST_CHECK_EQUAL(size[0],2u);
		BOOST_CHECK_EQUAL(size[1],3u); 

		H5Type file_type(H5Dget_type(dataset)); 
		H5Type mem_type = file_type.get_native_type(); 

		BOOST_CHECK(H5Tequal( mem_type, mem_type_in ) > 0); 

		vector<bool> read_data(6); 
		
		dataset.read_data(read_data, false); 

		for (int i = 0; i < 6; ++i)
			BOOST_CHECK_EQUAL(read_data[i], data[i]); 
	}
	
}

template <typename T> 
class TestDatasetFixture : public HDF5CoreFileFixture {
	
protected:
	void test(const string& path, const std::vector<hsize_t>& size, const std::vector<T>& data); 
private: 
	void save(const string& path, const std::vector<hsize_t>& size, const std::vector<T>& data); 
	void read_and_test(const string& path, const std::vector<hsize_t>& test_size, const std::vector<T>& test_data); 

}; 

template <typename T> 
void TestDatasetFixture<T>::test(const string& path, const std::vector<hsize_t>& size, const std::vector<T>& data)
{
	save(path, size, data); 
	read_and_test(path, size, data);
}


template <typename T> 
void TestDatasetFixture<T>::save(const string& path, const std::vector<hsize_t>& size, const std::vector<T>& data)
{
	auto file_type = Mia_to_h5_types<T>::file_datatype(); 
	
	
	auto space = H5Space::create(size); 
	auto dataset = H5Dataset::create(get_file(), path.c_str(), file_type, space);
	dataset.write_data(data, T());
}

template <typename T> 
void TestDatasetFixture<T>::read_and_test(const string& path, const std::vector<hsize_t>& test_size, 
					  const std::vector<T>& test_data)
{
	auto dataset = H5Dataset::open(get_file(), path.c_str());
	auto size = dataset.get_size(); 
	BOOST_CHECK_EQUAL(size.size(), test_size.size()); 
	BOOST_REQUIRE(size.size()== test_size.size()); 

	size_t length = 1; 
	for (size_t i = 0; i < size.size(); ++i) {
		BOOST_CHECK_EQUAL(size[i],test_size[i]);
		length *= test_size[i]; 
	}
	
	H5Type file_type(H5Dget_type(dataset)); 
	H5Type mem_type = file_type.get_native_type(); 

	BOOST_REQUIRE(H5Tequal( mem_type, Mia_to_h5_types<T>::mem_datatype() ) > 0); 
	
	std::vector<T> read_data(length); 
	
	dataset.read_data(read_data, T()); 
		
	for (size_t i = 0; i < length; ++i)
		BOOST_CHECK_EQUAL(read_data[i], test_data[i]); 
}

template <typename T> 
class TestDatasetIOInGroupFixture : public TestDatasetFixture<T> {
public:
	void run(); 

}; 


template <typename T> 
void TestDatasetIOInGroupFixture<T>::run() 
{
	vector<hsize_t> dims = {2,3}; 
	vector<T>  data = {1,2,3,4,5,6}; 
	const char path[] = "/group1/group2/testset"; 
	this->test(path, dims, data); 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_dataset_io, T , test_types )
{
	TestDatasetIOInGroupFixture<T>().run(); 
}


template <typename T>
class TestAttrfixture : public HDF5CoreFileFixture {
public: 
	void run(); 
}; 

template <typename T>
void TestAttrfixture<T>::run() 
{
	const T value = 10; 
	PAttribute attr(new TAttribute<T>(value));
	auto h5attr = H5AttributeTranslatorMap::instance().translate(get_file(), "attr", *attr); 

	auto pattr = H5AttributeTranslatorMap::instance().translate("attr", h5attr); 
	int test_type = attribute_type<T>::value; 
	BOOST_CHECK_EQUAL(pattr->type_id(), test_type);
	

	

	auto rattr = dynamic_cast<const TAttribute<T>*>(pattr.get());
	if (!rattr)  {
		cvdebug() << "cast from'" << typeid(attr.get()).name()
			  <<"' to '" << typeid(TAttribute<T> *).name() << "' failed\n";		
	}
	BOOST_REQUIRE(rattr); 
	
	BOOST_CHECK_EQUAL(*rattr, value); 
}

BOOST_AUTO_TEST_CASE_TEMPLATE( test_attributes , T , test_types )
{
	TestAttrfixture<T>().run(); 
}



template <typename T>
class TestVectorAttrfixture : public HDF5CoreFileFixture {
public: 
	void run(); 
}; 

template <typename T>
void TestVectorAttrfixture<T>::run() 
{
	const vector<T> value = {10, 20, 30}; 
	PAttribute attr(new TAttribute<vector<T>>(value));
	auto h5attr = H5AttributeTranslatorMap::instance().translate(get_file(), "attr", *attr); 

	auto pattr = H5AttributeTranslatorMap::instance().translate("attr", h5attr); 
	int test_type = attribute_type<vector<T>>::value; 
	BOOST_CHECK_EQUAL(pattr->type_id(), test_type); 

	auto rattr = dynamic_cast<const TAttribute<vector<T> > *>(pattr.get());
	if (!rattr)  {
		cvdebug() << "cast from'" << typeid(attr.get()).name()
			  <<"' to '" << typeid(TAttribute<T> *).name() << "' failed\n";		
	}
	BOOST_REQUIRE(rattr); 	
	
	const vector<T> rvalue = *rattr; 
	
	BOOST_CHECK_EQUAL(rvalue.size(), value.size()); 
	BOOST_REQUIRE(rvalue.size() == value.size()); 
	for (auto r = rvalue.begin(), v = value.begin(); r != rvalue.end(); ++r, ++v){
		BOOST_CHECK_EQUAL(*r, *v); 
	}
}



BOOST_AUTO_TEST_CASE_TEMPLATE( test_vector_attributes , T , test_types )
{
	TestVectorAttrfixture<T>().run(); 
}


BOOST_FIXTURE_TEST_CASE (test_string_attribute, HDF5CoreFileFixture) 
{
	string value("a test string"); 
	PAttribute attr(new TAttribute<string>(value));
	
	auto h5attr = H5AttributeTranslatorMap::instance().translate(get_file(), "attr", *attr); 

	auto pattr = H5AttributeTranslatorMap::instance().translate("attr", h5attr); 
	BOOST_REQUIRE(pattr); 

	int test_type = attribute_type<string>::value; 
	BOOST_CHECK_EQUAL(pattr->type_id(), test_type);
	
	auto& rattr = dynamic_cast<const TAttribute<string>&>(*pattr); 
	const string rvalue = rattr; 
	BOOST_CHECK_EQUAL(rvalue, value); 
}; 

BOOST_FIXTURE_TEST_CASE (test_vstring_attribute, HDF5CoreFileFixture) 
{
	const vector<string> value= {"a test string", "another test string"}; 
	PAttribute attr(new TAttribute<vector<string>>(value));
	
	auto h5attr = H5AttributeTranslatorMap::instance().translate(get_file(), "attr", *attr); 

	auto pattr = H5AttributeTranslatorMap::instance().translate("attr", h5attr); 
	BOOST_REQUIRE(pattr); 

	int test_type = attribute_type<vector<string>>::value; 
	BOOST_CHECK_EQUAL(pattr->type_id(), test_type);
	
	auto& rattr = dynamic_cast<const TAttribute<vector<string>>&>(*pattr); 
	const vector<string> rvalue = rattr; 
	BOOST_CHECK_EQUAL(rvalue.size(), value.size());
	
	for (auto r = rvalue.begin(), v = value.begin(); r != rvalue.end(); ++r, ++v){
		BOOST_CHECK_EQUAL(*r, *v); 
	}
}; 

BOOST_FIXTURE_TEST_CASE (test_attribute_list, HDF5CoreFileFixture) 
{
	CAttributedData original_data; 

	original_data.set_attribute("string", PAttribute(new CStringAttribute("some string"))); 
	original_data.set_attribute("int", PAttribute(new CIntAttribute(10))); 
	original_data.set_attribute("vfloat", PAttribute(new CVFloatAttribute({1.0, 2.3, 3.4}))); 

	translate_to_hdf5_attributes(get_file(), original_data); 
	
	CAttributedData loaded_data = get_file().read_attributes(); 
	
	BOOST_CHECK_EQUAL(loaded_data, original_data); 
}
