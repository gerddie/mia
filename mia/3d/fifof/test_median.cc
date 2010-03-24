/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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


/*
  \author Gert Wollny <gerddie at gmail.com>
*/


#include <mia/internal/autotest.hh>
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/fifof/median.hh>

NS_USE(median_2dstack_filter);
NS_MIA_USE;

BOOST_FIXTURE_TEST_CASE( test_fifof_gauss , fifof_Fixture )
{
	const size_t n_slices = 3;
	const C2DBounds size(3,3);

	float input_data[n_slices * 9] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9,
		11, 12, 13, 14, 15, 16, 17, 18, 19,
		21, 22, 23, 24, 25, 26, 27, 28, 29
	};

	float test_data[n_slices * 9] = {
		 8, 8.5, 9, 9.5,  10, 10.5, 11, 11.5, 12,
		 13,  13.5, 14, 14.5, 15, 15.5, 16, 16.5, 17,
		 18, 18.5, 19, 19.5, 20, 20.5, 21, 21.5, 22
	};

	prepare(input_data, test_data, size, n_slices);
	C2DMedianFifoFilter f(1);

	call_test(f);
}

