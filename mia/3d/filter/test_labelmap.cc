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

#include <mia/internal/autotest.hh>
#include <mia/3d/filter/labelmap.hh>

NS_MIA_USE;
using namespace std; 

BOOST_AUTO_TEST_CASE (test_labelmap) 
{
	C3DUSImage *src = new C3DUSImage(C3DBounds(8,32, 2));
	C3DUSImage ref(C3DBounds(8,32, 2));
	
	C3DUSImage::iterator is = src->begin();
	C3DUSImage::iterator ir = ref.begin();
	
	for (size_t i = 0; i < src->size(); ++i, ++is, ++ir)
		*is = *ir = i;
	
	CLabelMap map;
	map[1] = 2; 
	map[2] = 4;
	map[23] = 7;
	map[189] = 10;
	
	ref(1,0, 0) = 2; 
	ref(2,0, 0) = 4;
	ref(7,2, 0) = 7; 
	ref(5,23, 0) = 10; 
	
	P3DImage wsrc(src); 

	C3DLabelMapFilter filter(map); 
	
	P3DImage wres = filter.filter(*wsrc); 
	const C3DUSImage& result = dynamic_cast<const C3DUSImage&>(*wres); 
	
	BOOST_CHECK_EQUAL(result.get_size(), ref.get_size()); 
	BOOST_CHECK(equal(result.begin(), result.end(), ref.begin())); 

}						
