/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>

#include <mia/core/cmdlineparser.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/datapool.hh>

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>
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

BOOST_AUTO_TEST_CASE( test_pool_clear )
{
	CDatapool::instance().add("param1", 10);
	BOOST_CHECK(CDatapool::instance().has_key("param1"));
	BOOST_CHECK(!CDatapool::instance().has_key("unknown"));
	
	CDatapool::instance().clear(); 
	BOOST_CHECK(!CDatapool::instance().has_key("param1"));
}

struct PoolAccessTest {
	tbb::atomic<int> *n_errors; 
	PoolAccessTest(tbb::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

PoolAccessTest::PoolAccessTest(tbb::atomic<int> *_nerr):
	n_errors(_nerr)
{ 
}

void PoolAccessTest::operator() ( const blocked_range<int>& range ) const
{
	try {	
		for(auto i=range.begin(); i!=range.end(); ++i ) {
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
	tbb::atomic<int> n_errors;
	n_errors = 0; 
	PoolAccessTest ptest(&n_errors); 
	
	blocked_range<int> range( 0, 1000, 1 ); 
	parallel_for(range, ptest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 
}

struct PoolWriteLaterReadTest {
	tbb::atomic<int> *n_errors; 
	PoolWriteLaterReadTest(tbb::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

PoolWriteLaterReadTest::PoolWriteLaterReadTest(tbb::atomic<int> *_nerr):
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
	tbb::atomic<int> n_errors; 
	n_errors = 0; 
	PoolWriteLaterReadTest ptest(&n_errors); 
	
	blocked_range<int> range( 0, 1000, 5 ); 
	parallel_for(range, ptest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 
}
