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
#include <mia/core/cmdoptionflags.hh>


NS_MIA_USE; 

BOOST_AUTO_TEST_CASE( test_some_combinations ) 
{
        BOOST_CHECK_EQUAL(CCmdOptionFlags::required_output & CCmdOptionFlags::output, CCmdOptionFlags::output); 
        BOOST_CHECK_EQUAL(CCmdOptionFlags::required_input & CCmdOptionFlags::input, CCmdOptionFlags::input); 

        BOOST_CHECK_EQUAL(CCmdOptionFlags::input | CCmdOptionFlags::required, CCmdOptionFlags::required_input);
        BOOST_CHECK_EQUAL(CCmdOptionFlags::output | CCmdOptionFlags::required, CCmdOptionFlags::required_output);


        BOOST_CHECK(has_flag(CCmdOptionFlags::required_output, CCmdOptionFlags::output)); 
        BOOST_CHECK(has_flag(CCmdOptionFlags::required_input, CCmdOptionFlags::input)); 

}

BOOST_AUTO_TEST_CASE( test_some_ops ) 
{
        CCmdOptionFlags flags = CCmdOptionFlags::required_output; 
        
        flags -= CCmdOptionFlags::output; 
        BOOST_CHECK_EQUAL(flags, CCmdOptionFlags::required); 
}
