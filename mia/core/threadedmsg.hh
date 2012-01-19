/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#ifndef mia_core_threadedmsg_hh
#define mia_core_threadedmsg_hh

#include <ostream>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup infrastructure 
   \brief This class is used to handle syncronizized output of logging output in a multi-threaded environment

   This class is used to syncronize the output of the logging stream cverb and its
   helper functions cvmsg(), cvdebug(), cverror(), cvwarning() ... 
   
   To use it, just declare a variable of type CThreadMsgStream at the beginning of the 
   threaded function. 
   Output is only written to the master output stream if a newline is sent or explicit 
   syncronization via flush() is requested.
   
   Note, that a CThreadMsgStream itself can not serve as master stream since it would deadlock. 
*/
class EXPORT_CORE CThreadMsgStream : public std::ostream {
public: 
	/**
	   Constructor. This constructor sets the thread-local output of the vstream backend to itself 
	   and saves to old output. 
	 */
	CThreadMsgStream();

	/**
	   Destructor. This destructor flushes the output and then resets the thread-local vstream backend to the 
           original output. 
	 */
	~CThreadMsgStream();

	/**
	   Set the master output stream. The default is std::cerr. 
	   \param master the new master output stream; 
	   \remark if the new master is of type CThreadMsgStream a deadlock is certain.  
	 */
	template <typename OS>
	static void set_master_stream(OS& master); 
private: 
	
	static void do_set_master_stream(std::ostream& master); 
	
	template <typename OS, typename stupid>
	struct __dispatch_set_master_stream {
		static void apply(OS &os); 
	}; 
	
	template <typename OS, typename stupid> friend struct  __dispatch_set_master_stream; 
	std::ostream& m_old; 
}; 

//  Some logic to tell the user that passing a CThreadMsgStream as master stream is not possible 
template <typename OS, typename stupid>
void CThreadMsgStream::__dispatch_set_master_stream<OS, stupid>::apply(OS& master)
{
	CThreadMsgStream::do_set_master_stream(master); 
}

template <typename stupid>
struct CThreadMsgStream::__dispatch_set_master_stream<CThreadMsgStream, stupid> {
	static void apply(CThreadMsgStream& master)
	{
		static_assert(sizeof(stupid) == 0, "CThreadMsgStream can't be used as master stream because it would deadlock."); 
	}
}; 

template <typename OS>
void CThreadMsgStream::set_master_stream(OS& master)
{
	__dispatch_set_master_stream<OS, int>::apply(master); 
}



NS_MIA_END

#endif
