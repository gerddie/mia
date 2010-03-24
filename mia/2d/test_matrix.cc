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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/internal/autotest.hh>
#include <mia/2d/matrix.hh>

struct MatrixFixture {

	MatrixFixture();

	C2DFMatrix m;
	C2DFVector x;

};

MatrixFixture::MatrixFixture():
	m(C2DFVector(1.0, 2.0),C2DFVector(4.0, 3.0)),
	x(1.4, 2.3)
{
}


BOOST_FIXTURE_TEST_CASE(test_mult1, MatrixFixture)
{
	C2DFVector y = x * m;

	BOOST_CHECK_CLOSE(y.x, 6.0f, 0.1);
	BOOST_CHECK_CLOSE(y.y, 12.5f, 0.1);
}

BOOST_FIXTURE_TEST_CASE(test_mult2, MatrixFixture)
{
	C2DFVector y = m * x;

	BOOST_CHECK_CLOSE(y.x, 10.6f, 0.1);
	BOOST_CHECK_CLOSE(y.y, 9.7f, 0.1);
}
