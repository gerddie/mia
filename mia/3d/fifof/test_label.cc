/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
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
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/fifof/label.hh>

BOOST_FIXTURE_TEST_CASE( test_fifof_label , RegiongrowFixture )
{
	const size_t n_slices = 5; 
	const C2DBounds size(6,6); 
	
	unsigned char input_data[n_slices * 6 * 6] = { 
		  0, 5, 0,   0, 7, 0,   0, 0, 0 , 
		  0, 1, 0,   0, 6, 0,   0, 0, 0 , 
		  0, 2, 0,   8, 9, 7,   0, 6, 6 , 
		  0, 3, 0,   0, 7, 0,   0, 0, 6 , 
		  6, 4, 0,   0, 0, 0,   0, 0, 6 
	};

	bool test_data[n_slices * 6 * 6] = {
		  0, 0, 0,   0, 1, 0,   0, 0, 0, 
		  0, 0, 0,   0, 1, 0,   0, 0, 0, 
		  0, 0, 0,   1, 1, 1,   0, 1, 1, 
		  0, 0, 0,   0, 1, 0,   0, 0, 1, 
		  0, 0, 0,   0, 0, 0,   0, 0, 1 
	};

	prepare(input_data, test_data, size, n_slices); 

	C2DLabelFifoFilter filter;
	call_test(filter);

}

