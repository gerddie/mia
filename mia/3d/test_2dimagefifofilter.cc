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

#define BOOST_TEST_MODULE 2DIMAGEFIFOFILTER_TEST

#include <cassert>
#include <iostream>
#include <cmath>
#include <numeric>
#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/2dimagefifofilter.hh>

using namespace std;
using namespace mia;
namespace bfs=::boost::filesystem;

BOOST_AUTO_TEST_CASE( test_found_filters )
{
}
