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

// $Id: parallel.hh 928 2006-06-27 10:37:53Z write1 $


#ifndef mia_core_parallel_hh
#define mia_core_parallel_hh

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/noncopyable.hpp>
#include <boost/call_traits.hpp>
#include <mia/core/shared_ptr.hh>
#include <boost/any.hpp>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   @brief A message queue for thread syncronisation in a Master-Slave setting 

   This class implements a message queue that can be used for thread syncronisation and
   Master-Slave processing.

*/

class EXPORT_CORE CMessenger: private boost::noncopyable{
public:

	/**
	   Constructor of the messanger
	   \param nslaves number of slave threads to be handled - important for barriers.
	 */
	CMessenger(size_t nslaves);

	/**
	   the global command code to finish the slave threads
	 */
	static const int cmd_finish = 0;

	/**
	   send a command and wait, till all slave threads are done with it
	   \param command the command code
	   \param param some parameter that can be passed to the slaves
	 */
	void send_command_and_wait(int command, boost::any *param = 0);

	/**
	   This function just sits and waits until a command is send by another thread.
	   \returns the command code received
	   \remark currently this function also waits at a barrier, until all slave threads
	   are waiting for the command.
	 */
	int wait_for_command();

	/**
	   Tell the master thread that we are finished.
	 */
	void send_command_finished();

	/**
	    \returns the parameter
	 */
	boost::any *get_param();
private:
	boost::condition m_send_condition;
	boost::mutex m_send_mutex;
	int m_command;
	boost::any *m_param;

	boost::condition m_receive_condition;
	boost::mutex m_receive_mutex;
	boost::barrier m_precmd_barrier;
	bool m_finished;

};

/// Pointer type of the CMessage class 
typedef std::shared_ptr<CMessenger > PMessenger;

/// Pointer type of the CBarrier class  
typedef std::shared_ptr<boost::barrier > PBarrier;


/**
   @brief The base of all slave threads.
*/
class EXPORT_CORE CSlave: private boost::noncopyable {
public:
	/**
	   Constructor of the slave thread
	   \param messenger the syncronising object
	   \param final_barrier the barrier handling the post-command syncronisation
	   \remark why is it not part of the messanger?
	   \param nr the number of the slave
	 */
	CSlave(PMessenger messenger, PBarrier final_barrier, size_t nr);

	/// ensure virtual destruction
	virtual ~CSlave();

	/** the main loop of the slave, it handles all the syncronisation tasks and
	    calls \a run_command which must be implemented in a derived class.
	*/
	void operator ()();
protected:
	/// \returns the number of the slave
	size_t nr() const;
private:
	virtual void run_command(int command, boost::any *param) = 0;
	PMessenger m_messanger;
	PBarrier m_final_barrier;
	size_t m_nr;
};


/**
   @brief Generic class to provide syncronised access to a value

   This class holds a value of thegiven type and all access to it is scncronized.  
   @remark some of these things could probably done with atomic operations 
*/
template <typename T>
class TLockedValue:private boost::noncopyable {
public:

	TLockedValue();
	
	/// constructor to set the value 
	TLockedValue(T val);

	/// assignment operator 
	const TLockedValue& operator = (T val);

	/// operator for transparent access to the value 
	operator T() const;

	/// Set the value @param val 
	void set (T val);

	/// @returns the value 
	const T get() const;

	/// operator of the value 
	const TLockedValue<T>& operator += (T val);
private:
	T m_val;
	boost::mutex m_mutex;
};


inline size_t CSlave::nr() const
{
	return m_nr;
}

// implementation
inline boost::any *CMessenger::get_param()
{
	return m_param;
}

template <typename T>
TLockedValue<T>::TLockedValue():
	m_val()
{
}

template <typename T>
TLockedValue<T>::TLockedValue(T val):
	m_val(val)
{
}

template <typename T>
const TLockedValue<T>& TLockedValue<T>::operator = (T val)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_val = val;
}

template <typename T>
TLockedValue<T>::operator T() const
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_val;
}

template <typename T>
void TLockedValue<T>::set (T val)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_val = val;
}

template <typename T>
const T TLockedValue<T>::get() const
{
	boost::mutex::scoped_lock lock(m_mutex);
	return m_val;
}

template <typename T>
const TLockedValue<T>& TLockedValue<T>::operator += (T val)
{
	boost::mutex::scoped_lock lock(m_mutex);
	m_val += val;
	return *this;
}

NS_MIA_END
#endif
