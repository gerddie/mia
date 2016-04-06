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

#include <mia/core/singular_refobj.hh>
#include <mia/internal/autotest.hh>

using namespace mia; 

BOOST_AUTO_TEST_CASE ( test_singlular_refobj )
{
        TSingleReferencedObject<int> myobj1(1); 
        TSingleReferencedObject<int> myobj2(2); 

        TSingleReferencedObject<int> myotherobje(myobj1); 
        
        BOOST_CHECK_EQUAL(myotherobje, 1); 
        BOOST_CHECK_EQUAL(myobj1.get_refcount(), 2u); 

        myotherobje = myobj2; 
        BOOST_CHECK_EQUAL(myobj1.get_refcount(), 1u); 
        
        BOOST_CHECK_EQUAL(myotherobje, 2);
        BOOST_CHECK_EQUAL(myobj2.get_refcount(), 2u); 
        
}

BOOST_AUTO_TEST_CASE ( test_singlular_refobj_empty )
{
	TSingleReferencedObject<int> myobj; 
	BOOST_CHECK_EQUAL(myobj.get_refcount(), 0u); 

	TSingleReferencedObject<int> myobj2(myobj); 
	BOOST_CHECK_EQUAL(myobj.get_refcount(), 0u); 
}

BOOST_AUTO_TEST_CASE ( test_singlular_refobj_empty_then_copy )
{
	TSingleReferencedObject<int> myobj; 
	BOOST_CHECK_EQUAL(myobj.get_refcount(), 0u); 

	TSingleReferencedObject<int> myobj2(1); 
	myobj = myobj2; 
	BOOST_CHECK_EQUAL(myobj.get_refcount(), 2u); 

	myobj2 = TSingleReferencedObject<int>(); 
	BOOST_CHECK_EQUAL(myobj2.get_refcount(), 0u);
	BOOST_CHECK_EQUAL(myobj.get_refcount(), 1u); 
	
}
