/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2009 - 2010
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/internal/autotest.hh>

#include <stdexcept>
#include <cmath>
#include <iomanip>

#include <mia/core/property_flags.hh>


using namespace std;
using namespace mia;

BOOST_AUTO_TEST_CASE( test_has_flag )
{
	const char *flag1 = "flag1"; 
	const char *flag2 = "flag2"; 

	CPropertyFlagHolder holder; 
	
	holder.add(flag1); 
	BOOST_CHECK(holder.has(flag1)); 
	BOOST_CHECK(!holder.has(flag2)); 
	holder.add(flag2); 
	BOOST_CHECK(holder.has(flag2)); 
}

