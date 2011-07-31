/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <stdexcept>
#include <climits>
#include <atomic>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/datapool.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
using namespace tbb;


NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
using namespace boost;


BOOST_AUTO_TEST_CASE( test_pool_inout )
{

	CDatapool::instance().add("param1", 10);
	CDatapool::instance().add("param2", string("fun"));

	any p1 = CDatapool::instance().get("param1");

	BOOST_CHECK(p1.type() == typeid(int));
	BOOST_CHECK_EQUAL(any_cast<int>(p1), 10);

	any p2 = CDatapool::instance().get("param2");
	BOOST_CHECK(p2.type() == typeid(string));
	BOOST_CHECK_EQUAL(any_cast<string>(p2), string("fun"));

	BOOST_CHECK(!CDatapool::instance().has_unused_data());
}

BOOST_AUTO_TEST_CASE( test_pool_out_noexists )
{
	BOOST_CHECK_THROW(CDatapool::instance().get("param3"), invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_pool_get_and_remove )
{
	CDatapool::instance().add("param1", 10);
	any p1 = CDatapool::instance().get_and_remove("param1");
	BOOST_CHECK(!CDatapool::instance().has_key("param1"));
}

BOOST_AUTO_TEST_CASE( test_pool_has_unused )
{
	CDatapool::instance().add("param1", 10);
	BOOST_CHECK(CDatapool::instance().has_unused_data());
}

BOOST_AUTO_TEST_CASE( test_pool_has_key )
{
	CDatapool::instance().add("param1", 10);
	BOOST_CHECK(CDatapool::instance().has_key("param1"));
	BOOST_CHECK(!CDatapool::instance().has_key("unknown"));
}

struct PoolAccessTest {
	std::atomic<int> *n_errors; 
	PoolAccessTest(std::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

PoolAccessTest::PoolAccessTest(std::atomic<int> *_nerr):
	n_errors(_nerr)
{ 
}

void PoolAccessTest::operator() ( const blocked_range<int>& range ) const
{
	try {	
		int sum = 0; 
		for( int i=range.begin(); i!=range.end(); ++i ) {
			stringstream name; 
			name << "parallel" << i; 
			CDatapool::instance().add(name.str(), i);
			any p1 = CDatapool::instance().get(name.str());
			int k = any_cast<int>(p1); 
			if (k != i) 
				++(*n_errors); 
		}
	}
	catch (std::runtime_error& x) {
		cout << x.what() << "\n"; 
	}
}

BOOST_AUTO_TEST_CASE( test_pool_parallel_access )
{
	task_scheduler_init init;
	std::atomic<int> n_errors(0); 
	PoolAccessTest ptest(&n_errors); 
	
	blocked_range<int> range( 0, 1000, 1 ); 
	parallel_for(range, ptest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 
}

struct PoolWriteLaterReadTest {
	std::atomic<int> *n_errors; 
	PoolWriteLaterReadTest(std::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

PoolWriteLaterReadTest::PoolWriteLaterReadTest(std::atomic<int> *_nerr):
	n_errors(_nerr)
{ 
}

void PoolWriteLaterReadTest::operator() ( const blocked_range<int>& range ) const
{
	try {	
		for( int i=range.begin(); i!=range.end(); ++i ) {
			stringstream name; 
			name << "parallel" << i; 
			CDatapool::instance().add(name.str(), i);
		}
		for( int i=range.begin(); i!=range.end(); ++i ) {
			stringstream name; 
			name << "parallel" << i; 
			any p1 = CDatapool::instance().get(name.str());
			int k = any_cast<int>(p1); 
			if (k != i) 
				++(*n_errors); 
		}
	}
	catch (std::runtime_error& x) {
		cout << x.what() << "\n"; 
	}
}

BOOST_AUTO_TEST_CASE( test_pool_parallel_access_2 )
{
	task_scheduler_init init;
	std::atomic<int> n_errors(0); 
	PoolWriteLaterReadTest ptest(&n_errors); 
	
	blocked_range<int> range( 0, 1000, 5 ); 
	parallel_for(range, ptest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 
}
