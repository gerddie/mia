/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2011
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

#define VSTREAM_DOMAIN "THREADEDMSG"

#include <mia/internal/autotest.hh>
#include <iomanip>
#include <mia/core/threadedmsg.hh>
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
using namespace tbb;


NS_MIA_USE;
using namespace std;


struct ThreadFixture {
	ThreadFixture(); 

	void operator () ( const blocked_range<int>& range ) const; 

	void testit() const; 

}; 

stringstream master_stream; 

ThreadFixture::ThreadFixture()
{
	thread_streamredir::set_master_stream(master_stream); 
}

void ThreadFixture::operator () ( const blocked_range<int>& range ) const
{
	CThreadMsgStream thread_stream;

	for( int i=range.begin(); i!=range.end(); ++i ) {
		cvmsg() << "This " << "is " << "part " << "of" << " a" << " message ..." << setw(3) << i; 
		cverb << " and" << " this" << " is" <<" the" <<" other" << " part\n"; 
	}
}



BOOST_AUTO_TEST_CASE( test_threaded_msg )
{
	task_scheduler_init init;

	auto old_level =  cverb.get_level();
	cverb.set_verbosity(vstream::ml_message); 
	ThreadFixture fix; 
	parallel_for(blocked_range<int>( 0, 100), fix);
	
	cverb.set_verbosity(old_level); 
	
	cvdebug()  << master_stream.str(); 
	
	const char *s = master_stream.str().c_str(); 

	BOOST_REQUIRE(s);

	while (*s)  {
		int counter = 0; 
		while (*s && *s != '\n') {
			++counter; 
			++s; 
		}
		BOOST_CHECK_EQUAL(counter, 84); 
		++s; 
	}; 
}

