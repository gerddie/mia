/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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


#include <mia/2d/multicost.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/transformmock.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem; 


class C2DFullCostMock: public C2DFullCost {
public: 
	C2DFullCostMock(double weight, double cost, double gx, double gy); 
private:
	double do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;
	double do_value(const C2DTransformation& t) const;
	double do_value() const; 
	void do_set_size(); 
	void do_reinit(); 
	double _M_cost; 
	double _M_gx; 
	double _M_gy; 
}; 

C2DFullCostMock::C2DFullCostMock(double weight, double cost, double gx, double gy):
	C2DFullCost(weight),
	_M_cost(cost), 
	_M_gx(gx), 
	_M_gy(gy)

{
}

double C2DFullCostMock::do_evaluate(const C2DTransformation&, gsl::DoubleVector& gradient) const 
{
	for(auto g = gradient.begin(); g != gradient.end(); g += 2) {
		g[0] = _M_gx; 
		g[1] = _M_gy; 
	}
	return _M_cost; 
}

double C2DFullCostMock::do_value(const C2DTransformation& t) const
{
	return _M_cost; 
}

double C2DFullCostMock::do_value() const
{
	return _M_cost; 
}
void C2DFullCostMock::do_reinit()
{
}

void C2DFullCostMock::do_set_size()
{
}

BOOST_AUTO_TEST_CASE( test_fullcost ) 
{
	const double mcost = 10.0; 
	const double mgx   = 2.0; 
	const double mgy   = 3.0; 

	C2DFullCostMock c(0.5, mcost, mgx, mgy); 
	C2DTransformMock t(C2DBounds(2,1)); 
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	c.set_size(t.get_size()); 
	
	BOOST_CHECK_EQUAL(c.evaluate(t,gradient), 0.5 * mcost);

	BOOST_CHECK_EQUAL(gradient[0], 0.5 * mgx);
	BOOST_CHECK_EQUAL(gradient[1], 0.5 * mgy);
}

BOOST_AUTO_TEST_CASE( test_multicost ) 
{
	const double mcost1 = 10.0; 
	const double mgx1   = 2.0; 
	const double mgy1   = 3.0; 

	const double mcost2 = 2.0; 
	const double mgx2   = 4.0; 
	const double mgy2   = 9.0; 

	P2DFullCost c1(new C2DFullCostMock(0.5, mcost1, mgx1, mgy1)); 
	P2DFullCost c2(new C2DFullCostMock(0.2, mcost2, mgx2, mgy2)); 
	
	C2DFullCostList costs; 
	costs.push(c1); 
	costs.push(c2); 
	
	C2DTransformMock t(C2DBounds(2,1)); 
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	costs.set_size(t.get_size()); 
	
	BOOST_CHECK_EQUAL(costs.evaluate(t,gradient), 0.5 * mcost1 + 0.2 * mcost2);

	BOOST_CHECK_EQUAL(gradient[0], 0.5 * mgx1 + 0.2 * mgx2);
	BOOST_CHECK_EQUAL(gradient[1], 0.5 * mgy1 + 0.2 * mgy2);
}

BOOST_AUTO_TEST_CASE( test_multicost_property ) 
{
	const double mcost1 = 10.0; 
	const double mgx1   = 2.0; 
	const double mgy1   = 3.0; 

	const double mcost2 = 2.0; 
	const double mgx2   = 4.0; 
	const double mgy2   = 9.0; 

	P2DFullCost c1(new C2DFullCostMock(0.5, mcost1, mgx1, mgy1)); 
	P2DFullCost c2(new C2DFullCostMock(0.2, mcost2, mgx2, mgy2)); 

	const char *test_prop = "test_prop"; 
	
	C2DFullCostList costs; 
	BOOST_CHECK(!costs.has(test_prop)); 
	
	costs.push(c1); 
	costs.push(c2); 
	
	BOOST_CHECK(!costs.has(test_prop)); 
	c1->add(test_prop); 
	BOOST_CHECK(!costs.has(test_prop)); 
	c2->add(test_prop); 
	BOOST_CHECK(costs.has(test_prop)); 
}


BOOST_AUTO_TEST_CASE( test_load_plugins ) 
{
	list< bfs::path> cost_kernel_plugpath;
	cost_kernel_plugpath.push_back(bfs::path("cost"));
	C2DImageCostPluginHandler::set_search_path(cost_kernel_plugpath);
	
	list< bfs::path> cost_plugpath;
	cost_plugpath.push_back(bfs::path("fullcost"));
	C2DFullCostPluginHandler::set_search_path(cost_plugpath);
	
	const C2DFullCostPluginHandler::Instance& handler = C2DFullCostPluginHandler::instance(); 
	BOOST_CHECK_EQUAL(handler.size(), 2); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "divcurl image ");
}
	

