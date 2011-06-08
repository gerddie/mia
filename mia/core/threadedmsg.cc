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

#include <tbb/mutex.h>
NS_MIA_BEGIN
using std::setw; 
using std::setfill; 


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
	send_to_master(); 
	
}

int thread_streamredir::overflow(int c)
{
	send_to_master(); 
	m_buffer << (char)c; 
}

std::streamsize thread_streamredir::xsputn( const char * s, std::streamsize n )
{
	int i = 0; 
	while (i < n) {
		m_buffer << *s; 
		if (*s == '\n' || *s == '\r') 
			send_to_master();
		++s; 
		++i; 
	}
	return i; 
}

void thread_streamredir::send_to_master()
{
	CScopedLock lock(m_master_lock); 
	*m_master << "[thread " << setw(3) << setfill('0') << m_id << "]:" << m_buffer.str(); 
	m_buffer.str(""); 
	m_buffer.clear(); 
}

std::ostream *thread_streamredir::m_master = &std::cerr;
tbb::mutex thread_streamredir::m_master_lock; 
int thread_streamredir::m_next_id = 0; 

NS_MIA_END
