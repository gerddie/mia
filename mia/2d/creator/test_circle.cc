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
#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>

#include <mia/2d/creator/circle.hh>

using namespace mia;
using namespace creator_circle_2d;


typedef boost::mpl::vector<bool,
                           signed char,
                           unsigned char,
                           signed short,
                           unsigned short,
                           signed int,
                           unsigned int,
                           double
#ifdef HAVE_INT64
                           mia_int64,
                           mia_uint64,
#endif
                           > type_list;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_circle_create, T , type_list )
{
        auto ccreator = BOOST_TEST_create_from_plugin<C2DCircleCreatorPlugin>("circle:f=2,p=3");
        
        C2DBounds size(16,16); 
        P2DImage image = (*ccreator)(size, pixel_type<T>::value);
        
        const T2DImage<T>& cc = dynamic_cast<const T2DImage<T>&>(*image); 
        
        BOOST_CHECK_EQUAL(cc.get_size(), size);
}

/// test the pattern only once, for the float case
BOOST_AUTO_TEST_CASE( test_circle_create_float )
{
        auto ccreator = BOOST_TEST_create_from_plugin<C2DCircleCreatorPlugin>("circle:f=2,p=3");
	
        C2DBounds size(16,16); 
        P2DImage image = (*ccreator)(size, it_float);
	
        const C2DFImage& cc = dynamic_cast<const C2DFImage&>(*image); 

        BOOST_CHECK_EQUAL(cc.get_size(), size);

	vector<float> result= {
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0.995185,
		0.989177, 0.995185, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 1, -0.24298, -0.92388, -0.998795,
		-1, -0.998795, -0.92388, -0.24298, 1, 0, 0, 0, 
		0, 0, 0, 0, -0.24298, -0.881921, -0.14673, 0.19509,
		0.24298, 0.19509, -0.14673, -0.881921, -0.24298, 0, 0, 0, 
		0, 0, 0, 0, -0.92388, -0.14673, 0.707107, 0.903989,
		0.92388, 0.903989, 0.707107, -0.14673, -0.92388, 0, 0, 0, 
		0, 0, 0, 0.995185, -0.998795, 0.19509, 0.903989,
		0.995185, 0.998795, 0.995185, 0.903989, 0.19509, -0.998795, 0.995185, 0, 0, 
		0, 0, 0, 0.989177, -1, 0.24298, 0.92388, 0.998795,
		1, 0.998795, 0.92388, 0.24298, -1, 0.989177, 0, 0, 
		0, 0, 0, 0.995185, -0.998795, 0.19509, 0.903989, 0.995185,
		0.998795, 0.995185, 0.903989, 0.19509, -0.998795, 0.995185, 0, 0, 
		0, 0, 0, 0, -0.92388, -0.14673, 0.707107, 0.903989, 0.92388,
		0.903989, 0.707107, -0.14673, -0.92388, 0, 0, 0, 
		0, 0, 0, 0, -0.24298, -0.881921, -0.14673, 0.19509, 0.24298,
		0.19509, -0.14673, -0.881921, -0.24298, 0, 0, 0, 
		0, 0, 0, 0, 1, -0.24298, -0.92388, -0.998795, -1,
		-0.998795, -0.92388, -0.24298, 1, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0.995185, 0.989177,
		0.995185, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}; 

	auto ic = cc.begin();
	auto ec = cc.end();
	auto ir = result.begin();

	while (ic != ec) {

		if (fabs(*ir) > 1e-5)
			BOOST_CHECK_CLOSE(*ic, *ir, 0.1);
		else
			BOOST_CHECK_SMALL(*ic, 1e-5f);
		
		++ic; ++ir; 
	}; 
}



