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


#include <iostream>
#include <iomanip>
#include <mia/core/threadedmsg.hh>
#include <sstream> 
#include <tbb/mutex.h>
#include <stdexcept>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using std::setw; 
using std::setfill; 
using std::logic_error; 


class thread_streamredir: public std::streambuf  {
public: 
	thread_streamredir(); 

	static void set_master_stream(std::ostream& master); 
protected: 
	int sync(); 
	int overflow(int c); 
	std::streamsize xsputn ( const char * s, std::streamsize n ); 
private:
	void send_to_master(); 
	
	std::stringstream m_buffer; 
	int m_id; 
	static std::ostream *m_master;
	static tbb::mutex m_master_lock; 
	static int m_next_id; 
}; 


thread_streamredir::thread_streamredir()
{
	CScopedLock lock(m_master_lock); 
	m_id = m_next_id++; 
}
	

void thread_streamredir::set_master_stream(std::ostream& master)
{
	m_master = &master; 
}

int thread_streamredir::sync()
{
	if (!m_buffer.str().empty()) 
		send_to_master(); 
	return 0; 
}

int thread_streamredir::overflow(int c)
{
	send_to_master(); 
	m_buffer << (char)c; 
	return 0;
}

std::streamsize thread_streamredir::xsputn( const char * s, std::streamsize n )
{
	int i = 0; 
	while (i < n) {
		if (*s != '\n') 
			m_buffer << *s; 
		else
			send_to_master();
		++s; 
		++i; 
	}
	return i; 
}

void thread_streamredir::send_to_master()
{
	CScopedLock lock(m_master_lock); 
	*m_master << "[thread " << setw(3) << setfill('0') << m_id << "]:" << m_buffer.str() << "\n"; 
	m_buffer.str(""); 
	m_buffer.clear(); 
}

std::ostream *thread_streamredir::m_master = &std::cerr;
tbb::mutex thread_streamredir::m_master_lock; 
int thread_streamredir::m_next_id = 0; 


CThreadMsgStream::CThreadMsgStream():
	std::ostream(new thread_streamredir()), 
	m_old(vstream::instance().set_stream(*this))
{
}

CThreadMsgStream::~CThreadMsgStream()
{
	flush(); 
	vstream::instance().set_stream(m_old);
	delete rdbuf();
}

void CThreadMsgStream::do_set_master_stream(std::ostream& master)
{
	// if this is actually a CThreadMsgStream, then deadlock is certain 
	// hence throw and tell the programmer to fix the program
	if (dynamic_cast<CThreadMsgStream*>(&master)) {
		throw logic_error("CThreadMsgStream::set_master_stream: trying to set the master stream to a CThreadMsgStream. "
				  "Since this would make deadlock certain I bail out now.\nPlease fix your program\n"); 
	}
	thread_streamredir::set_master_stream(master); 
}

NS_MIA_END
