/* -*- mia-c++  -*-
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
#include <mia/2d/cost/fatngf.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace nfg_2dimage_fatcost;

CSplineKernelTestPath spline_kernel_path_init; 

const size_t g_nx = 4;
const size_t g_ny = 3;


class CCostEvaluatorTest: public CCostEvaluator {
public:
	typedef CCostEvaluator::param_pass param_pass;
	virtual const  char *do_get_name() const;
	virtual float get_cost(size_t x,param_pass& p) const;
	virtual float get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const;
};


const  char *CCostEvaluatorTest::do_get_name() const
{
	return "test";
}

float CCostEvaluatorTest::get_cost(size_t x,param_pass& /*p*/) const
{
	return x;
}

float CCostEvaluatorTest::get_cost_grad(size_t x,param_pass& /*p*/, C2DFVectorfield::iterator g)const
{
	g[x].x = x;
	g[x].y = 2.0;
	return x;
}



class TestEvaluators {
public:
        TestEvaluators();
        void run(const CCostEvaluator& eval, float  test_cost, const C2DFVector& testgrad) const;
private:
        C2DFVectorfield r, s;
};

TestEvaluators::TestEvaluators():
        r(C2DBounds(g_nx,g_ny)),
	s(C2DBounds(g_nx,g_ny))
{
	TRACE("TestEvaluators::TestEvaluators");
        const size_t size = g_nx * g_ny;
        const float  rx_init[size] = { 0, 2, 3, 0,  3, 1, 2, 0,  0, 2, 0, 0 };
        const float  ry_init[size] = { 0, 2, 0, 2,  1, 3, 0, 2,  1, 1, 2, 2 };

        const float  sx_init[size] = { 0, 2, 0, 1,  1, 2, 0, 1,  1, 1, 2, 1 };
        const float  sy_init[size] = { 1, 4, 2, 2,  1, 2, 2, 2,  2, 2, 1, 2 };

	C2DFVectorfield::iterator ri = r.begin();
	C2DFVectorfield::iterator si = s.begin();

	for (size_t i =0; i < size; ++i, ++ri, ++si) {
		*ri = C2DFVector( rx_init[i], ry_init[i]);
		*si = C2DFVector( sx_init[i], sy_init[i]);
	}

}

void TestEvaluators::run(const CCostEvaluator& eval, float  test_cost, const C2DFVector& testgrad) const
{
        CCostEvaluator::param_pass p;

	p.ref = r.begin_at(0,1);
	p.src = s.begin_at(0,1);


        p.srcp = s.begin_at(0,2);
	p.srcm = s.begin_at(0,0);

	C2DFVectorfield grad(C2DBounds(g_nx,1));
	C2DFVectorfield::iterator gout = grad.begin();

        BOOST_CHECK_CLOSE(eval.get_cost(1, p), test_cost, float (0.1));
        BOOST_CHECK_CLOSE(eval.get_cost_grad(1, p, gout), test_cost, float (0.1));
        BOOST_CHECK_CLOSE(gout[1].x, testgrad.x, float (0.1));
        BOOST_CHECK_CLOSE(gout[1].y, testgrad.y, float (0.1));

}

// crazy, but this test needs a test ;)
BOOST_FIXTURE_TEST_CASE( test_testEvaluator, TestEvaluators )
{
	run(CCostEvaluatorTest(), 1, C2DFVector(1.0, 2.0));
}

BOOST_FIXTURE_TEST_CASE( test_CostEvaluatorsCross, TestEvaluators  )
{
	CCostEvaluatorCross cross;
        run(cross, 16, C2DFVector(-  16 , -  4));
}

BOOST_FIXTURE_TEST_CASE( test_CostEvaluatorsScalar, TestEvaluators  )
{
        CCostEvaluatorScalar scalar;
        run(scalar, - 64,  C2DFVector( 8 * 2, -  8 * 7));
}

BOOST_FIXTURE_TEST_CASE( test_CostEvaluatorsSQDelta, TestEvaluators )
{
        CCostEvaluatorSQDelta delta;
        run(delta,  36 * 18, C2DFVector(- 32 * 36,   36 * 16 * 7));
}

BOOST_FIXTURE_TEST_CASE( test_CCostEvaluatorDeltaScalar, TestEvaluators )
{
        CCostEvaluatorDeltaScalar ds;
        run(ds, 4.0458, C2DFVector(-5.0885, 10.177));
}



BOOST_AUTO_TEST_CASE( test_fatngf_2d )
{
	const float weight = 1.0;
	const C2DBounds size(10,20);

	P2DImage src(new C2DUBImage(size));
	P2DImage ref(new C2DUBImage(size));


	PEvaluator eval(new CCostEvaluatorTest());
	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3, bc_mirror_on_bounds));

	CFatNFG2DImageCost cost(src, ref, ipf, weight, eval);
	double test_cost = 0.5 * weight * 18 * 36 / 200.0;

	BOOST_CHECK_CLOSE(cost.value(), test_cost, 0.1);

	C2DFVectorfield force(size);

	double c = cost.evaluate_force(force);
	BOOST_CHECK_CLOSE(c, test_cost, 0.1);

	const C2DFVector boundelm(0,0);

	C2DFVectorfield::const_iterator iforce = force.begin();
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++iforce) {
			if (x == 0 || x == size.x - 1 ||
			    y == 0 || y == size.y - 1) {
				BOOST_CHECK_EQUAL(*iforce, boundelm);
			} else {
				BOOST_CHECK_EQUAL(*iforce, C2DFVector(x,2.0));
			}
		}
}
