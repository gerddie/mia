/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Genoa 1999-2016 Gert Wollny
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

#include <mia/core/convergence_measure.hh>
#include <mia/internal/autotest.hh>

#include <limits>


BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_0 )
{
        mia::CConvergenceMeasure cm(4);

        BOOST_CHECK_EQUAL( cm.value(), std::numeric_limits<double>::max());
        BOOST_CHECK( !cm.is_full_size()); 
        BOOST_CHECK_EQUAL( cm.fill(), 0u);
	BOOST_CHECK_EQUAL( cm.rate(), 0.0);
}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_1 )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(1.0);
        BOOST_CHECK( !cm.is_full_size());
        BOOST_CHECK_EQUAL( cm.fill(), 1u); 
        BOOST_CHECK_EQUAL( cm.value(), 1.0);
	BOOST_CHECK_EQUAL( cm.rate(), 0.0);
}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_2 )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(1.0);
        cm.push(2.0);
        BOOST_CHECK( !cm.is_full_size());
        BOOST_CHECK_EQUAL( cm.fill(), 2u); 
        BOOST_CHECK_CLOSE( cm.value(), 1.5, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), 1.0, 1e-8);

}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_3 )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(1.0);
        cm.push(2.0);
        cm.push(3.0);
        BOOST_CHECK( !cm.is_full_size());
        BOOST_CHECK_EQUAL( cm.fill(), 3u); 
        BOOST_CHECK_CLOSE( cm.value(), 2.0, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), 1.0, 1e-8);

}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_4 )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(1.0);
        cm.push(2.0);
        cm.push(3.0);
	cm.push(4.0);
        BOOST_CHECK( cm.is_full_size());
        BOOST_CHECK_EQUAL( cm.fill(), 4u); 
        BOOST_CHECK_CLOSE( cm.value(), 2.5, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), 1.0, 1e-8);

        
}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_5 )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(1.0);
        cm.push(2.0);
        cm.push(3.0);
	cm.push(4.0);
	cm.push(5.0);
        BOOST_CHECK( cm.is_full_size());
        BOOST_CHECK_EQUAL( cm.fill(), 4u); 
        BOOST_CHECK_CLOSE( cm.value(), 3.5, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), 1.0, 1e-8);

        
}

BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_inv )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(90);
        cm.push(10);
        BOOST_CHECK_EQUAL( cm.fill(), 2u); 
        BOOST_CHECK_CLOSE( cm.value(), 50, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), -80, 1e-8);
}


BOOST_AUTO_TEST_CASE ( test_CConvergenceMeasure_inv_big )
{
        mia::CConvergenceMeasure cm(4);
        cm.push(5.1401e+07);
        cm.push(5.01538e+07);
        BOOST_CHECK_EQUAL( cm.fill(), 2u); 
        BOOST_CHECK_CLOSE( cm.value(), (5.1401e+07 + 5.01538e+07)/2, 1e-8);
	BOOST_CHECK_CLOSE( cm.rate(), 5.01538e+07 - 5.1401e+07, 1e-8);
}

