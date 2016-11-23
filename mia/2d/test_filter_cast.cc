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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/2d/filter.hh>

#include <boost/test/test_case_template.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/list.hpp>


NS_MIA_USE
using namespace std;
namespace bmpl=boost::mpl;

struct FTestFilter : public TFilter<bool> {
	template <typename T>
	bool operator () (const T2DImage<T>& image) const {
		return image.size() != 0; 
	}
	
}; 

typedef bmpl::vector<bool,
		     int8_t,
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

BOOST_AUTO_TEST_CASE_TEMPLATE( test_test_type, T, test_types )
{	
	T2DImage<T> timage(C2DBounds(1,1)); 
	const C2DImage& image = timage; 
	FTestFilter filter; 
	
	BOOST_CHECK(mia::filter(filter, image)); 
}
