/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#ifndef mia_core_cmdlineparser_hh
#define mia_core_cmdlineparser_hh

#include <miaconfig.h>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <iterator>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/flagstring.hh>
#include <mia/core/handlerbase.hh>

NS_MIA_BEGIN

/// the string defining the name of the help options 
extern EXPORT_CORE const char *g_help_optiongroup; 

/** holds the history info of a certain program call */
typedef std::map<std::string, std::string> CHistoryRecord;

/** holds the name of the program and the associated history record */
typedef std::pair<std::string, CHistoryRecord> CHistoryEntry;

class CCmdOption;

/// Class to provide a maping from short option names to options
typedef std::map<char,  CCmdOption *>        CShortoptionMap;

/// Class to provide a maping from long option names to options
typedef std::map<std::string,  CCmdOption *> CLongoptionMap;


/**
    Data structure to provied help information oabout the program 
 */
struct SProgramDescrption {
	
	/// group ths program belongs to
	const char *group; 
	
	/// a basic description of the program 
	const char *description; 
	
	/// an example of the usage (description)
	const char *example_descr; 
	
	/// an example of the usage (sell code) 
	const char *example_code; 
}; 

/** 
    \ingroup infrastructure 

    \brief The base class for all command line options. 

    The base class of all possible command line options. It defines the interface
    of the options as well as some basic functionality to create help strings. 
*/
class EXPORT_CORE CCmdOption  {
public:
	typedef std::map<std::string, const CPluginHandlerBase*> HandlerHelpMap; 

	/**
	   Option flags 
	*/
	enum Flags {
		not_required = 0, /**< option not required */
		required = 1      /**< option required     */
	}; 
        /** The constructor
	    \param short_opt the short option character 
	    \param long_opt the long option name
	    \param long_help a long help string
	    \param short_help help to print out when only usage information is requested 
	    \param flags option flags 
        */
	CCmdOption(char short_opt, const char *long_opt, const char *long_help, 
		   const char *short_help, Flags flags);

        /// ensure virtual destruction
	virtual ~CCmdOption();

        /** Add this option to the given option maps
	    \param sm a \a CShortoptionMap to add this option to
	    \param lm a \a CLongoptionMap to add this option to
        */
	void add_option(CShortoptionMap& sm, CLongoptionMap& lm);

        /** \returns how many arguments after the option argument on the command 
	    line will be used by this option
        */
	size_t get_needed_args() const;

        /** Writes out the value of the option to an output stream
	    \param os the output stream to write this options value to
        */
	void   write_value(std::ostream& os) const;

        /** Set the value of the option parameter based on the input string
	    \param str_value the string value to be vonverted to the option value
        */
	void set_value(const char *str_value);

        /// \returns the long name of the option
	const char *get_long_option() const;

        /// \returns the long name of the option
	char get_short_option() const;

        /** Print the short help of the option to an output stream
	    \param os the output stream
        */
	void print_short_help(std::ostream& os) const;

        /** Print the help of the option to an output stream
	    \param os the output stream
         */
        void get_opt_help(std::ostream& os)const;

        /** Print the long option name of the option to an output stream
	    \param os the output stream
         */
	void get_long_help(std::ostream& os)const;

        /// \returns the options value as string
	const std::string get_value_as_string() const;

	/// @returns true if the option is a required option and hasn't been set
	bool is_required() const; 

        /// \returns the long help string
	const char *long_help() const;

	std::string get_long_help_xml(HandlerHelpMap& handler_map) const; 
protected:

	/// clear the "required" flag 
	void clear_required(); 
private:
	const char *get_short_help() const;

	virtual void do_add_option(CShortoptionMap& sm, CLongoptionMap& lm);
	virtual void do_print_short_help(std::ostream& os) const;
	virtual void do_get_opt_help(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const = 0;
	virtual bool do_set_value(const char *str_value) = 0;
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const = 0;

	virtual const std::string do_get_value_as_string() const;

	virtual void do_get_long_help_xml(std::ostream& os, HandlerHelpMap& handler_map) const; 

	char m_short_opt; 
	const char *m_long_opt;
	const char *m_long_help;
	const char *m_short_help;
	Flags m_flags;
};

/// a shared pointer definition of the Option
typedef std::shared_ptr<CCmdOption > PCmdOption;

/** 
    \brief Templated version based on CCmdOptionValue for values that can be converted to 
    and from strings by stream operators 

    Templated implementation of a command line option to hold a value of type T.
    \tparam T value parameter, the type T has to support the operators
    "std::ostream& operator << (std::ostream& os, T x)" and
    "std::istream& operator >> (std::istream& os, T x)"
    must be defined.
    f the string passed to the option for translation can not be translation to a 
    value of type T,
    the \a set_value method will throw a \a std::invalid_argument exception
    If T is of type template &lt; typename R &gt; std::vector &lt; R &gt;,  
    the list of N values needs to be  given like value1,value2,...,valueN.
*/
template <typename T>
class TCmdOption: public  CCmdOption{

public:
        /** Constructor of the command option
	    \param[in,out] val variable to hold the parsed option value - pass in the default value -
            exception: \a bool values always default to \a false
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param flags support options like required 
        */
	TCmdOption(T& val, char short_opt, const char *long_opt, const char *long_help,
                   const char *short_help, CCmdOption::Flags flags = CCmdOption::not_required);

private:

	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	T& m_value;
};

/** \brief A command line option that uses a dictionary to map strings to values. 

    Command line option that translates a set of string values to corresponding 
    values of type \a T given in a table \a TDictMap<T>. 
    If a string is passed to the option that is not defined in the table,
    it will throw a \a std::invalid_argument exception.
*/

template <typename T>
class TCmdDictOption: public  CCmdOption{

public:
        /** Constructor of the command option
	    \param[in,out] val variable to hold the parsed option value - pass in the default value -
	    \param map the lookup table for the option
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param flags set whether the option is required or not 
	    the option is really set
	*/
	TCmdDictOption(T& val, const TDictMap<T>& map, char short_opt, const char *long_opt,
                       const char *long_help, const char *short_help, Flags flags = not_required);
private:
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	T& m_value;
	const TDictMap<T> m_map;
};


/** 
    \brief Command line option that translates a string to a set of flags.
*/

class CCmdFlagOption: public  CCmdOption{

public:
        /** Constructor of the command option
	    \param[in,out] val variable to hold the parsed option value - pass in the default value -
	    \param map the lookup table for the option
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param flags if this is set to true, extra checking will be done weather
	    the option is really set
	*/
	CCmdFlagOption(int& val, const CFlagString& map, char short_opt, const char *long_opt,
                       const char *long_help, const char *short_help, Flags flags = not_required);
private:
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	int& m_value;
	const CFlagString m_map;
};

/** \brief A command line option that supports a limited set of values interpreted as strings. 
    
    Command line option that supports only a limited number of values 
    (given and interpreted as strings)
    If the string given to the option is not element of the set of allowed values, 
    the \a set_value method will throw a \a std::invalid_argument exception.
*/
class EXPORT_CORE CCmdSetOption: public CCmdOption{
public:
         /** Constructor of the command option
	     \param[in,out] val variable to hold the parsed option value - pass in the default value -
	     \param set the set of allowed values for the option
	     \param short_opt short option name (or 0)
	     \param long_opt long option name (must not be NULL)
	     \param long_help long help string (must not be NULL)
	     \param short_help short help string
	     \param flags if this is set to true, extra checking will be done weather
	     the option is really set
         */

	CCmdSetOption(std::string& val, const std::set<std::string>& set, char short_opt, 
		      const char *long_opt, const char *long_help, 
		      const char *short_help, Flags flags = not_required);
private:
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	std::string& m_value;
	const std::set<std::string> m_set;
};

/**
   \brief A command line option that will appear in the help group 
   and exits the program after printing the help. 

   Option that will appear in the help group and setting it will 
   always terminate the program after printing out the requested 
   help. 
 */
class EXPORT_CORE CHelpOption: public CCmdOption {
public:

	
	class Callback {
	public:
		virtual void print(std::ostream& os) const = 0;
	};

	/** Constructor of the command option
	     \param cb callback to call when help option is requested
	     \param short_opt short option name (or 0)
	     \param long_opt long option name (must not be NULL)
	     \param long_help long help string (must not be NULL)
         */
	CHelpOption(Callback *cb, char short_opt, const char*long_opt, const char *long_help);

	/** Print the option to a stream 
	    @param os 
	*/
	void print(std::ostream& os) const;

private:
	std::unique_ptr<Callback> m_callback;
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
};



template <typename PluginHandler>
class TPluginHandlerHelpCallback: public CHelpOption::Callback {
	void print(std::ostream& os) const{
		PluginHandler::instance().print_help(os);
	}
}; 


/**
   \brief The class to hold the list of options

   This class holds all the user defined and default command line option, 
   handles the parsing and the printing of help. 
   \todo the whole command line option class structure needs a code cleanup 
*/
class EXPORT_CORE CCmdOptionList {
public:

	/**
	   This enum describes the type of help information that has been requested 
	*/
	enum EHelpRequested {
		hr_no = 0,    /**< no help has been requested */
		hr_help,      /**< standard help output has been requested */
		hr_help_xml,  /**< XML-formatted help has been requested */
		hr_usage,     /**< a short usage description has been requested */ 
		hr_copyright  /**< The long copyright information has been requested */
	};

        /**
	   Constructor creates the options list and adds some defaut options like
	   --help, --verbose, --copyright, and --usage

	   \param description give a description of the program 
        */
	CCmdOptionList(const SProgramDescrption& description); 

        /// cleanup
	~CCmdOptionList();

        /** add a new option to the option list
	    \param opt the option to add
        */
	void add(PCmdOption opt);

        /** Add a new option to an option group
	    \param group option group to add this option to
	    \param opt the option to add
        */
	void add(const std::string& group, PCmdOption opt);

        /** the work routine, can take the arguemnts straight from \a main
	    This version parses the command line and allows for additional arguments that can be 
	    read by get_remaining(). 
	    \param argc number of arguments
	    \param args array of arguments strings
	    \param additional_type will is a help string to describe the type of free parameters
        */
	EHelpRequested parse(size_t argc, char *args[], const std::string& additional_type) 
		__attribute__((warn_unused_result));

        /** the work routine, can take the arguemnts straight from \a main
	    This version parses the command line and allows for additional arguments that can be 
	    read by get_remaining(). 
	    \param argc number of arguments
	    \param args array of arguments strings
	    \param additional_type will is a help string to describe the type of free parameters
        */
	EHelpRequested parse(size_t argc, const char *args[], const std::string& additional_type) 
		__attribute__((warn_unused_result));

        /** the work routine, can take the arguemnts straight from \a main
	    This version parses doesn't allow additional parameters. 
	    \param argc number of arguments
	    \param args array of arguments strings
        */
	EHelpRequested parse(size_t argc, char *args[]) __attribute__((warn_unused_result));
	
	/** the work routine, can take the arguemnts straight from \a main
	    This version parses doesn't allow additional parameters. 
	    \param argc number of arguments
	    \param args array of arguments strings
        */
	EHelpRequested parse(size_t argc, const char *args[]) __attribute__((warn_unused_result));
	
		/// \returns a vector of the remaining arguments
	const std::vector<const char *>& get_remaining() const;

        /** \returns the values of all arguments as a history record to support tracking
	    of changes on data worked on by \a mia
        */
	CHistoryRecord get_values() const;

	/**
	   Set the option group to add subsequent options to 
	   @param group 
	 */
	void set_group(const std::string& group); 
	
	/**
	   Set the output stream for help/usage messages
	   \param os new output stream
	 */
	void set_logstream(std::ostream& os); 
private:
	EHelpRequested do_parse(size_t argc, const char *args[], bool has_additional) 
		__attribute__((warn_unused_result));
	int handle_shortargs(const char *arg, size_t argc, const char *args[]);
	struct CCmdOptionListData *m_impl;
};


// implementation of template classes and functions


// standard handler for option <-> "value" translators
template <typename T>
struct __dispatch_opt {
        static void init(T& /*value*/){

        }
        static bool  apply(const char *svalue, T& value) {
                std::istringstream sval(svalue);

                sval >> value;
                while (isspace(sval.peek())) {
                        char c;
                        sval >> c;
                }
                return sval.eof();
        }
        static size_t size(const T /*value*/) {
                return 1;
        }
        static void apply(std::ostream& os, const T& value, bool /*required*/) {
                os << "=" << value << " ";
        }

        static const std::string get_as_string(const T& value) {
                std::ostringstream os;
                os << value;
                return os.str();
        }
};

// standard handler for option <-> "vector of values" translators
template <typename T>
struct __dispatch_opt< std::vector<T> > {
        static void init(std::vector<T>& /*value*/){

        }
        static bool  apply(const char *svalue, std::vector<T>& value) {
		std::string h(svalue);
		for(std::string::iterator hb = h.begin(); hb != h.end(); ++hb)
			if (*hb == ',')
				*hb = ' ';

                std::istringstream sval(h);

		while (!sval.eof()) {
			T s;
			sval >> s;
			value.push_back(s);
		}
                return sval.eof();
        }

        static size_t size(const std::vector<T>& /*value*/) {
                return 1;
        }

        static void apply(std::ostream& os, const std::vector<T>& value, bool required) {

                os << "=";
		if (value.empty() && required)
			os << "[required] "; 
		else {
			for (typename std::vector<T>::const_iterator i = value.begin(); i != value.end(); ++i) {
				if (i != value.begin())
					os << ",";
				os << *i;
			}
			os << " ";
		}
        }

        static const std::string get_as_string(const std::vector<T>& value) {
                std::ostringstream os;
		for (typename std::vector<T>::const_iterator i = value.begin(); i != value.end(); ++i) {
			if (i != value.begin())
				os << ",";
			os << *i;
		}
                return os.str();
        }
};

// handler for option <-> "bool values" translators
template <>
struct __dispatch_opt<bool> {
        static void init(bool& value) {
                value = false;
        }
        static bool apply(const char */*svalue*/, bool& value) {
                value = true;
                return true;
        }
        static size_t size(bool /*value*/) {
                return 0;
        }
        static void apply(std::ostream& /*os*/, bool /*value*/, bool /*required*/) {
        }
        static const std::string get_as_string(const bool& value) {
                return value ? "true" : "false";
        }
};


// handler for option <-> "string values" translators
template <>
struct __dispatch_opt<std::string> {
	static void init(std::string& /*value*/) {
        }
        static bool apply(const char *svalue, std::string& value) {
                value = std::string(svalue);
                return true;
        }
        static size_t size(std::string /*value*/) {
                return 1;
        }
        static void apply(std::ostream& os, const std::string& value, bool required) {
		if (value.empty()) 
			if (required)
				os << "[required] "; 
			else
				os << "=NULL ";
		else 
			os << "=" << value;
        }
        static const std::string get_as_string(const std::string& value) {
                return value;
        }
};


//
// Implementation of the standard option that holds a value
//
template <typename T>
TCmdOption<T>::TCmdOption(T& val, char short_opt, const char *long_opt, const char *long_help,
                        const char *short_help, 
			  CCmdOption::Flags flags):
        CCmdOption(short_opt, long_opt, long_help, short_help, flags),
        m_value(val)
{
        __dispatch_opt<T>::init(m_value);
}

template <typename T>
bool TCmdOption<T>::do_set_value(const char *svalue)
{
        return __dispatch_opt<T>::apply(svalue, m_value);
}

template <typename T>
size_t TCmdOption<T>::do_get_needed_args() const
{
        return __dispatch_opt<T>::size(m_value);
}

template <typename T>
void TCmdOption<T>::do_get_long_help(std::ostream& /*os*/) const
{
}

template <typename T>
void TCmdOption<T>::do_write_value(std::ostream& os) const
{
        __dispatch_opt<T>::apply( os, m_value, is_required());
}


template <typename T>
        const std::string TCmdOption<T>::do_get_value_as_string() const
{
        return __dispatch_opt<T>::get_as_string(m_value);
}

//
// Implementation of the dictionary option
//

template <typename T>
TCmdDictOption<T>::TCmdDictOption( T& val, const TDictMap<T>& map, char short_opt, 
				   const char *long_opt, const char *long_help,
                                   const char *short_help, Flags flags ) :
                CCmdOption( short_opt, long_opt, long_help, short_help, flags ),
                m_value( val ),
m_map( map ) {}

template <typename T>
bool TCmdDictOption<T>::do_set_value( const char *svalue )
{
        m_value = m_map.get_value( svalue );
        return true;
}

template <typename T>
size_t TCmdDictOption<T>::do_get_needed_args() const
{
        return 1;
}

template <typename T>
void TCmdDictOption<T>::do_write_value( std::ostream& os ) const
{
        os << "=" << m_map.get_name( m_value );
}

template <typename T>
const std::string TCmdDictOption<T>::do_get_value_as_string() const
{
        return m_map.get_name( m_value );
}

template <typename T>
void TCmdDictOption<T>::do_get_long_help( std::ostream& os ) const
{
        const std::set<std::string> names_help = m_map.get_name_set();
	for (auto i = m_map.get_help_begin(); i != m_map.get_help_end(); ++i) {
		os << "\n  " << i->second.first << ": " << i->second.second; 
	}
}



/**
   Convinience function: Create a standard option
   \param value value variable to hold the parsed option value - pass in the default value -
   exception: \a bool values always default to \a false
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
*/
template <typename T>
PCmdOption make_opt(T& value, const char *long_opt, char short_opt, 
		    const char *long_help,
		    const char *short_help, CCmdOption::Flags flags = CCmdOption::not_required)
{
	return PCmdOption(new TCmdOption<T>(value, short_opt, long_opt, long_help, 
					    short_help, flags ));
}


/**
   Convinience function: Create a standard option
   \param value value variable to hold the parsed option value - pass in the default value -
   exception: \a bool values always defaults to \a false
   \param short_opt short option name (or 0)
   \param long_opt long option name (must not be NULL)
   \param help long help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
*/
template <typename T>
PCmdOption make_opt(T& value, const char *long_opt, char short_opt, 
		    const char *help, CCmdOption::Flags flags = CCmdOption::not_required)
{
	return PCmdOption(new TCmdOption<T>(value, short_opt, long_opt, help, 
					    long_opt, flags ));
}

/**
   Convinience function: Create a table lookup option
   \param[in,out] value variable to hold the parsed and translated option value
   \param map the lookup table for the option
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
 */

template <typename T>
PCmdOption make_opt(T& value, const TDictMap<T>& map, const char *long_opt, char short_opt,
		    const char *long_help, const char *short_help, 
		    CCmdOption::Flags flags = CCmdOption::not_required)
{
	return PCmdOption(new TCmdDictOption<T>(value, map, short_opt, long_opt,
                          long_help, short_help, flags ));
}

/**
   Convinience function: Create a table lookup option
   \param[in,out] value variable to hold the parsed and translated option value
   \param map the lookup table for the option
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
 */

template <typename T>
PCmdOption make_opt(T& value, const TDictMap<T>& map, const char *long_opt, char short_opt,
		    const char *help, CCmdOption::Flags flags = CCmdOption::not_required)
{
	return PCmdOption(new TCmdDictOption<T>(value, map, short_opt, long_opt,
						help, long_opt, flags ));
}


/**
   Convinience function: Create a flag lookup option
   \param[in,out] value variable to hold the parsed and translated option value
   \param map the lookup table for the option flags
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
 */

PCmdOption  EXPORT_CORE make_opt(int& value, const CFlagString& map, const char *long_opt,
				 char short_opt, const char *long_help, 
				 const char *short_help, 
				 CCmdOption::Flags flags = CCmdOption::not_required);


/**
   Convinience function: Create a set restricted option
   \param[in,out] value variable to hold the parsed and translated option value
   \param set the set of allowed values
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
 */
PCmdOption EXPORT_CORE make_opt(std::string& value, const std::set<std::string>& set,
                                const char *long_opt, char short_opt, const char *long_help,
                                const char *short_help, CCmdOption::Flags flags);



/**
   Convinience function: Create a set restricted option
   \param[in,out] value variable to hold the parsed and translated option value
   \param set the set of allowed values
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help long help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
 */
PCmdOption EXPORT_CORE make_opt(std::string& value, const std::set<std::string>& set,
                                const char *long_opt, char short_opt, 
				const char *help, 
				CCmdOption::Flags flags = CCmdOption::not_required);


PCmdOption EXPORT_CORE make_help_opt(const char *long_opt, char short_opt, 
				     const char *long_help, CHelpOption::Callback* cb); 

NS_MIA_END

#endif
