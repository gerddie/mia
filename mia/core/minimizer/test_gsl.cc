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

#include <mia/internal/autotest.hh>
#include <mia/core/minimizer/gsl.hh>
using namespace gsl;
using namespace mia;

class TestCFDFProblem : public CMinimizer::Problem
{
public:
       TestCFDFProblem();
private:
       virtual double  do_f(const CDoubleVector& x);
       virtual void    do_df(const CDoubleVector&  x, CDoubleVector&  g);
       virtual double  do_fdf(const CDoubleVector&  x, CDoubleVector&  g);
       virtual size_t do_size() const;
       std::vector<double> m_p;
       size_t m_size;
};

BOOST_AUTO_TEST_CASE( test_cfdf_multmin )
{
       CGSLFDFMinimizer minimizer(gsl_multimin_fdfminimizer_conjugate_fr,
                                  0.01, 0.01, 100, 0.01);
       CMinimizer::PProblem problem(new TestCFDFProblem);
       problem->add(property_gradient);
       minimizer.set_problem(problem);
       CDoubleVector x(2);
       x[0] = 5.0;
       x[1] = 7.0;
       BOOST_REQUIRE(minimizer.run(x) ==  CMinimizer::success);
       BOOST_CHECK_CLOSE(x[0], 1.0, 0.1);
       BOOST_CHECK_CLOSE(x[1], 2.0, 0.1);
}

TestCFDFProblem::TestCFDFProblem():
       CGSLFDFMinimizer::Problem(),
       m_p(5),
       m_size(2)
{
       double p_init[5] = {1.0, 2.0, 10.0, 20.0, 30.0 };
       copy(p_init, p_init + 5, m_p.begin());
}

size_t TestCFDFProblem::do_size()const
{
       return m_size;
}

double  TestCFDFProblem::do_f(const CDoubleVector&  v)
{
       const double x = v[0];
       const double y = v[1];
       return m_p[2] * (x - m_p[0]) * (x - m_p[0]) +
              m_p[3] * (y - m_p[1]) * (y - m_p[1]) + m_p[4];
}

void    TestCFDFProblem::do_df(const CDoubleVector&  v, CDoubleVector&  g)
{
       const double x = v[0];
       const double y = v[1];
       g[0] = 2.0 * m_p[2] * (x - m_p[0]);
       g[1] = 2.0 * m_p[3] * (y - m_p[1]);
}

double  TestCFDFProblem::do_fdf(const CDoubleVector&  x, CDoubleVector&  g)
{
       do_df(x, g);
       return do_f(x);
}


class TestCFProblem : public CMinimizer::Problem
{
public:
       TestCFProblem();
private:
       virtual double  do_f(const CDoubleVector& x);
       virtual void    do_df(const CDoubleVector& x, CDoubleVector&  g);
       virtual double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
       virtual size_t do_size() const;
       std::vector<double> m_p;
       size_t m_size;
};


BOOST_AUTO_TEST_CASE(test_cf_multmin )
{
       CGSLFMinimizer minimizer(gsl_multimin_fminimizer_nmsimplex,
                                0.001, 100, 1);
       CMinimizer::PProblem problem(new TestCFProblem);
       minimizer.set_problem(problem);
       CDoubleVector x(2);
       x[0] = 5.0;
       x[1] = 7.0;
       BOOST_REQUIRE(minimizer.run(x) == CMinimizer::success);
       BOOST_CHECK_CLOSE(x[0], 1.0, 1);
       BOOST_CHECK_CLOSE(x[1], 2.0, 1);
}


TestCFProblem::TestCFProblem():
       m_p(5),
       m_size(2)
{
       double p_init[5] = {1.0, 2.0, 10.0, 20.0, 30.0 };
       copy(p_init, p_init + 5, m_p.begin());
}

double  TestCFProblem::do_f(const CDoubleVector&  v)
{
       const double x = v[0];
       const double y = v[1];
       return m_p[2] * (x - m_p[0]) * (x - m_p[0]) +
              m_p[3] * (y - m_p[1]) * (y - m_p[1]) + m_p[4];
}

void  TestCFProblem::do_df(const CDoubleVector&   /*x*/, CDoubleVector&   /*g*/)
{
       BOOST_FAIL("Gradient free problem was called by gradient requiring optimizer");
}

double  TestCFProblem::do_fdf(const CDoubleVector&   /*x*/, CDoubleVector&   /*g*/)
{
       BOOST_FAIL("Gradient free problem was called by gradient requiring optimizer");
       return 0.0;
}

size_t TestCFProblem::do_size()const
{
       return m_size;
}
