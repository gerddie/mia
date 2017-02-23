/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/3d/filter/gradnorm.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace gradnorm_3dimage_filter;

BOOST_AUTO_TEST_CASE( test_gradnorm )
{
	const size_t size = 3;
	const float src_data[size * size * size] = {
		1, 1, 1, 
		1, 2, 3, 
                4, 2, 3, 
                
		4, 2, 1,
		2, 4, 1, 
		2, 3, 2,

		5, 3, 6,
		3, 1, 3, 
		2, 2, 2,
		
	};

	const float ref_data[size * size * size] = {
		0,          0,      0, 
	      1.5f, sqrt(1.25f),      1, 
		0,        0.5f,      0, 

		2,         sqrt(3.25f),   2.5f, 
	sqrt(2.0f),  sqrt(0.75f),   0.5f, 
                1,           0,   0.5f, 

                0,         0.5f,     0,
		1.5f,       0.5f,     2, 
		0,         0,       0, 
	};


	C3DFImage *src = new C3DFImage(C3DBounds(size, size, size));

	C3DFImage::iterator f = src->begin();
	for (size_t i = 0; i < size*size*size; ++i, ++f)
		*f = src_data[i];

	P3DImage srcw(src);

	auto filter = BOOST_TEST_create_from_plugin<C3DGradnormFilterPlugin>("gradnorm");	

	auto res = filter->filter(*srcw);

	BOOST_CHECK_EQUAL(res->get_pixel_type(), it_float);
	BOOST_REQUIRE(res->get_pixel_type() == it_float);

	C3DFImage *resi = dynamic_cast<C3DFImage *>(res.get());
	BOOST_REQUIRE(resi);

	BOOST_CHECK_EQUAL(resi->get_size(), src->get_size());
	BOOST_REQUIRE(resi->get_size() == src->get_size());

	f = resi->begin();
	for (size_t i = 0; i < size*size*size; ++i, ++f) {
		BOOST_CHECK_CLOSE(1.0 + *f, 1.0 + ref_data[i]/2.5f, 0.1);
	}
}
