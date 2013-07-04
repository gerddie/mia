/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <addons/hdf5/hdf5mia.hh> 

NS_MIA_USE
using namespace std;


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
	int data [6] = {1,2,3,4,5,6}; 
	auto mem_type_in = Mia_to_h5_types<int>::mem_datatype(); 

	// write the data set 
	{
		
		auto file_type = Mia_to_h5_types<int>::file_datatype(); 

		
		auto space = H5Space::create(2, dims); 
		auto dataset = H5Dataset::create(get_file(), "/testset", file_type, space);
		
		dataset.write(mem_type_in, data);
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

		int read_data [6] = {0,0,0,0,0,0};
		
		dataset.read(mem_type, read_data); 

		for (int i = 0; i < 6; ++i)
			BOOST_CHECK_EQUAL(read_data[i], data[i]); 
	}
	
}




BOOST_FIXTURE_TEST_CASE(test_ingroup_dataset,  HDF5CoreFileFixture)
{
	hsize_t dims[2] = {2,3}; 

	int data [6] = {1,2,3,4,5,6}; 

	auto file_type = Mia_to_h5_types<int>::file_datatype(); 
	auto mem_type = Mia_to_h5_types<int>::mem_datatype(); 
	
	auto space = H5Space::create(2, dims); 
	auto dataset = H5Dataset::create(get_file(), "/group/testset", file_type, space);
	

}

