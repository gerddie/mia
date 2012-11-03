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
#include <iomanip> 
#include <mia/internal/autotest.hh>
#include <mia/core/boundary_conditions.hh>

using namespace mia; 
using std::vector; 

CSplineBoundaryConditionTestPath  bc_test_path; 

BOOST_AUTO_TEST_CASE( test_BoundaryConditionsPlugins ) 
{
	const auto& handler = CSplineBoundaryConditionPluginHandler::instance();

	BOOST_CHECK_EQUAL(handler.size(), 3u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "mirror repeat zero "); 

}

BOOST_AUTO_TEST_CASE( test_Produce ) 
{
	auto bc = produce_spline_boundary_condition("repeat", 10); 
	BOOST_CHECK_EQUAL(bc->get_init_string(), "repeat:w=10");
}


class CBoundaryConditionMock : public CSplineBoundaryCondition {
public: 
	virtual CSplineBoundaryCondition *clone() const __attribute__((warn_unused_result)); 

	virtual void do_apply(CSplineKernel::VIndex& index, CSplineKernel::VWeight& weights) const;
	virtual void test_supported(int npoles) const;
	virtual double initial_coeff(const std::vector<double>& coeff, double pole) const;
	virtual double initial_anti_coeff(const std::vector<double>& coeff, double pole)const;

}; 

BOOST_AUTO_TEST_CASE( test_boundary_conditions_filter_line_one_coeff ) 
{
	CBoundaryConditionMock bc; 

	vector<double> poles(1); 
	vector<double> coeff(1); 
	coeff[0] = 1.0; 
	
	bc.filter_line(coeff, poles);
	BOOST_CHECK_EQUAL(coeff[0], 1.0); 
	
}

CSplineBoundaryCondition *CBoundaryConditionMock::clone() const 
{
	return new CBoundaryConditionMock(); 
}

void CBoundaryConditionMock::do_apply(CSplineKernel::VIndex& /*index*/, CSplineKernel::VWeight& /*weights*/) const
{
}

void CBoundaryConditionMock::test_supported(int /*npoles*/) const
{
}

double CBoundaryConditionMock::initial_coeff(const std::vector<double>& /*coeff*/, double /*pole*/) const
{
	return 0.0; 
}

double CBoundaryConditionMock::initial_anti_coeff(const std::vector<double>& /*coeff*/, double /*pole*/)const
{
	return 0.0; 
}


