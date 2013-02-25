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

#include <mia/internal/autotest.hh>
#include <mia/core/productcache.hh>

#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/atomic.h>

using namespace tbb;


NS_MIA_USE
using namespace std; 

struct Integer {
	
	Integer(int v):value(v){}; 

	operator int() {
		return value; 
	}

	int value; 
};

bool operator == (const Integer& lhs, const Integer& rhs) 
{
	return lhs.value == rhs.value; 
}

ostream& operator << (ostream& os, const Integer& i) 
{
	os << i.value; 
	return os; 
}

typedef shared_ptr<Integer> CProductMock;  


typedef TProductCache<CProductMock> MyProductCache; 

MyProductCache the_cache("Integer"); 

BOOST_AUTO_TEST_CASE(test_basic_cache_enabled) 
{
	the_cache.clear(); 
	the_cache.enable_write(true); 

	BOOST_CHECK(!the_cache.get("1")); 

	the_cache.add("1", CProductMock(new Integer(1))); 

	auto one = the_cache.get("1"); 
	
	BOOST_CHECK(one); 
	BOOST_CHECK_EQUAL(*one, 1); 

	the_cache.enable_write(false); 
	BOOST_CHECK(the_cache.get("1"));

	the_cache.clear(); 
	BOOST_CHECK(!the_cache.get("1")); 

}

BOOST_AUTO_TEST_CASE(test_basic_cache_disabled) 
{
	the_cache.clear(); 
	the_cache.enable_write(false); 

	BOOST_CHECK(!the_cache.get("1")); 

	the_cache.add("1", CProductMock(new Integer(1))); 

	auto one = the_cache.get("1"); 
	
	BOOST_CHECK(!one); 
}


BOOST_AUTO_TEST_CASE(test_basic_cache_enabled_global_clear) 
{
	the_cache.clear(); 
	the_cache.enable_write(true); 

	BOOST_CHECK(!the_cache.get("1")); 

	the_cache.add("1", CProductMock(new Integer(1))); 

	auto one = the_cache.get("1"); 
	
	BOOST_CHECK(one); 
	BOOST_CHECK_EQUAL(*one, 1); 

	CProductCacheHandler::instance().clear_all(); 
	BOOST_CHECK(!the_cache.get("1")); 

}


struct CacheAccessTest {
	tbb::atomic<int> *n_errors; 
	CacheAccessTest(tbb::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

CacheAccessTest::CacheAccessTest(tbb::atomic<int> *_nerr):
	n_errors(_nerr)
{ 
}

void CacheAccessTest::operator() ( const blocked_range<int>& range ) const
{
	try {	
		for(auto i=range.begin(); i!=range.end(); ++i ) {
			stringstream s; 
			s << i;
			if (the_cache.get(s.str()))
				++(*n_errors);
		}
	}
	catch (std::runtime_error& x) {
		cout << x.what() << "\n"; 
	}
}

struct CacheWriteTest {
	void operator()( const blocked_range<int>& range ) const; 
}; 

void CacheWriteTest::operator() ( const blocked_range<int>& range ) const
{
	for(auto i=range.begin(); i!=range.end(); ++i ) {
		stringstream s; 
		s << (i / 2);
		the_cache.add(s.str(), CProductMock(new Integer(i/2)));
	}
}


struct CacheReadTest {
	tbb::atomic<int> *n_errors; 
	CacheReadTest(tbb::atomic<int> *_nerr); 
	void operator()( const blocked_range<int>& range ) const; 
}; 

CacheReadTest::CacheReadTest(tbb::atomic<int> *_nerr):
	n_errors(_nerr)
{ 
}

void CacheReadTest::operator() ( const blocked_range<int>& range ) const
{
	try {	
		for(auto i=range.begin(); i!=range.end(); ++i ) {
			stringstream s; 
			s << (i / 2);
			auto value = the_cache.get(s.str()); 
			
			if (!value) {
				++(*n_errors); 
			}else {
				if (*value != i/2)
					++(*n_errors);
			}
		}
	}
	catch (std::runtime_error& x) {
		cout << x.what() << "\n"; 
	}
}

BOOST_AUTO_TEST_CASE( test_cache_parallel_access )
{
	the_cache.clear(); 
	the_cache.enable_write(true); 


	task_scheduler_init init;
	tbb::atomic<int> n_errors; 
	n_errors = 0; 

	blocked_range<int> range( 0, 1000, 1 ); 
	CacheAccessTest ptest(&n_errors); 
	


	parallel_for(range, ptest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 

	CacheWriteTest wtest; 
	parallel_for(range, wtest); 

	n_errors = 0; 
	CacheReadTest rtest(&n_errors);
	parallel_for(range, rtest); 
	BOOST_CHECK_EQUAL(n_errors, 0); 
	
	
}
