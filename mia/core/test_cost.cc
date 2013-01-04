/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define BOOST_TEST_DYN_LINK

#include <climits>
#include <stdexcept>

#include <mia/core/cost.hh>
#include <mia/core/msgstream.hh>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/test_tools.hpp>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;


class TestCost: public TCost<double, double> {
	double do_value(const double& a, const double& b) const;
	double do_evaluate_force(const double& a, const double& b, float scale, double& force) const;
};



double TestCost::do_value(const double& a, const double& b) const
{
	double delta = a - b;
	return 0.5 * delta * delta;
}


double TestCost::do_evaluate_force(const double& a, const double& b, float scale, double& force) const
{
	double delta = a - b;
	force  = scale * a * delta;
	return do_value(a, b); 
}



BOOST_FIXTURE_TEST_CASE( test_cost_basic, TestCost )
{
	double a = 2.0;
	double b = 4.0;

	set_reference(b); 
	BOOST_CHECK_CLOSE(value(a), 2.0, 0.1);

	double force = 1.0;
	BOOST_CHECK_CLOSE(evaluate_force(a, 3.0, force), 2.0, 0.1); 

	BOOST_CHECK_CLOSE(force, -12.0, 0.1);
}

#include <mia/core/cost.cxx>
