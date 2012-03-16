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



#include <iomanip>
#include <mia/internal/plugintester.hh> 
#include <mia/3d/combiner/labelxmap.hh>

using namespace labelxmap_3dimage_filter; 
using namespace mia; 
using namespace std; 

BOOST_AUTO_TEST_CASE ( test_labelxmap ) 
{
	auto f = BOOST_TEST_create_from_plugin<CLabelXMapPlugin>("labelxmap");

	const int init_data1[10]   = {0, 0, 1, 2, 3, 4, 2, 1, 2, 0};
	const short init_data2[10] = {1, 0, 2, 1, 2, 3, 4, 2, 1, 1};

	CXLabelResult right_answer;

	right_answer.add(1,2);
	right_answer.add(1,2);
	right_answer.add(2,1);
	right_answer.add(2,1);
	right_answer.add(3,2);
	right_answer.add(4,3);
	right_answer.add(2,4);


	P3DImage image(new C3DUSImage(C3DBounds(1,1,1)));
	P3DImage fimage(new C3DFImage(C3DBounds(10,1,1)));

	P3DImage int_image(new C3DSIImage(C3DBounds(10,1,1), init_data1));
	P3DImage short_image(new C3DSSImage(C3DBounds(10,1,1), init_data2));

	BOOST_CHECK_THROW(f->combine( *image, *short_image), invalid_argument); 
	BOOST_CHECK_THROW(f->combine( *fimage, *short_image), invalid_argument); 

	C3DImageCombiner::result_type result = f->combine( *int_image, *short_image);

	const  CXLabelResult& xlr = dynamic_cast<const  CXLabelResult&>(*result);
	BOOST_CHECK(xlr == right_answer);

}
