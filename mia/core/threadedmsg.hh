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


#ifndef mia_core_threadedmsg_hh
#define mia_core_threadedmsg_hh

#include <ostream>
#include <sstream> 
#include <tbb/mutex.h>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

class EXPORT_CORE thread_streamredir: public std::streambuf  {
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

class CThreadMsgStream : public std::ostream {
public: 
	CThreadMsgStream():std::ostream(new thread_streamredir()), 
			   m_old(vstream::instance().set_stream(*this))
	{
		
	}
	~CThreadMsgStream()
	{
		vstream::instance().set_stream(m_old);
	}
private: 
	std::ostream& m_old; 
}; 

NS_MIA_END

#endif
