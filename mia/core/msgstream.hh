/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

// $Id: miaIOStream.hh 936 2006-07-10 13:32:49Z write1 $

/*! \brief A verbose output stream

*/

#ifndef CVERB_HH
#define CVERB_HH 1

//#pragma interface
#include <vector>
#include <cassert>
#include <ostream>
#include <boost/call_traits.hpp>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>

NS_MIA_BEGIN

#ifndef VSTREAM_DOMAIN
#define VSTREAM_DOMAIN "**"
#endif

/**
   This class impelemtns a std::ostream like class to output messages during run-time, 
   supporting various levels of verbosity. The class is implemened as a singleton.
   On initialization it is set to use std::cerr as output stream, but this can be replaced 
   by any type implementing the std::ostream interface. 
   \remark The levels \a debug and \a trace are not compiled in, if -DNDEBUG is set - i.e. 
   the corresponding output operators are replaced by dummy functions that should be 
   optimized away. 
*/
class EXPORT_CORE vstream {
public:
	enum Level {
		ml_trace,
		ml_debug,
		ml_info,
		ml_message,
		ml_warning,
		ml_fail,
		ml_error,
		ml_fatal,
		ml_undefined
	};

	/** initialise a  stream that writes only messages above a certain
	    verbosity level
	*/

	static vstream& instance();
	/** set the verbosity output level
	    \param l
	*/
	void set_verbosity(Level l);

	/// \returns the curent verbosity level
	vstream::Level get_level() const;

	/**
	    sets the output stream
	    \param os
	    \returns the old output stream
	*/
	std::ostream&  set_stream(std::ostream& os);

	void flush();

	/** \param l verbosity level
	    \returns true if cverb will show the given verbosity level
	*/
	bool shows(Level l)const;
	
	/// \returns true if the output level is equal or below "debug" 
	bool show_debug() const;


	/** a special handling to set the output level "inline"
	    \param l the level of the following messages
	*/
	vstream& operator << (Level const l);

	/** general output routine; output is only given, if the data verbosity level is
	    set higher or equal to the stream verbosity level.
	    \param text the text to be written to the stream
	    \returns a reference to this object
	*/
	template <class T>
	vstream& operator << (const T&  text) {
		if (_M_message_level >= _M_output_level)
			*_M_output << text;
		return *this;
	}

	/** A funny construct to enable std::endl to work on this stream
	    idea of Dave Brondsema:
	    http://gcc.gnu.org/bugzilla/show_bug.cgi?id=8567
	*/
	vstream & operator<<(std::ostream& (*f)(std::ostream&));



	/**
	   Set the output target with which the stream should be initialized 
	   if std::cerr is not to be used. 
	 */
	static void set_output_target(std::ostream* os);

	/**
	   Transparent conversion operator to an std::ostream. 
	 */
	operator std::ostream& () {
		return *_M_output;
	}

private:
	vstream(std::ostream& output, Level l);

	static std::ostream* _M_output_target;
	std::ostream* _M_output;
	Level _M_output_level;
	Level _M_message_level;

};


extern EXPORT_CORE const TDictMap<vstream::Level> g_verbose_dict;

/**
 * @param verbose: verbose state
 *
 *  Set the cverb ostream in a verbose/non verbose mode depending on the
 * verbose parameter. Currently set_verbose() can be called only one time. If
 * this function is never called the default state of cverb is non-verbose mode
 */
void set_verbose(bool verbose);


inline bool vstream::shows(Level l)const
{
	return l >= _M_output_level;
}


#ifdef NDEBUG
#define TRACE(DOMAIN)
#define TRACE_FUNCTION
#define FUNCTION_NOT_TESTED
class CDebugSink {
public:
	template <class T>
	CDebugSink& operator << (const T val) {
		return *this;
	}
	CDebugSink & operator<<(std::ostream& (*f)(std::ostream&)) {
		return *this;
	}
};

inline CDebugSink& cvdebug()
{
	static CDebugSink sink;
	return sink;
}

#else

inline vstream& cvdebug()
{
	vstream::instance() << vstream::ml_debug << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

class EXPORT_CORE CTrace {
public:
	CTrace(const char *domain):
		_M_domain(domain),
		_M_fill(_M_depth, ' ')  {
		vstream::instance() << vstream::ml_trace
				    << _M_fill << "enter " << _M_domain  << "\n";
		++_M_depth;
	};
	~CTrace() {
		vstream::instance() << vstream::ml_trace
				    << _M_fill << "leave " << _M_domain  << "\n";
		--_M_depth;
	}
private:
	const char *_M_domain;
	std::string _M_fill;
	// should be thread local, or at least protected by a mutex
	static size_t _M_depth;
};

/// a macro to trace scopes in a debug built
#define TRACE(DOMAIN) ::mia::CTrace _xxx_trace(DOMAIN)
#define TRACE_FUNCTION ::mia::CTrace _xxx_trace(__PRETTY_FUNCTION__)

/// a macro to indicate that there are no tests for a function 
#define FUNCTION_NOT_TESTED ::mia::cvwarn() << __PRETTY_FUNCTION__ << ":not tested\n"

#endif

inline vstream& cvinfo()
{
	vstream::instance() << vstream::ml_info << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

inline bool vstream::show_debug() const
{
	return shows(ml_debug);
}


/// \returns the curent verbosity level
inline vstream::Level vstream::get_level() const
{
	return _M_output_level;
}

inline void vstream::flush()
{
	_M_output->flush();
}

// some inlines

inline vstream& cvfatal()
{
	vstream::instance() << vstream::ml_fatal << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

inline vstream& cvfail()
{
	vstream::instance() << vstream::ml_fail << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}


inline vstream& cverr()
{
	vstream::instance() << vstream::ml_error << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

inline vstream& cvwarn()
{
	vstream::instance() << vstream::ml_warning << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

inline vstream& cvmsg()
{
	vstream::instance() << vstream::ml_message << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

#define cverb ::mia::vstream::instance()

/**
   Impelment the direct streaming of std::vectors. 
*/
template <typename T> 
vstream& operator << (vstream& os, const std::vector<T>& v) {
	os << "["; 
	for (auto i =v.begin(); i != v.end(); ++i) 
		os << *i << ", "; 
	os << "]"; 
	return os; 
}

NS_MIA_END

#endif /* !CVERB_HH */
