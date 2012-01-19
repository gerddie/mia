/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <stdexcept>
#include <climits>
#include <memory>

#include <mia/internal/autotest.hh>
#include <mia/2d/groundtruthproblem.hh>

using namespace mia;

const size_t N = 5;
const C2DBounds slice_size(3,3);
const size_t psize = 5 * 9;


struct GroundTruthAccess: public GroundTruthProblem {
	GroundTruthAccess(const CDoubleVector& left_side,
			   const  CCorrelationEvaluator::result_type& corr);
	void check_spacial_gradient(const double *result);
	void check_time_derivative(const double *result);

private:
	void check_vector_equal(const vector<double>& result, const double *test);
};

struct GroundTruthFixture {
	GroundTruthFixture();

	CDoubleVector left_side;
	std::vector<double> x;
	CCorrelationEvaluator::result_type corr;
	unique_ptr<GroundTruthAccess> pgta;
};

BOOST_FIXTURE_TEST_CASE( test_value, GroundTruthFixture )
{
	BOOST_CHECK_CLOSE(pgta->f(x), 598, 0.01);
}

BOOST_FIXTURE_TEST_CASE( test_value_diff, GroundTruthFixture )
{
	x[4] += 0.1;
	BOOST_CHECK_CLOSE(pgta->f(x), 597.056, 0.01);
	x[4] -= 0.1;
	BOOST_CHECK_CLOSE(pgta->f(x), 598, 0.01);
}


BOOST_FIXTURE_TEST_CASE( test_gradient_only_value, GroundTruthFixture )
{
	pgta->set_alpha_beta(0.0,0.0);

	vector<double> g(psize);
	pgta->df(x, g);
	const double h = 0.0001;

	for(size_t i = 0; i < psize; ++i) {
		vector<double> xp(x);
		vector<double> xm(x);

		xp[i] += h;
		xm[i] -= h;

		// bug in g++ ?
		BOOST_REQUIRE(xp[i] == x[i] + h);
		BOOST_REQUIRE(xm[i] == x[i] - h);

		cvdebug() << x[i] << " xm = " << xm[i] << " xp= " << xp[i] << "\n";

		double fp = pgta->f(xp);
		double fm = pgta->f(xm);

		cvdebug() << "fp= "<< fp << ", fm=" << fm << ", h=" << h << "\n";
		double df = (fp - fm) / (2 * h);

		BOOST_CHECK_CLOSE(g[i], df, 0.1);
	}
}


BOOST_FIXTURE_TEST_CASE( test_gradient_only_spacial_direct, GroundTruthFixture )
{
	pgta->set_alpha_beta(1.0,0.0);

	copy(x.begin(), x.end(), left_side.begin());
	vector<double> g(psize);
	pgta->df(x, g);

	float grad[psize] = {
		-8, 22, -12, -2, -7,  7,  4, -2,  -2,
		-6, 14, -12, -1, -2,  11, 3, -3,  -4,
		-4,  6, -12, -1,  2,  15, 3, -3,  -6,
		-2,  -2,-10, -3,  9,  14, 9, -9,  -6,
		0, 0, 0,  0, 0, 0,  0, 0, 0
	};

	for(size_t i = 0; i < psize; ++i)
		BOOST_CHECK_CLOSE(g[i], grad[i], 0.1);
}

#if 0
BOOST_FIXTURE_TEST_CASE( test_gradient_all_finite_diff, GroundTruthFixture )
{
	pgta->set_alpha_beta(1.0,1.0);


	DoubleVector g(psize);
	pgta->df(x, pgta.get(), g);

	const double h = 0.0001;

	// only test inside
	for(size_t i = 0; i < psize; ++i) {
		DoubleVector xp(x);
		DoubleVector xm(x);

		xp[i] += h;
		xm[i] -= h;

		// bug in g++ ?
		BOOST_REQUIRE(xp[i] == x[i] + h);
		BOOST_REQUIRE(xm[i] == x[i] - h);

		cvdebug() << x[i] << " xm = " << xm[i] << " xp= " << xp[i] << "\n";

		double fp = pgta->f(xp, pgta.get());
		double fm = pgta->f(xm, pgta.get());
		double df = (fp - fm) / (2 * h);

		BOOST_CHECK_CLOSE(g[i], df, 0.1);
	}

}
#endif

BOOST_FIXTURE_TEST_CASE( test_gradient_only_spacial_finite_diff, GroundTruthFixture )
{
	pgta->set_alpha_beta(1.0,0.0);

	copy(x.begin(), x.end(), left_side.begin());
	vector<double> g(psize);
	pgta->df(x, g);

	const double h = 0.0001;

	// only test inside
	for(size_t i = 0; i < psize; ++i) {
		vector<double> xp(x);
		vector<double> xm(x);

		xp[i] += h;
		xm[i] -= h;

		// bug in g++ ?
		BOOST_REQUIRE(xp[i] == x[i] + h);
		BOOST_REQUIRE(xm[i] == x[i] - h);

		cvdebug() << x[i] << " xm = " << xm[i] << " xp= " << xp[i] << "\n";

		double fp = pgta->f(xp);
		double fm = pgta->f(xm);
		double df = (fp - fm) / (2 * h);

		BOOST_CHECK_CLOSE(g[i], df, 0.1);
	}

}

BOOST_FIXTURE_TEST_CASE( test_gradient_only_temporal_direct, GroundTruthFixture )
{
	pgta->set_alpha_beta(0.0,1.0);

	copy(x.begin(), x.end(), left_side.begin());
	vector<double> g(psize);
	pgta->df(x, g);

	float grad[psize] = {
		0,  4, 0,  -4, -4,  -4,  1,  1,   0,
		0, -4, 0,   4,  4,   3,  0, -4,   0,
		0,  0, 0,  -5, -5,  -2,-13,  5,  -1,
		0,  4, 0,  26, 26,  23, 50,  2,  6,
		0, -4, 0, -21,-21, -20, -38, -4, -5
	};

	for(size_t i = 0; i < psize; ++i) {
		BOOST_CHECK_CLOSE(100 + g[i], 100 + grad[i], 0.1);
	}
}

BOOST_FIXTURE_TEST_CASE( test_gradient_only_temporal_finite_diff, GroundTruthFixture )
{
	pgta->set_alpha_beta(0.0,1.0);

	copy(x.begin(), x.end(), left_side.begin());
	vector<double> g(psize);

	pgta->df(x, g);

	const double h = 0.0001;

	// only test inside
	for(size_t i = 0; i < psize; ++i) {
		vector<double> xp(x);
		vector<double> xm(x);

		xp[i] += h;
		xm[i] -= h;

		// bug in g++ ?
		BOOST_REQUIRE(xp[i] == x[i] + h);
		BOOST_REQUIRE(xm[i] == x[i] - h);

		cvdebug() << x[i] << " xm = " << xm[i] << " xp= " << xp[i] << "\n";

		double fp = pgta->f(xp);
		double fm = pgta->f(xm);
		double df = (fp - fm) / (2 * h);

		cvmsg() << "fp["<<i<<"]= "<< fp << ", fm=" << fm << ", h=" << h
			<< " g=" << g[i] << ", df = " << df << "\n";


		BOOST_CHECK_CLOSE(100 + g[i], 100 + df, 0.1);
	}
}

void GroundTruthAccess::check_vector_equal(const vector<double>& result, const double *test)
{
	BOOST_REQUIRE(psize == result.size());
	size_t i = 0;
	auto t = test;
	for(auto sg = result.begin(); sg !=result.end(); ++t, ++sg, ++i) {
		if (*sg != *t)
			cvfail() << i << "\n";
		BOOST_CHECK_CLOSE(*sg, *t, 0.1);
	}
}

GroundTruthAccess::GroundTruthAccess(const CDoubleVector& left_side,
		   const  CCorrelationEvaluator::result_type& corr):
	GroundTruthProblem(1.0, 1.0, slice_size, N, left_side, corr)
{
}

void GroundTruthAccess::check_spacial_gradient(const double *test)
{
	check_vector_equal(get_spacial_gradient(), test);
}

void GroundTruthAccess::check_time_derivative(const double *test)
{
	check_vector_equal(get_time_derivative(), test);
}

GroundTruthFixture::GroundTruthFixture():
	left_side(psize),
	x(psize)
{
	float hinit[6] = {1.0, 1.0, 1.0, 0.5, 0.5, 0.0};
	float vinit[6] = {0.0, 1.0, 1.0, 0.5, 0.5, 1.0};
	float xinit[psize] = {
		0, 4, 0,  1, 1, 2,  3, 1, 1,
		0, 3, 0,  2, 2, 3,  3, 1, 1,
		0, 2, 0,  3, 3, 4,  4, 2, 1,
		0, 1, 0,  4, 4, 4,  7, 1, 1,
		0, 0, 0,  0, 0, 0,  0, 0, 0
	};


	corr.horizontal = C2DFImage(C2DBounds(2,3), hinit);
	corr.vertical = C2DFImage(C2DBounds(3,2), vinit);

	pgta.reset(new GroundTruthAccess(left_side, corr));
	copy(xinit, xinit + psize, x.begin());
}



