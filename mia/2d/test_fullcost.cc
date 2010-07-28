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
	C2DFullCostMock(double weight); 
private:
	double do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;
	void do_set_size(); 
}; 

C2DFullCostMock::C2DFullCostMock(double weight):
	C2DFullCost(weight)
{
}

double C2DFullCostMock::do_evaluate(const C2DTransformation&, gsl::DoubleVector& gradient) const 
{
	fill(gradient.begin(), gradient.end(), 4.0); 
	return 10.0; 
}

void C2DFullCostMock::do_set_size()
{
}

BOOST_AUTO_TEST_CASE( test_fullcost ) 
{
	C2DFullCostMock c(0.5); 
	C2DTransformMock t(C2DBounds(2,1)); 
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	c.set_size(t.get_size()); 
	
	BOOST_CHECK_EQUAL(c.evaluate(t,gradient), 5.0);

	BOOST_CHECK_EQUAL(gradient[0], 2.0);
	BOOST_CHECK_EQUAL(gradient[1], 2.0);
}

BOOST_AUTO_TEST_CASE( test_multicost ) 
{
	P2DFullCost c1(new C2DFullCostMock(0.5)); 
	P2DFullCost c2(new C2DFullCostMock(0.2)); 
	
	C2DFullCostList costs; 
	costs.push(c1); 
	costs.push(c2); 
	
	C2DTransformMock t(C2DBounds(2,1)); 
	gsl::DoubleVector gradient(t.degrees_of_freedom()); 
	costs.set_size(t.get_size()); 
	
	BOOST_CHECK_EQUAL(costs.evaluate(t,gradient), 7.0);

	BOOST_CHECK_EQUAL(gradient[0], 2.8);
	BOOST_CHECK_EQUAL(gradient[1], 2.8);
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
	

