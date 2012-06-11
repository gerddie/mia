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
   \ingroup logging
   \brief A output stream to enable certain levels of verbosity 

   This class impelemtns a std::ostream like class to output messages during run-time, 
   supporting various levels of verbosity. The class is implemened as a singleton.
   On initialization it is set to use std::cerr as output stream, but this can be replaced 
   by any type implementing the std::ostream interface. 
   \remark if -DNDEBUG is set, the levels \a debug and \a trace are not compiled in - i.e. 
   the corresponding output operators are replaced by dummy functions that should be 
   optimized away. 
*/
class EXPORT_CORE vstream {
public:
	/**
	   Output verbosity level threshhold, ml_trace is the lowest threshhold and 
	   ml_fatal the highest. Output is written, when the output threshold is above or equal
	   to the one set within the output stream. 
	 */
	enum Level {
		ml_trace,  /**< write trace of function calls and higher, disabled with -DNDEBUG */
		ml_debug,  /**< write debugging information  and higher, disabled with -DNDEBUG */
		ml_info,   /**< write additional info about the data processed  and higher */
		ml_message,/**< write process status messages  and higher*/
		ml_warning,/**< write warnings  and higher*/
		ml_fail,   /**< write failture messages (for tests)  and higher */
		ml_error,  /**< write non-fatal error messages  and higher */
		ml_fatal,  /**< write only fatal error messages */
		ml_undefined /**< stopper */
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

	/// write pending output 
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
		if (m_message_level >= m_output_level)
			*m_output << text;
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
		return *m_output;
	}

private:
	vstream(std::ostream& output, Level l);

	static std::ostream* m_output_target;
	static __thread std::ostream* m_output;
	Level m_output_level;
	static __thread Level m_message_level;

};

/** 
    \ingroup logging
    \brief Dictonary for the verbosity of the logging as used by --verbose comand line option 
*/
extern EXPORT_CORE const TDictMap<vstream::Level> g_verbose_dict;

/**
   \ingroup logging
 * @param verbose: verbose state
 *
 *  Set the cverb ostream in a verbose/non verbose mode depending on the
 * verbose parameter. Currently set_verbose() can be called only one time. If
 * this function is never called the default state of cverb is non-verbose mode
 */
void set_verbose(bool verbose);


inline bool vstream::shows(Level l)const
{
	return l >= m_output_level;
}


#ifdef NDEBUG
#define TRACE(DOMAIN)
#define TRACE_FUNCTION
#define FUNCTION_NOT_TESTED
class CDebugSink {
public:
	template <class T>
	CDebugSink& operator << (const T /*val*/) {
		return *this;
	}
	CDebugSink & operator<<(std::ostream& (* /*f*/)(std::ostream&)) {
		return *this;
	}
};

inline CDebugSink& cvdebug()
{
	static CDebugSink sink;
	return sink;
}

#else

/** 
    \ingroup logging
    \brief  Short for debug output in non-debug build output send to this will be ignored. 
*/
inline vstream& cvdebug()
{
	vstream::instance() << vstream::ml_debug << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

class EXPORT_CORE CTrace {
public:
	CTrace(const char *domain):
		m_domain(domain),
		m_fill(m_depth, ' ')  {
		vstream::instance() << vstream::ml_trace
				    << m_fill << "enter " << m_domain  << "\n";
		++m_depth;
	};
	~CTrace() {
		vstream::instance() << vstream::ml_trace
				    << m_fill << "leave " << m_domain  << "\n";
		--m_depth;
	}
private:
	const char *m_domain;
	std::string m_fill;
	// should be thread local, or at least protected by a mutex
	static __thread size_t m_depth;
};

/// a macro to trace scopes in a debug built
#define TRACE(DOMAIN) ::mia::CTrace _xxx_trace(DOMAIN)

/// a macro to trace functions ina a debug build 
#define TRACE_FUNCTION ::mia::CTrace _xxx_trace(__PRETTY_FUNCTION__)

/// a macro to indicate that there are no tests for a function 
#define FUNCTION_NOT_TESTED ::mia::cvwarn() << __PRETTY_FUNCTION__ << ":not tested\n"

#endif

/**
   \ingroup logging
   \brief \a  informal output that may be of interest to understand problems with a program
   and are of higher priority then debugging output. 
 */
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
	return m_output_level;
}

inline void vstream::flush()
{
	m_output->flush();
}

// some inlines

/**
   \ingroup logging
   \brief direct output to this stream adapter to print out \a fatalities in the code 
*/
inline vstream& cvfatal()
{
	vstream::instance() << vstream::ml_fatal << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

/**
   \ingroup logging
   \brief direct output to this stream adapter to print out \a failtures in tests beyond BOOST_FAIL
*/
inline vstream& cvfail()
{
	vstream::instance() << vstream::ml_fail << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

/**
   \ingroup logging
   \brief send errors to this stream adapter 
*/
inline vstream& cverr()
{
	vstream::instance() << vstream::ml_error << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

/**
   \ingroup logging
   \brief send \a warnings to this stream adapter 
*/
inline vstream& cvwarn()
{
	vstream::instance() << vstream::ml_warning << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

/**
   \ingroup logging
   \brief send \a messages to this stream adapter 
*/
inline vstream& cvmsg()
{
	vstream::instance() << vstream::ml_message << VSTREAM_DOMAIN << ":";
	return vstream::instance();
}

/**
   \ingroup logging
   \brief define a shortcut to the raw output stream 
*/
#define cverb ::mia::vstream::instance()

/**
   \ingroup logging
   \brief implements the direct streaming of std::vectors. 
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
