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

#define VSTREAM_DOMAIN "NiftiImageIOtest"

#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>


#include <nifti/niftiimage.hh>
#include <unistd.h>

using namespace mia; 
using namespace std; 
using namespace niftiimage; 
namespace bmpl=boost::mpl;


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
        C3DFVector origin(10,20,30);
        Quaternion rot(0.5, 0.1, 0.5, 0.7);
        auto iv = image->begin(); 
	auto ev = image->end();
        int i = 0; 

	while (iv != ev)
		*iv++ = i++;
	pimage->set_voxel_size(voxel); 
	pimage->set_origin(origin); 
        pimage->set_rotation(rot); 
	pimage->set_attribute("nifti-qform-code", 1);

	CNifti3DImageIOPlugin io; 
        CNifti3DImageIOPlugin::Data images;
        images.push_back(pimage); 

	stringstream filename; 
	filename << "testimage-" << __type_descr<T>::value << ".nii"; 

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
	BOOST_CHECK_EQUAL(ploaded.get_origin(), origin); 
        auto qloaded = ploaded.get_rotation().as_quaternion();

        BOOST_CHECK_CLOSE(qloaded.x(), rot.x(), 0.001);
        BOOST_CHECK_CLOSE(qloaded.y(), rot.y(), 0.001);
        BOOST_CHECK_CLOSE(qloaded.z(), rot.z(), 0.001);
        BOOST_CHECK_CLOSE(qloaded.w(), rot.w(), 0.001);

        unlink(filename.str().c_str()); 
}


