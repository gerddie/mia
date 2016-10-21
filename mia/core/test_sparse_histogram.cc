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


#include <mia/internal/autotest.hh>
#include <mia/core/sparse_histogram.hh>

NS_MIA_USE
using std::vector;
using std::invalid_argument;

BOOST_AUTO_TEST_CASE( test_uint8 )
{
        vector <unsigned char> input = {1, 1, 200, 200, 200};

        CSparseHistogram h;

        BOOST_CHECK_EQUAL(h(input.begin(), input.end()), 5u);
        
        auto ch = h.get_compressed_histogram(); 
        BOOST_REQUIRE(ch.size() == 2u);

        BOOST_CHECK_EQUAL( ch[0].first,   1 );
        BOOST_CHECK_EQUAL( ch[0].second,  2 ); 
        BOOST_CHECK_EQUAL( ch[1].first, 200 );
        BOOST_CHECK_EQUAL( ch[1].second,  3 );
        
}

BOOST_AUTO_TEST_CASE( test_sint8 )
{
        vector <signed char> input = {-2, -2, 30, 30, 30};

        CSparseHistogram h;

        BOOST_CHECK_EQUAL(h(input.begin(), input.end()), 5u);
        
        auto ch = h.get_compressed_histogram(); 
        BOOST_REQUIRE(ch.size() == 2u);

        BOOST_CHECK_EQUAL( ch[0].first, -2 );
        BOOST_CHECK_EQUAL( ch[0].second, 2 ); 
        BOOST_CHECK_EQUAL( ch[1].first, 30 );
        BOOST_CHECK_EQUAL( ch[1].second, 3 );
        
}


BOOST_AUTO_TEST_CASE( test_uint16 )
{
        vector <unsigned short> input = {1, 1, 400, 400, 400};

        CSparseHistogram h;

        BOOST_CHECK_EQUAL(h(input.begin(), input.end()), 5u);
        
        auto ch = h.get_compressed_histogram(); 
        BOOST_REQUIRE(ch.size() == 2u);

        BOOST_CHECK_EQUAL( ch[0].first,   1 );
        BOOST_CHECK_EQUAL( ch[0].second,  2 ); 
        BOOST_CHECK_EQUAL( ch[1].first, 400 );
        BOOST_CHECK_EQUAL( ch[1].second,  3 );
        
}

BOOST_AUTO_TEST_CASE( test_sint16 )
{
        vector <signed short> input = {-200, -200, 300, 300, 300};

        CSparseHistogram h;

        BOOST_CHECK_EQUAL(h(input.begin(), input.end()), 5u);
        
        auto ch = h.get_compressed_histogram();
        BOOST_CHECK_EQUAL(ch.size(), 2u);
        BOOST_REQUIRE(ch.size() == 2u);

        BOOST_CHECK_EQUAL( ch[0].first, -200 );
        BOOST_CHECK_EQUAL( ch[0].second,   2 ); 
        BOOST_CHECK_EQUAL( ch[1].first,  300 );
        BOOST_CHECK_EQUAL( ch[1].second,   3 );
        
}

BOOST_AUTO_TEST_CASE( test_unsupported )
{
        CSparseHistogram hsi;
        vector <signed int> si = { -64000, -64000, 64002, 64002 }; 
        BOOST_CHECK_THROW(hsi(si.begin(), si.end()), invalid_argument);

        CSparseHistogram hui;
        vector <unsigned int> ui = { 64000, 64000, 64002, 64002 }; 
        BOOST_CHECK_THROW(hui(ui.begin(), ui.end()), invalid_argument);

        CSparseHistogram hf;
        vector <float> f = { -1.0f, -1.1f, 2.0f, 3.0f }; 
        BOOST_CHECK_THROW(hf(si.begin(), si.end()), invalid_argument);

        CSparseHistogram hd;
        vector <double> d = { -1.0, -1.1, 2.0, 3.0 }; 
        BOOST_CHECK_THROW(hd(d.begin(), d.end()), invalid_argument);
       
}

BOOST_AUTO_TEST_CASE( test_mix_types )
{
        vector <signed short> input_ss = {-200, -200, 300, 300, 300};
        vector <unsigned short> input_us = {200, 200, 400, 400, 400};
        
        CSparseHistogram h;

        BOOST_CHECK_EQUAL(h(input_ss.begin(), input_ss.end()), 5u);
        BOOST_CHECK_THROW(h(input_us.begin(), input_us.end()), invalid_argument);
}

