/* -*- mia-c++  -*-
 *
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

#include <cstdlib>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>


#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/2d/2dvfio.hh>
#include <mia/core/filter.hh>

NS_MIA_USE
using namespace boost; 
using namespace std; 
using namespace boost::unit_test;
namespace bfs=::boost::filesystem; 

const char revision[] = "FakeRevision"; 



static void check_save_load(const C2DIOVectorfield& vf, const C2DVFIOPluginHandler::Instance::Interface& vfio)
{
	const string format = vfio.get_name();
	const string tmp_name = string("vf2dtest") + string(".")+ format;
	
	BOOST_REQUIRE(vfio.save(tmp_name.c_str(), vf));
	
	cvdebug() << format << "saved" << std::endl;
	
	std::shared_ptr<C2DIOVectorfield > reread(vfio.load(tmp_name.c_str()));
	
	BOOST_REQUIRE(reread.get()); 
        cvdebug() << vf.size() << " vs. " << reread->size() << "\n"; 
	BOOST_REQUIRE(vf.size() == reread->size()); 

	BOOST_REQUIRE(vf.get_size() == reread->get_size()); 
	
	for (C2DIOVectorfield::const_iterator iv = vf.begin(), ir = reread->begin(); 
	     iv != vf.end();  ++iv, ++ir) {
		cvdebug() << *iv << " vs. " << *ir << "\n"; 
		BOOST_CHECK(*iv == *ir); 
	}
	
	unlink(tmp_name.c_str());
	cvdebug() << tmp_name << " unlinked\n"; 
}

static void test_2dvfio_plugin_handler() 
{
	const C2DVFIOPluginHandler::Instance& vfioh = C2DVFIOPluginHandler::instance(); 
	BOOST_CHECK(vfioh.size() == 1); 
	BOOST_CHECK(vfioh.get_plugin_names() == "exr "); 
}

static void test_2dvfio()
{
	
	const C2DVFIOPluginHandler::Instance& vfioh = C2DVFIOPluginHandler::instance(); 
	C2DIOVectorfield vf(C2DBounds(5,6)); 
	
	for (C2DIOVectorfield::iterator i = vf.begin(); i != vf.end(); ++i)
#ifdef WIN32
		*i = C2DFVector((float)rand()/RAND_MAX, (float)rand()/RAND_MAX); 
#else
		*i = C2DFVector(drand48(), drand48());		
#endif
	
	
	for (C2DVFIOPluginHandler::Instance::const_iterator i = vfioh.begin(); 
	     vfioh.end() != i; ++i)
		check_save_load(vf, *i->second); 
}

static void handler_setup()
{
	std::list< bfs::path> searchpath; 
	searchpath.push_back( bfs::path("2d") / bfs::path("io")); 
	searchpath.push_back( bfs::path("io")); 
	C2DVFIOPluginHandler::set_search_path(searchpath); 
}

void add_2dvfio_tests(test_suite* test)
{	
	handler_setup(); 

	//test->add( BOOST_TEST_CASE( &test_2dvfio_plugin_handler));
	test->add( BOOST_TEST_CASE( &test_2dvfio));
}

