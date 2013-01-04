/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

}; 

struct ThreadFixture2 {
	ThreadFixture2(); 

	void operator () ( const blocked_range<int>& range ) const; 
}; 


stringstream master_stream; 

ThreadFixture::ThreadFixture()
{
	CThreadMsgStream::set_master_stream(master_stream); 
}

void ThreadFixture::operator () ( const blocked_range<int>& range ) const
{
	CThreadMsgStream thread_stream;

	for( int i=range.begin(); i!=range.end(); ++i ) {
		cvmsg() << "This " << "is " << "part " << "of" << " a" << " message ..." << setw(3) << i; 
		cverb << " and" << " this" << " is" <<" the" <<" other" << " part\n"; 
	}
}

ThreadFixture2::ThreadFixture2()
{
	CThreadMsgStream::set_master_stream(master_stream); 
}

void ThreadFixture2::operator () ( const blocked_range<int>& range ) const
{
	CThreadMsgStream thread_stream;

	for( int i=range.begin(); i!=range.end(); ++i ) {
		cvmsg() << "This "<<"message "<<"has "<<"no "<< "newline"; 
		thread_stream.flush(); 
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
	
	string help = master_stream.str(); 
	const char *s = help.c_str(); 

	BOOST_REQUIRE(s);

	int lines = 0; 
	BOOST_REQUIRE(s); 
	while (*s)  {
		int counter = 0; 
		while (*s && *s != '\n') {
			++counter; 
			++s; 
		}
		++lines; 
		BOOST_CHECK_EQUAL(counter, 84); 
		++s; 
	}; 
	BOOST_CHECK_EQUAL(lines, 100); 
	master_stream.str(""); 
}


BOOST_AUTO_TEST_CASE( test_threaded_msg_sync )
{
	task_scheduler_init init;

	auto old_level =  cverb.get_level();
	cverb.set_verbosity(vstream::ml_message); 
	ThreadFixture2 fix; 
	parallel_for(blocked_range<int>( 0, 100), fix);
	
	cverb.set_verbosity(old_level); 
	
	cvdebug()  << master_stream.str(); 

	string help = master_stream.str(); 
	const char *s = help.c_str(); 

	BOOST_REQUIRE(s);

	int lines = 0; 
	while (*s)  {
		int counter = 0; 
		while (*s && *s != '\n') {
			++counter; 
			++s; 
		}
		BOOST_CHECK_EQUAL(counter, 52); 
		++lines; 
		++s; 
	}; 
	BOOST_CHECK_EQUAL(lines, 100); 
	master_stream.str(""); 
}


/* normally a compile time error tells us that we can't set a CThreadMsgStream
   as thread master output stream. 
   However, when this stream comes disguised as a std::ostream, no compile time error 
   will be generated. This test checks if the user at least gets to see the logic-error 
*/
BOOST_AUTO_TEST_CASE( test_set_master_CThreadMsgStream_throws  )
{
	CThreadMsgStream s; 
	ostream& h = s; 
	BOOST_CHECK_THROW(CThreadMsgStream::set_master_stream(h), logic_error); 
}

/*
  The following test is to check that a user gets the message that is is not a good idea to 
  use a stream derived from CThreadMsgStream as master stream for threaded output. 
  Unortunately, this can not be checked at compile time. 
*/
class WeAreBoned : public CThreadMsgStream {

}; 

BOOST_AUTO_TEST_CASE( test_set_are_we_boned  )
{
	WeAreBoned s; 
	BOOST_CHECK_THROW(CThreadMsgStream::set_master_stream(s), logic_error); 
}
