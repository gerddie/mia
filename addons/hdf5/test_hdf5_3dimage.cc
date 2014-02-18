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


#define VSTREAM_DOMAIN "test-HDF5-3dimage"
#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>

#include <addons/hdf5/hdf5_3dimage.hh>

using namespace std; 
using namespace mia; 
using namespace hdf5_3dimage; 

template <typename T> 
struct __fill_image {
	static void apply(T3DImage<T>& image) {
		int v = 1; 
		for (auto i = image.begin(); i != image.end(); ++i) 
			*i = v++; 
	}
}; 

template <> 
struct __fill_image<bool> {
	static void apply(C3DBitImage& image) {
		bool v = false; 
		for (auto i = image.begin(); i != image.end(); ++i) {
			*i = v; 
			v = !v; 
		}
	}
}; 


typedef boost::mpl::vector<
	bool, 
	signed char,  
	unsigned char,
	signed short,
	unsigned short,
	signed int,
	unsigned int,
#ifdef LONG_64BIT
	signed long, 
	unsigned long, 
#endif
	float,
	double
	> test_pixeltypes;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_simple_write_read, T, test_pixeltypes ) 
{
	C3DBounds size (2,3,4); 
	T3DImage<T> *image = new T3DImage<T>(size); 
	image->set_attribute("int", PAttribute(new CIntAttribute(2))); 
	const C3DFVector voxel_size(2,2.5,2.5); 
	image->set_voxel_size(voxel_size); 

	__fill_image<T>::apply(*image); 
	
	CHDF53DImageIOPlugin io; 
        CHDF53DImageIOPlugin::Data images;
        images.push_back(P3DImage(image)); 
	
	stringstream filename; 
	filename << "testimage-" << __type_descr<T>::value << ".h5"; 

	cvdebug() << "test with " << filename.str() << "\n"; 

	BOOST_REQUIRE(io.save(filename.str(), images)); 
	
	auto loaded = io.load(filename.str()); 
	BOOST_REQUIRE(loaded); 
	
	BOOST_REQUIRE(loaded->size() == 1u); 
        const auto& ploaded = dynamic_cast<const T3DImage<T>&>(*(*loaded)[0]); 	
	auto iv = image->begin(); 
	auto ev = image->end(); 


	auto il = ploaded.begin(); 
	
	while (iv != ev) {
		BOOST_CHECK_EQUAL(*il, *iv); 
		++iv; 
		++il; 
	}
	BOOST_CHECK_EQUAL(ploaded.get_voxel_size(), voxel_size); 
	
	auto int_attr = ploaded.template get_attribute_as<int>("int"); 
	BOOST_CHECK_EQUAL(int_attr, 2); 

        unlink(filename.str().c_str()); 

}


