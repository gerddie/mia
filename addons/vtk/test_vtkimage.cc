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

#define VSTREAM_DOMAIN "vtkImageIOtest"

#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>


#include <vtk/vtkimage.hh>
#include <unistd.h>

using namespace mia; 
using namespace std; 
using namespace vtkimage; 
namespace bmpl=boost::mpl;

// LONG_64BIT seems to be buggy in vtkDataReader
#ifdef LONG_64BIT
#undef LONG_64BIT
#endif 


typedef bmpl::vector<
	unsigned char,
	signed short,
	unsigned short,
	signed int,
	unsigned int,
	float,
	double
#ifdef LONG_64BIT
	,long, unsigned long
#endif
		     > type;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_simple_write_read, T, type ) 
{
        C3DBounds size(2,3,4);
	T3DImage<T> *image = new T3DImage<T>(size); 
        P3DImage pimage(image); 

	C3DFVector voxel(2.0,3.0,4.0); 
        auto iv = image->begin(); 
	auto ev = image->end();
        int i = 0; 

	while (iv != ev)
		*iv++ = i++;
	pimage->set_voxel_size(voxel); 
	pimage->set_origin(C3DFVector(10,20,30)); 

	CVtk3DImageIOPlugin io; 
        CVtk3DImageIOPlugin::Data images;
        images.push_back(pimage); 

	stringstream filename; 
	filename << "testimage-" << __type_descr<T>::value << ".vtk"; 

	cvdebug() << "test with " << filename.str() << "\n"; 

	BOOST_REQUIRE(io.save(filename.str(), images)); 
	
	auto loaded = io.load(filename.str()); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->size() == 1u); 
        const auto& ploaded = dynamic_cast<const T3DImage<T>&>(*(*loaded)[0]); 	
	iv = image->begin(); 


	auto il = ploaded.begin(); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}

	BOOST_CHECK_EQUAL(ploaded.get_voxel_size(), voxel); 
        unlink(filename.str().c_str()); 
}


typedef bmpl::vector<
	unsigned char,
	signed short,
	unsigned short,
	signed int,
	unsigned int,
	float,
	double
#ifdef LONG_64BIT
	,long, unsigned long
#endif
		     > type_xml;

BOOST_AUTO_TEST_CASE_TEMPLATE( test_xml_write_read, T, type_xml ) 
{
        C3DBounds size(2,3,4);
	T3DImage<T> *image = new T3DImage<T>(size); 
        P3DImage pimage(image); 

        auto iv = image->begin(); 
	auto ev = image->end();
        int i = 0; 

	while (iv != ev)
		*iv++ = i++;
	
	C3DFVector voxel(2.0,3.0,4.0); 
	pimage->set_voxel_size(voxel); 

	pimage->set_origin(C3DFVector(10,20,30)); 

	CVtkXML3DImageIOPlugin io; 
        CVtkXML3DImageIOPlugin::Data images;
        images.push_back(pimage); 

	stringstream filename; 
	filename << "testimage-xml-" << __type_descr<T>::value << ".vtk"; 

	cvdebug() << "test with " << filename.str() << "\n"; 

	BOOST_REQUIRE(io.save(filename.str(), images)); 
	
	auto loaded = io.load(filename.str()); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->size() == 1u); 
        const auto& ploaded = dynamic_cast<const T3DImage<T>&>(*(*loaded)[0]); 	
	iv = image->begin(); 


	auto il = ploaded.begin(); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}
	BOOST_CHECK_EQUAL(ploaded.get_voxel_size(), voxel); 
        unlink(filename.str().c_str()); 
}

typedef bmpl::vector<
	signed char,
	unsigned char,
	signed short,
	unsigned short,
	signed int,
	unsigned int,
	float,
	double
#ifdef LONG_64BIT
	,long, unsigned long
#endif
		     > type_mhd;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_mhd_write_read, T, type_mhd ) 
{
        C3DBounds size(2,3,4);
	T3DImage<T> *image = new T3DImage<T>(size); 
        P3DImage pimage(image); 

        auto iv = image->begin(); 
	auto ev = image->end();
        int i = 0; 

	while (iv != ev)
		*iv++ = i++;

	C3DFVector voxel(2.0,3.0,4.0); 
	pimage->set_voxel_size(voxel); 
       
	pimage->set_origin(C3DFVector(10,20,30)); 
	
	CMhd3DImageIOPlugin io; 
        CMhd3DImageIOPlugin::Data images;
        images.push_back(pimage); 

	stringstream filename; 
	stringstream rawfilename; 
	stringstream zrawfilename; 

	filename << "testimage-mhd-" << __type_descr<T>::value << ".mhd"; 
	rawfilename << "testimage-mhd-" << __type_descr<T>::value << ".raw"; 
	zrawfilename << "testimage-mhd-" << __type_descr<T>::value << ".zraw"; 

	cvdebug() << "test with " << filename.str() << "\n"; 

	BOOST_REQUIRE(io.save(filename.str(), images)); 
	
	auto loaded = io.load(filename.str()); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->size() == 1u); 
	EPixelType expect_type = pixel_type<T>::value; 
	BOOST_CHECK_EQUAL((*loaded)[0]->get_pixel_type(), expect_type); 
        const auto& ploaded = dynamic_cast<const T3DImage<T>&>(*(*loaded)[0]); 	
	iv = image->begin(); 


	auto il = ploaded.begin(); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}
	BOOST_CHECK_EQUAL(ploaded.get_voxel_size(), voxel); 
        unlink(filename.str().c_str()); 
        unlink(rawfilename.str().c_str()); 
        unlink(zrawfilename.str().c_str()); 
	
}





BOOST_AUTO_TEST_CASE( test_simple_write_read_bool ) 
{
        C3DBounds size(2,3,4);
	C3DBitImage *image = new C3DBitImage(size); 
        P3DImage pimage(image); 

	C3DFVector voxel(2.0,3.0,4.0); 
        auto iv = image->begin(); 
	auto ev = image->end();
        int i = 0; 

	while (iv != ev)
		*iv++ = i++ & 1;
	pimage->set_voxel_size(voxel); 

	CVtk3DImageIOPlugin io; 
        CVtk3DImageIOPlugin::Data images;
        images.push_back(pimage); 

	stringstream filename; 
	filename << "testimage-" << __type_descr<bool>::value << ".vtk"; 

	cvdebug() << "test with " << filename.str() << "\n"; 

	BOOST_REQUIRE(io.save(filename.str(), images)); 
	
	auto loaded = io.load(filename.str()); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->size() == 1u); 
        const auto& ploaded = dynamic_cast<const C3DBitImage&>(*(*loaded)[0]); 	
	iv = image->begin(); 


	auto il = ploaded.begin(); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}

	BOOST_CHECK_EQUAL(ploaded.get_voxel_size(), voxel); 
//        unlink(filename.str().c_str()); 
}
