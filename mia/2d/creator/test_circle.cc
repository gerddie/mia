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
                           float,
                           double
#ifdef HAVE_INT64
                           mia_int64,
                           mia_uint64,
#endif
                           > type_list;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_circle_create, T , type_list )
{
        auto ccreator = BOOST_TEST_create_from_plugin<C2DCircleCreatorPlugin>("circle:f=2,p=3");

        C2DBounds size(16,32); 
        P2DImage image = (*ccreator)(size, pixel_type<T>::value);

        const T2DImage<T>& cc = dynamic_cast<const T2DImage<T>&>(*image); 

        BOOST_CHECK_EQUAL(cc.get_size(), size);

        // check the pattern

        
}



