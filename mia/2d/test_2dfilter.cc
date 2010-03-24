/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/core/history.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/filtertest.hh>

NS_MIA_USE
using namespace std; 
using namespace boost;
namespace bfs=::boost::filesystem; 
using namespace boost::unit_test;

static void test_available_filters()
{
	const C2DFilterPluginHandler::Instance& handler = C2DFilterPluginHandler::instance(); 
	cvdebug() << "Found " << handler.size() << " plugins:" << handler.get_plugin_names() <<"\n"; 
	BOOST_CHECK_EQUAL(handler.size(), 22); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(),
		    "adaptmed admean aniso bandpass binarize close convert crop cst dilate downscale erode fft gauss gradnorm mask median mlv ngfnorm noise open sepconv ");
}


void add_2dimagefilter_tests(test_suite* test)
{	

	imagefiltertest2d_prepare_plugin_path(); 
	test->add( BOOST_TEST_CASE( &test_available_filters));
	add_2dfilter_plugin_test(test); 

}

