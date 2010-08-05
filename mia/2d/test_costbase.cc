/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include <mia/core/shared_ptr.hh>


#include <mia/2d/costbase.hh>
#include <mia/2d/transformmock.hh>

NS_MIA_USE;


struct C2DCostTest: public C2DCostBase {

	C2DCostTest(float weight): C2DCostBase(weight){}
private:
	double do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const;

};


BOOST_AUTO_TEST_CASE (test_costbase )
{
	const C2DBounds size(1,1);
	C2DCostTest cost(1.0);

	C2DFVectorfield force(size);

	C2DTransformMock t(size);

	BOOST_CHECK_EQUAL(cost.evaluate(t, force), 1.0);
	BOOST_CHECK_EQUAL(force(0,0), C2DFVector(1.2, 2.3));

}

BOOST_AUTO_TEST_CASE (test_costbase_with_weight )
{
	const C2DBounds size(1,1);
	C2DCostTest cost(2.0);

	C2DFVectorfield force(size);

	C2DTransformMock t(size);

	BOOST_CHECK_EQUAL(cost.evaluate(t, force), 2.0);
	BOOST_CHECK_EQUAL(force(0,0), C2DFVector(2.4, 4.6));

}



double C2DCostTest::do_evaluate(const C2DTransformation& t, C2DFVectorfield& force) const
{

	force(0,0) = t(C2DFVector(0.0, 0.0));
	return 1.0;
}
