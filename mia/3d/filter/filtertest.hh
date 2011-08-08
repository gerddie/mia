/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <boost/shared_ptr.hpp>
#include <mia/internal/autotest.hh>
#include <mia/3d/3dfilter.hh>


#ifndef mia_3d_filtertest_hh
#define mia_3d_filtertest_hh

struct FilterTestFixtureBase {
	FilterTestFixtureBase(const mia::C3DBounds& s): 
		size(s)
	{
	}
	
	template <typename T> 
	void check_mask_result(mia::P3DImage result, const T *test_data, const C3DBounds& test_size)
	{
		const mia::T3DImage<T> *r =  dynamic_cast< const mia::T3DImage<T> *>(result.get());
		
		BOOST_REQUIRE(r); 
		
		BOOST_CHECK_EQUAL(r->get_size(), test_size); 
		BOOST_REQUIRE(r->get_size()== test_size); 
		
		const T *t = test_data; 
		typename mia::T3DImage<T>::const_iterator i = r->begin(); 
		for (size_t z = 0; z < test_size.z; ++z) 
			for (size_t y = 0; y < test_size.y; ++y) 
				for (size_t x = 0; x < test_size.x; ++x, ++t, ++i) {
					if (*i !=  *t) 
						cverr() << x  << ", " << y << ", " << z << "\n"; 
					BOOST_CHECK_EQUAL(*i, *t); 
				}
        } 
	
	mia::C3DBounds size; 
}; 

#endif
