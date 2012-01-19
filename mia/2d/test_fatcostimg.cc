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

#ifndef BOOST_TEST_DYN_LINK
#define BOOST_TEST_DYN_LINK
#endif

#include <stdexcept>

#include <mia/core/transformation.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdlineparser.hh>

#include <mia/2d/gridtransformation.hh>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/transform.hh>
#include <mia/core/fatcost.cxx>

using namespace mia; 
using namespace std; 
namespace bfs=::boost::filesystem; 
using namespace boost::unit_test;
using namespace boost; 

class CTest2DFatCost : public C2DImageFatCost {
public: 
	CTest2DFatCost(const P2DImage& src, const P2DImage& ref, float w); 
private: 
	virtual double do_value() const; 
	virtual double do_evaluate_force(C2DFVectorfield& force) const; 
	virtual P2DImageFatCost cloned(P2DImage src, P2DImage ref, float weight)const; 
}; 

typedef std::shared_ptr<CTest2DFatCost > PTest2DFatCost; 


CTest2DFatCost::CTest2DFatCost(const P2DImage& src, const P2DImage& ref, float w):
	C2DImageFatCost(src, ref, w)
{
}

double CTest2DFatCost::do_value() const
{
	return 1.0; 
}

double CTest2DFatCost::do_evaluate_force(C2DFVectorfield& force) const
{
	// no nothing 
	return value(); 
}

P2DImageFatCost CTest2DFatCost::cloned(P2DImage src, P2DImage ref, float weight)const 
{
	return P2DImageFatCost(new CTest2DFatCost(src, ref, weight)); 
}

static void test_2dfatimgcost()
{
	C2DBounds size(8,16);
	vector<float> init_test(8 * 16, 1.0); 
	vector<float> init_ref(8 * 16, 2.0); 
	
	init_test[8*8 + 4] =	1.5f; 

	C2DFImage *ptest_image = new C2DFImage(size, &init_test[0]); 

	P2DImage test_image(ptest_image);
	P2DImage ref_image(new C2DFImage(size, &init_ref[0]));



	BOOST_CHECK_EQUAL((*ptest_image)(4,8), 1.5f);

	CTest2DFatCost cost(test_image, ref_image, 0.5); 
	
	BOOST_CHECK_CLOSE(cost.value(), 0.5, 0.001);  
	BOOST_CHECK(cost.get_size() == size); 
	
	P2DImageFatCost cost_down = cost.get_downscaled(C2DBounds(2,2)); 
	
	BOOST_CHECK(cost_down->get_size() == C2DBounds(4,8)); 

	C2DGridTransformation trans( size); 
	fill(trans.field_begin(), trans.field_end(), C2DFVector(0,0)); 
	
	trans.field()(3,10) = C2DFVector(-1,2); 

	P2DInterpolatorFactory ipf = P2DInterpolatorFactory(create_2dinterpolation_factory(ip_nn)); 
	P2DImage def_image = transform2d(*test_image, *ipf, trans);

        try {
                C2DFImage& pref_image = dynamic_cast<C2DFImage&>(*def_image); 
                BOOST_CHECK_EQUAL( pref_image(3,10), 1.5f); 
	}
        catch (...){
                BOOST_FAIL("cast failed"); 
	}	
        cvmsg() << "hello\\n"; 
}



static void fatcost2d_prepare_plugin_path()
{
	list< bfs::path> fatcostsearchpath; 
	fatcostsearchpath.push_back( bfs::path("2d") / bfs::path("cost")); 
	fatcostsearchpath.push_back( bfs::path("cost")); 
	C2DFatImageCostPluginHandler::set_search_path(fatcostsearchpath);	
	C2DImageCostPluginHandler::set_search_path(fatcostsearchpath);
}



static void test_2dfatcost(const C2DFatImageCostPluginHandler::value_type& i) 
{
	BOOST_CHECK_MESSAGE(i.second->test(false), i.second->get_long_name()); 
}

void add_2dfatimgcost_tests(boost::unit_test::test_suite* suite)
{
	fatcost2d_prepare_plugin_path(); 
	suite->add( BOOST_PARAM_TEST_CASE(&test_2dfatcost, 
					  C2DFatImageCostPluginHandler::instance().begin(), 
					  C2DFatImageCostPluginHandler::instance().end()
					  ));

	suite->add( BOOST_TEST_CASE( &test_2dfatimgcost)); 
}
