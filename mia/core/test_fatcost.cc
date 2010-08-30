/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/transformation.hh>

#include <mia/core/fatcost.cxx>

NS_MIA_USE
using namespace std;
using namespace boost;
using namespace boost::unit_test;



struct TestInt {
	int value;
	typedef float dimsize_type;
	TestInt(int i):value(i){}
};

TestInt operator - (const TestInt& ti, int a)
{
	return TestInt(ti.value - a);
}

TestInt operator - (const TestInt& ti, const TestInt& tb)
{
	return TestInt(ti.value - tb.value);
}

float operator * (float f, const TestInt& ti)
{
	return f * ti.value;
}

typedef std::shared_ptr<TestInt > PTestInt;

class TestTransformation: public Transformation<TestInt, TestInt> {
        virtual PTestInt apply(const TestInt& input, const TestInt& /*dummy*/) const {
		return PTestInt(new TestInt(input - 1));
	}
};


class TTestFatCost: public TFatCost<TestTransformation, float>
{
public:
	TTestFatCost(PTestInt a, PTestInt b, PTestInt interp, float w);
private:
	typedef TestTransformation::Data MyData;
	virtual double do_value() const;
	virtual double do_evaluate_force(float& force) const;
};

TTestFatCost::TTestFatCost(PTestInt a, PTestInt b, PTestInt interp, float w):
	TFatCost<TestTransformation, float>(a,b,interp,w)
{}

double TTestFatCost::do_value() const
{
	return (get_floating() - get_ref()).value;
}

double TTestFatCost::do_evaluate_force( float& force) const
{
	force += get_weight() * (get_ref() - get_floating());
	return value();
}


BOOST_AUTO_TEST_CASE( test_fatcost_basics)
{
	PTestInt a(new TestInt(1));
	PTestInt b(new TestInt(2));
	float force = 1.0f;
	PTestInt interp(new TestInt(1));

	TTestFatCost cost(a, b, interp, 0.5f);

	BOOST_CHECK_EQUAL(cost.get_src().value, 1);
	BOOST_CHECK_EQUAL(cost.get_ref().value, 2);
	BOOST_CHECK_EQUAL(cost.get_weight(), 0.5f);


	BOOST_CHECK_CLOSE(cost.value(), -0.5, 0.0001);
	BOOST_CHECK_CLOSE(cost.evaluate_force(force), -0.5, 0.0001);

	BOOST_CHECK_CLOSE(force, 1.5f, 0.001);

	TestTransformation tt;

	cost.transform(tt);

	BOOST_CHECK_CLOSE(cost.value(), -1.0, 0.0001);


}

