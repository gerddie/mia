/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_core_threadedmsg_hh
#define mia_core_threadedmsg_hh

#include <ostream>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup logging
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
	template <typename OS, typename placeholder> friend struct __dispatch_set_master_stream; 
	
	static void do_set_master_stream(std::ostream& master); 
	
	
	std::ostream& m_old; 

}; 

/** 
    @cond INTERNAL
    
    \ingroup traits
    \brief Structure to ensure the sane initialization of CThreadMsgStream
    
    This structure is used to capture and report an error when a developer tries 
    to use a CThreadMsgStream as master stream.
    
    \tparam OS the output stream type to be used as master stream 
    \tparam placeholder additional type to make sure the static_assert is only triggered if 
    OS==CThreadMsgStream
    
*/
template <typename OS, typename placeholder>
struct __dispatch_set_master_stream {
	static void apply(OS &master){
		CThreadMsgStream::do_set_master_stream(master); 
	}
	
}; 

template <typename placeholder>
struct __dispatch_set_master_stream<CThreadMsgStream, placeholder> {
	static void apply(CThreadMsgStream& /*master*/){
		static_assert(sizeof(placeholder) == 0, 
			      "CThreadMsgStream can't be used as master stream because it would deadlock."); 
	}
}; 

/// @endcond 

template <typename OS>
void CThreadMsgStream::set_master_stream(OS& master)
{
	__dispatch_set_master_stream<OS, int>::apply(master); 
}



NS_MIA_END

#endif
