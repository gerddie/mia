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

#include <mia/2d/multicost.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fullcost.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/transformmock.hh>
#include <mia/2d/2dimageio.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs=::boost::filesystem; 

CSplineKernelTestPath spline_kernel_path_init; 

class C2DFullCostMock: public C2DFullCost {
public: 
	C2DFullCostMock(double weight, double cost, double gx, double gy); 
private:
	double do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const;
	double do_value(const C2DTransformation& t) const;
	double do_value() const; 
	void do_set_size(); 
	void do_reinit(); 
	double m_cost; 
	double m_gx; 
	double m_gy; 
}; 

C2DFullCostMock::C2DFullCostMock(double weight, double cost, double gx, double gy):
	C2DFullCost(weight),
	m_cost(cost), 
	m_gx(gx), 
	m_gy(gy)

{
}

double C2DFullCostMock::do_evaluate(const C2DTransformation&, CDoubleVector& gradient) const 
{
	for(auto g = gradient.begin(); g != gradient.end(); g += 2) {
		g[0] = m_gx; 
		g[1] = m_gy; 
	}
	return m_cost; 
}

double C2DFullCostMock::do_value(const C2DTransformation& /*t*/) const
{
	return m_cost; 
}

double C2DFullCostMock::do_value() const
{
	return m_cost; 
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
	CDoubleVector gradient(t.degrees_of_freedom()); 
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
	CDoubleVector gradient(t.degrees_of_freedom(), true); 
	costs.set_size(t.get_size()); 
	
	BOOST_CHECK_EQUAL(costs.evaluate(t,gradient), 0.5 * mcost1 + 0.2 * mcost2);

	BOOST_CHECK_EQUAL(gradient[0], 0.5 * mgx1 + 0.2 * mgx2);
	BOOST_CHECK_EQUAL(gradient[1], 0.5 * mgy1 + 0.2 * mgy2);
}

BOOST_AUTO_TEST_CASE( test_multicost2 ) 
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
	costs.push(c2); 
	costs.push(c1); 

	
	C2DTransformMock t(C2DBounds(2,1)); 
	CDoubleVector gradient(t.degrees_of_freedom()); 
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

class PrepareFullcostTests {
public: 
	static const PrepareFullcostTests& instance(); 
	
	const C2DFullCostPluginHandler::Instance& fullcost_handler()const; 
	const C2DTransformCreatorHandler::Instance& transform_handler()const; 
private:
	PrepareFullcostTests(); 
}; 


BOOST_AUTO_TEST_CASE( test_load_plugins ) 
{	
	const C2DFullCostPluginHandler::Instance& handler = PrepareFullcostTests::instance().fullcost_handler(); 
	BOOST_CHECK_EQUAL(handler.size(), 2u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "divcurl image ");
}

#if 0 
BOOST_AUTO_TEST_CASE ( test_ssd_cost_spline_rate_3 ) 
{
	auto cost_ssd = PrepareFullcostTests::instance().fullcost_handler().produce("image:cost=ssd"); 
	auto transform_factory = PrepareFullcostTests::instance().transform_handler().produce("spline:rate=3"); 

	C2DBounds size(16,16); 
	const float src_init[ 16 * 16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,20,20,20,10,10,10,10,10,20,10,10,10, 0, 
		0,10,10,20,30,30,30,30,30,30,30,30,20,10, 0, 
		0, 0,20,20,20,20,20,20,20,20,20,20,20,10, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,20,20,20,10,10,10,10,10,20,10,10,10, 0, 
		0,10,10,20,30,30,30,30,30,30,30,30,20,10, 0, 
		0, 0,20,20,20,20,20,20,20,20,20,20,20,10, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,20,20,20,10,10,10,10,10,20,10,10,10, 0, 
		0,10,10,20,30,30,30,30,30,30,30,30,20,10, 0, 
		0, 0,20,20,20,20,20,20,20,20,20,20,20,10, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		
	}; 

	const float ref_init[ 16 * 16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,30,30,40,20,20,30,30,40,40,20,30,10, 0, 
		0,10,20,10,20,20,20,30,40,50,40,40,30,10, 0, 
		0, 0,20,20,20,20,20,20,20,20,20,20,20,10, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,20,20,20,10,10,10,10,10,20,10,10,10, 0, 
		0,10,30,30,20,20,32,20,30,40,50,30,20,10, 0, 
		0, 0,30,30,30,30,30,30,20,20,50,20,20,10, 0, 
		0,10,30.10,30,30,30,30,30,20,20,20,10,10, 0, 
		0,10,30,10,30,30,40,43,20,20,30,30,10,10, 0, 
		0,10,10,20,30,30,20,20,20,20,20,30,20,10, 0, 
		0, 0,20,20,20,30,30,20,20,20,20,20,20,10, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		
	}; 

	P2DImage src(new C2DFImage(size, src_init)); 
	save_image2d("src.@", src); 

	P2DImage ref(new C2DFImage(size, ref_init)); 
	save_image2d("ref.@", ref); 

	cost_ssd->reinit(); 
	cost_ssd->set_size(size); 
	
	P2DTransformation t = transform_factory->create(size); 

	auto params = t->get_parameters(); 
	CDoubleVector gradient(params.size());  

	double cost = cost_ssd->evaluate(*t, gradient);
//	BOOST_CHECK_CLOSE(cost, 55*55 / 1024.0, 0.1); 
	
	for (size_t i = 0; i < params.size(); ++i) {
		params[i] -= 0.0001; 
		t->set_parameters(params); 
		double costm = cost_ssd->cost_value(*t);
		params[i] += 0.0002; 
		t->set_parameters(params); 
		double costp = cost_ssd->cost_value(*t);
		params[i] -= 0.0001;
		double cgrad = (costp - costm) / 0.0002; 

		cvinfo()  << gradient[i] << " vs " << cgrad << " " 
			  << gradient[i]/ cgrad<< "\n"; 

		if (fabs(cgrad) > 0.001) 
			BOOST_CHECK_CLOSE(gradient[i], cgrad, 0.1); 
		else 
			BOOST_CHECK_CLOSE(1.0 + gradient[i], 1.0, 0.1); 
	}

}


BOOST_AUTO_TEST_CASE ( test_ssd_cost_vf ) 
{
	auto cost_ssd = PrepareFullcostTests::instance().fullcost_handler().produce("image:cost=ssd"); 
	auto transform_factory = PrepareFullcostTests::instance().transform_handler().produce("vf"); 

	C2DBounds size(16,16); 
	const float src_init[ 16 * 16] = {
		0,10,20,30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,20,30,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,30,40,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,40,50,60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,50,60,70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,60,70,80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,70,80,90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,80,90,00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,70,80,90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,60,70,80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,50,60,70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,40,50,60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,30,40,50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,20,30,40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,10,20,30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0,10,20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		
	}; 

	const float ref_init[ 16 * 16] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,30,30,40,20,20,30,30,40,40,20,30,10, 0, 
		0,10,20,10,20,20,20,30,40,50,40,40,30,10, 0, 
		0, 0,20,20,20,20,20,20,20,20,20,20,20,10, 0, 
		0,10,10,10,10,10,10,20,20,20,20,20,10,10, 0, 
		0,10,20,20,20,10,10,10,10,10,20,10,10,10, 0, 
		0,10,30,30,20,20,32,20,30,40,50,30,20,10, 0, 
		0, 0,30,30,30,30,30,30,20,20,50,20,20,10, 0, 
		0,10,30.10,30,30,30,30,30,20,20,20,10,10, 0, 
		0,10,30,10,30,30,40,43,20,20,30,30,10,10, 0, 
		0,10,10,20,30,30,20,20,20,20,20,30,20,10, 0, 
		0, 0,20,20,20,30,30,20,20,20,20,20,20,10, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		
	}; 

	P2DImage src(new C2DFImage(size, src_init)); 
	save_image2d("src.@", src); 

	P2DImage ref(new C2DFImage(size, ref_init)); 
	save_image2d("ref.@", ref); 

	cost_ssd->reinit(); 
	cost_ssd->set_size(size); 
	
	P2DTransformation t = transform_factory->create(size); 

	auto params = t->get_parameters(); 
	CDoubleVector gradient(params.size());  

	double cost = cost_ssd->evaluate(*t, gradient);
//	BOOST_CHECK_CLOSE(cost, 55*55 / 1024.0, 0.1); 
	
	for (size_t i = 0; i < params.size(); ++i) {
		params[i] -= 0.0001; 
		t->set_parameters(params); 
		double costm = cost_ssd->cost_value(*t);
		params[i] += 0.0002; 
		t->set_parameters(params); 
		double costp = cost_ssd->cost_value(*t);
		params[i] -= 0.0001;
		double cgrad = (costp - costm) / 0.0002; 

		cvinfo()  << gradient[i] << " vs " << cgrad << " " 
			  << gradient[i]/ cgrad<< "\n"; 

		if (fabs(cgrad) > 0.01) 
			BOOST_CHECK_CLOSE(gradient[i], cgrad, 0.1); 
		else 
			BOOST_CHECK_CLOSE(1.0 + gradient[i], 1.0, 0.1); 
	}

}
#endif

const PrepareFullcostTests& PrepareFullcostTests::instance()
{
	const static PrepareFullcostTests  me; 
	return me; 
}

const C2DFullCostPluginHandler::Instance& PrepareFullcostTests::fullcost_handler() const
{
	return C2DFullCostPluginHandler::instance(); 
}

const C2DTransformCreatorHandler::Instance& PrepareFullcostTests::transform_handler()const
{
	return C2DTransformCreatorHandler::instance(); 
}

PrepareFullcostTests::PrepareFullcostTests()
{
	list< bfs::path> cost_kernel_plugpath;
	cost_kernel_plugpath.push_back(bfs::path("cost"));
	C2DImageCostPluginHandler::set_search_path(cost_kernel_plugpath);
	
	list< bfs::path> cost_plugpath;
	cost_plugpath.push_back(bfs::path("fullcost"));
	C2DFullCostPluginHandler::set_search_path(cost_plugpath);

	list< bfs::path> transform_searchpath;
	transform_searchpath.push_back(bfs::path("transform"));
	C2DTransformCreatorHandler::set_search_path(transform_searchpath);

}

