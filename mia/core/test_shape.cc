/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
#include <mia/core/shape.hh>
#include <mia/core/shape.cxx>


template <typename T> 
struct pass_type {
	typedef T value_type; 
	T x; 
}; 

namespace mia {

	template <typename T>
	struct less_then<pass_type<T> > {
		bool operator() (const pass_type<T>& a, const pass_type<T>& b) const {
			return a.x < b.x;
		}
	}; 

	template <>
	struct __adjust<pass_type<int> > {
		static void apply(pass_type<int>& size, const pass_type<int>& p)  {
			int x = (p.x < 0 ? -2 * p.x : 2 *  p.x) + 1;
			
			if (size.x < x)
				size.x = x;
			
		}
	};

}


NS_MIA_USE; 
struct vector_size_type {
	unsigned int x; 
}; 


struct TestVector : public std::vector<bool> {
	typedef int Super; 

	vector_size_type get_size() const {
		vector_size_type s; 
		s.x = size(); 
		return s; 
	}
}; 


typedef TShape<pass_type, TestVector > C1DShape; 

BOOST_AUTO_TEST_CASE( test_shape_has_location ) 
{
	pass_type<int> x = {1}; 
	pass_type<int> y = {2}; 
	C1DShape shape; 
	shape.insert(x); 
	
	BOOST_CHECK_EQUAL(shape.size(), 1u); 

	BOOST_CHECK(shape.has_location(x)); 
	BOOST_CHECK(!shape.has_location(y));
}


