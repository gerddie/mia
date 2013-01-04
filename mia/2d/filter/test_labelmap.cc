/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/2d/filter/labelmap.hh>

using namespace labelmap_2dimage_filter; 
NS_MIA_USE;
using namespace std; 

BOOST_AUTO_TEST_CASE (test_labelmap) 
{
	C2DUSImage *src = new C2DUSImage(C2DBounds(8,32));
	C2DUSImage ref(C2DBounds(8,32));
	
	C2DUSImage::iterator is = src->begin();
	C2DUSImage::iterator ir = ref.begin();
	
	for (size_t i = 0; i < src->size(); ++i, ++is, ++ir)
		*is = *ir = i;
	
	CLabelMap map;
	map[1] = 2; 
	map[2] = 4;
	map[23] = 7;
	map[189] = 10;
	
	ref(1,0) = 2; 
	ref(2,0) = 4;
	ref(7,2) = 7; 
	ref(5,23) = 10; 
	
	P2DImage wsrc(src); 

	C2DLabelMapFilter filter(map); 
	
	P2DImage wres = filter.filter(*wsrc); 
	const C2DUSImage& result = dynamic_cast<const C2DUSImage&>(*wres); 
	
	BOOST_CHECK_EQUAL(result.get_size(), ref.get_size()); 
	BOOST_CHECK(equal(result.begin(), result.end(), ref.begin())); 

}						
