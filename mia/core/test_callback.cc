/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/internal/autotest.hh>
#include <mia/core/callback.hh>

NS_MIA_USE; 

BOOST_AUTO_TEST_CASE( test_callback )
{
	CMsgStreamPrintCallback cb("%1% of %2%"); 
	cb.set_range(100); 
	
	for (size_t i = 0; i < 100; i++) {
		cb.update(i); 
	}
	for (size_t i = 0; i < 10; i++) {
		cb.pulse(); 
	}

}
