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

#include <climits>

#define VSTREAM_DOMAIN "test-transio"
#include <mia/internal/autotest.hh>


#include <set>

#include <mia/core.hh>
#include <mia/3d/vectorfield.hh>

#include <boost/mpl/vector.hpp>

#include <cmath>

namespace bmpl=boost::mpl;
NS_MIA_USE; 
using namespace std; 


struct C3DVectorfieldFixture {
	C3DVectorfieldFixture(); 
	C3DBounds size; 
	C3DFVectorfield field; 
}; 

BOOST_FIXTURE_TEST_CASE(  test_3ddatafield_zslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.x * size.y * 3); 
	
	field.read_zslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,y,1).x);
			BOOST_CHECK_EQUAL(i[1], field(x,y,1).y);
			BOOST_CHECK_EQUAL(i[2], field(x,y,1).z);
		}
			
	field.write_zslice_flat(2, slice); 
	i = slice.begin(); 
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,y,2).x);
			BOOST_CHECK_EQUAL(i[1], field(x,y,2).y);
			BOOST_CHECK_EQUAL(i[2], field(x,y,2).z);
		}
}


BOOST_FIXTURE_TEST_CASE( test_3ddatafield_yslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.x * size.z * 3); 
	
	field.read_yslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t x = 0; x < size.x; ++x, i+=3 ) {
			BOOST_CHECK_EQUAL(i[0], field(x,1,z).x);
			BOOST_CHECK_EQUAL(i[1], field(x,1,z).y);
			BOOST_CHECK_EQUAL(i[2], field(x,1,z).z);
		}

	field.write_yslice_flat(2, slice); 
	i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t x = 0; x < size.x; ++x, i+=3) {
			BOOST_CHECK_EQUAL(i[0], field(x,2,z).x);
			BOOST_CHECK_EQUAL(i[1], field(x,2,z).y);
			BOOST_CHECK_EQUAL(i[2], field(x,2,z).z);
		}
}

BOOST_FIXTURE_TEST_CASE( test_3ddatafield_xslice_flat, C3DVectorfieldFixture)
{
	vector<float> slice(size.y * size.z * 3); 
	
	field.read_xslice_flat(1, slice); 
	auto i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y, i+=3){
			BOOST_CHECK_EQUAL(i[0], field(1,y,z).x);
			BOOST_CHECK_EQUAL(i[1], field(1,y,z).y);
			BOOST_CHECK_EQUAL(i[2], field(1,y,z).z);
		}
		

	field.write_xslice_flat(0, slice); 
	i = slice.begin(); 
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y, i+=3){
			BOOST_CHECK_EQUAL(i[0], field(0,y,z).x);
			BOOST_CHECK_EQUAL(i[1], field(0,y,z).y);
			BOOST_CHECK_EQUAL(i[2], field(0,y,z).z);
		}
}


C3DVectorfieldFixture::C3DVectorfieldFixture():
	size(4,5,6), 
	field(size)
{
	auto i = field.begin(); 
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++i)
				*i = C3DFVector(x+1, y+1, z+1); 
}


typedef bmpl::vector<
#ifdef __SSE__
	C3DSSELinearVectorfieldInterpolator,
#endif 
	C3DLinearVectorfieldInterpolator>::type test_types; 

BOOST_AUTO_TEST_CASE_TEMPLATE( test_interpolation, Interpolator ,  test_types)
{			
	
						
	C3DBounds size(3,3,3); 
	C3DFVectorfield f(size); 

	
	auto ifield = f.begin(); 
	for (size_t iz = 0; iz < size.z; ++iz) 
		for (size_t iy = 0; iy < size.y; ++iy) 
			for (size_t ix = 0; ix < size.x; ++ix, ++ifield) {
				*ifield = C3DFVector(2* ix+1, 3* iy +2, 4 * iz+1); 
			}

	
	Interpolator ip(f); 

	unsigned  n = 10; 
	float d = n / 2.0;
	

	for (size_t iz = 1; iz < n; ++iz) 
		for (size_t iy = 1; iy < n; ++iy) 
			for (size_t ix = 1; ix < n; ++ix) {
				C3DFVector dx(ix / d, iy / d,  iz / d); 
				C3DFVector value = ip(dx);

				int x = static_cast<int>( floor(dx.x) ); 
				int y = static_cast<int>( floor(dx.y) ); 
				int z = static_cast<int>( floor(dx.z) ); 

				dx.x = dx.x - x; 
				dx.y = dx.y - y; 
				dx.z = dx.z - z; 

				C3DFVector px = C3DFVector::_1 - dx; 
				
				cvdebug() << "X: " <<"(" << x << ", " << y << ", " << z <<")"
					  << dx << ": " << px << ", " << value << "\n";  
				
				C3DFVector test_value = 
					px.z * (px.y * ( px.x * f(x,y,z) + dx.x * f(x+1,y,z) ) + 
						dx.y * ( px.x * f(x,y+1,z) + dx.x * f(x+1,y+1,z) )) + 
					
					dx.z * (px.y * ( px.x * f(x,y,z+1) + dx.x * f(x+1,y,z+1)) + 
						dx.y * ( px.x * f(x,y+1,z+1) + dx.x * f(x+1,y+1,z+1))); 

				BOOST_CHECK_CLOSE(value.x, test_value.x, 0.1); 
				BOOST_CHECK_CLOSE(value.y, test_value.y, 0.1); 
				BOOST_CHECK_CLOSE(value.z, test_value.z, 0.1); 
	
			}


	std::set<int> index_set; 
	index_set.insert(0); 
	index_set.insert(2); 
	

	// z \in {0,2}
	for (auto iz: index_set)
		for (size_t iy = 1; iy < n; ++iy) 
			for (size_t ix = 1; ix < n; ++ix) {
				C3DFVector dx(ix / d, iy / d,  iz); 
				C3DFVector value = ip(dx);

				int x = static_cast<int>( floor(dx.x) ); 
				int y = static_cast<int>( floor(dx.y) ); 

				
				dx.y = dx.y - y; 
				dx.x = dx.x - x; 
				
				C3DFVector px = C3DFVector::_1 - dx; 
				
				cvdebug() << "X: " << dx << ": " << px << ", " << value << "\n";  
				
				C3DFVector test_value = 
					(px.y * ( px.x * f(x,y,iz)   + dx.x * f(x+1,y,iz) ) + 
					 dx.y * ( px.x * f(x,y+1,iz) + dx.x * f(x+1,y+1,iz) )); 
				
				BOOST_CHECK_CLOSE(value.x, test_value.x, 0.1); 
				BOOST_CHECK_CLOSE(value.y, test_value.y, 0.1); 
				BOOST_CHECK_CLOSE(value.z, test_value.z, 0.1); 
				
			}
	
	// y \in {0,1}
	for (size_t iz = 1; iz < n; ++iz) 
		for (auto iy : index_set)
			for (size_t ix = 1; ix < n; ++ix) {
				
				C3DFVector dx(ix / d, iy, iz / d); 
				C3DFVector value = ip(dx);
				
				int x = static_cast<int>( floor(dx.x) ); 
				int z = static_cast<int>( floor(dx.z) ); 

				dx.z = dx.z - z; 
				dx.x = dx.x - x; 
				
				C3DFVector px = C3DFVector::_1 - dx; 
				
				cvdebug() << "X: " << dx << ": " << px << ", " << value << "\n";  
				
				C3DFVector test_value = 
					px.z * ( px.x * f(x,iy,z) + dx.x * f(x+1,iy,z) ) + 
					dx.z * ( px.x * f(x,iy,z+1) + dx.x * f(x+1,iy,z+1)); 
				
				BOOST_CHECK_CLOSE(value.x, test_value.x, 0.1); 
				BOOST_CHECK_CLOSE(value.y, test_value.y, 0.1); 
				BOOST_CHECK_CLOSE(value.z, test_value.z, 0.1); 
				
			}
	
	for (size_t iz = 1; iz < n; ++iz) 
		for (size_t iy = 1; iy < n; ++iy) 
			for (auto ix : index_set) {
				C3DFVector dx(ix, iy / d, iz / d); 
				C3DFVector value = ip(dx);
				
				int y = static_cast<int>( floor(dx.y) ); 
				int z = static_cast<int>( floor(dx.z) ); 

				dx.z = dx.z - z; 
				dx.y = dx.y - y; 
				
				C3DFVector px = C3DFVector::_1 - dx; 
				
				cvdebug() << "X: " << dx << ": " << px << ", " << value << "\n";  
				
				C3DFVector test_value = 
					px.z * (px.y * f(ix,y,z) +  dx.y *  f(ix,y+1,z) ) + 
					dx.z * (px.y * f(ix,y,z+1) + dx.y * f(ix,y+1,z+1) ); 
				
				BOOST_CHECK_CLOSE(value.x, test_value.x, 0.1); 
				BOOST_CHECK_CLOSE(value.y, test_value.y, 0.1); 
				BOOST_CHECK_CLOSE(value.z, test_value.z, 0.1); 
				
			}
}

BOOST_AUTO_TEST_CASE (test_vectorfield_as_inverse_of) 
{
	C3DBounds size(10,10,10); 
	
	C3DFVectorfield other(size); 
	
	for (auto io = other.begin_range(C3DBounds::_0, size); 
	     io != other.end_range(C3DBounds::_0, size); ++io)  {
		*io = C3DFVector(cos(io.pos().x * M_PI / 9), cos(io.pos().y * M_PI / 9), cos(io.pos().z * M_PI / 9));
	}
		
	C3DFVectorfield me(size); 
	vectorfield_as_inverse_of(me, other, 1e-5f, 30); 

	
}



NS_MIA_USE


