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

#ifndef mia_core_cmdlineparser_hh
#define mia_core_cmdlineparser_hh

#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <iterator>
#include <mia/core/shared_ptr.hh>
#include <mia/core/defines.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/flagstring.hh>

NS_MIA_BEGIN



/** holds the history info of a certain program call */
typedef std::map<std::string, std::string> CHistoryRecord;

/** holds the name of the program and the associated history record */
typedef std::pair<std::string, CHistoryRecord> CHistoryEntry;

class CCmdOption;

/// Class to provide a maping from short option names to options
typedef std::map<char,  CCmdOption *>        CShortoptionMap;

/// Class to provide a maping from long option names to options
typedef std::map<std::string,  CCmdOption *> CLongoptionMap;

/** The base class of all possible command line options. It defines the interface
    of the options as well as some basic functionality to create help strings
    \todo "Add a start or set group command"
    \todo "Add a parameter to the parse command that tells whether additional parameters 
    are allowed. 
*/
class EXPORT_CORE CCmdOption  {
 public:
        /** The constructor
	    \param long_opt the long option name
	    \param longh_help a long help string
        */
	CCmdOption(const char *long_opt, const char *long_help, bool required);

        /// ensure virtual destruction
	virtual ~CCmdOption();

        /** Add this option to the given option maps
	    \param sm a \a CShortoptionMap to add this option to
	    \param lm a \a CLongoptionMap to add this option to
        */
	void add_option(CShortoptionMap& sm, CLongoptionMap& lm);

        /** \returns how many arguments after the option argument on the command line will be used
            by this option
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

	bool required() const; 
protected:
        /// \returns the long help string
	const char *long_help() const;

	void clear_required(); 
private:
	virtual void do_add_option(CShortoptionMap& sm, CLongoptionMap& lm)  = 0;
	virtual void do_set_value(const char *str_value) = 0;
	virtual size_t do_get_needed_args() const = 0;
	virtual void do_write_value(std::ostream& os) const = 0;

	virtual void do_print_short_help(std::ostream& os) const = 0;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual void do_get_opt_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	const char *_M_long_opt;
	const char *_M_long_help;
	bool _M_required;
};

/// a shared pointer definition of the Option
typedef SHARED_PTR(CCmdOption) PCmdOption;


/**
    The base class to options that really hold values
*/
class EXPORT_CORE CCmdOptionValue: public CCmdOption {
public:
        /** Construct the option with all the meta-data
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param required if this is set to true, extra checking will be done weather
	    the option is really set on the command line
        */
	CCmdOptionValue(char short_opt, const char *long_opt, const char *long_help,
                        const char *short_help, bool required = false);

        /// destroy some of the strings
	virtual ~CCmdOptionValue();
private:

	char get_short_option() const;
	const char *get_short_help() const;

	virtual void do_add_option(CShortoptionMap& sm, CLongoptionMap& lm);
	virtual void do_print_short_help(std::ostream& os) const;
	virtual void do_get_opt_help(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual void do_get_long_help_really(std::ostream& os) const;
	virtual void do_set_value(const char *str_value);
	virtual bool do_set_value_really(const char *str_value) = 0;

	struct CCmdOptionData *_M_impl;
};



/** Templated implementation of a command line option to hold a value of type T.
    For type \a T, the operators
    "std::ostream& operator << (std::ostream& os, T x)" and
    "std::istream& operator >> (std::istream& os, T x)"
    must be defined.
    If the string passed to the option for translation can not be translation to a value of type T,
    the \a set_value method will throw a \a std::invalid_argument exception
    If T is of type template <typename R> std::vector<R>,  the list of N values needs to be
    given like value1,value2,...,valueN.
*/
template <typename T>
class TCmdOption: public  CCmdOptionValue{

public:
        /** Constructor of the command option
	    \retval val variable to hold the parsed option value - pass in the default value -
            exception: \a bool values always default to \a false
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param required if this is set to true, extra checking will be done weather
	    the option is really set
        */
	TCmdOption(T& val, char short_opt, const char *long_opt, const char *long_help,
                   const char *short_help, bool required = false);

private:

	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	T& _M_value;
};

/** Command line option that translates a set of string values to corresponding values of type \a T
    given in a table \a TDictMap<T>. If a string is passed to the option that is not defined in the table,
    it will throw a \a std::invalid_argument exception.
*/

template <typename T>
class TCmdDictOption: public  CCmdOptionValue{

public:
        /** Constructor of the command option
	    \retval val variable to hold the parsed option value - pass in the default value -
	    \param map the lookup table for the option
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param required if this is set to true, extra checking will be done weather
	    the option is really set
	*/
	TCmdDictOption(T& val, const TDictMap<T>& map, char short_opt, const char *long_opt,
                       const char *long_help, const char *short_help, bool required);
private:
	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help_really(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	T& _M_value;
	const TDictMap<T> _M_map;
};


/** Command line option that translates a string to a set of flags.
*/

class CCmdFlagOption: public  CCmdOptionValue{

public:
        /** Constructor of the command option
	    \retval val variable to hold the parsed option value - pass in the default value -
	    \param map the lookup table for the option
	    \param short_opt short option name (or 0)
	    \param long_opt long option name (must not be NULL)
	    \param long_help long help string (must not be NULL)
	    \param short_help short help string
	    \param required if this is set to true, extra checking will be done weather
	    the option is really set
	*/
	CCmdFlagOption(int& val, const CFlagString& map, char short_opt, const char *long_opt,
                       const char *long_help, const char *short_help, bool required);
private:
	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help_really(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	int& _M_value;
	const CFlagString _M_map;
};

/** Command line option that supports only a limited number of values (given and interpreted as strings)
     If the string given to the option is not element of the set of allowed values, the \a set_value
     method will throw a \a std::invalid_argument exception.
*/

class EXPORT_CORE CCmdSetOption: public CCmdOptionValue{
public:
         /** Constructor of the command option
	     \retval val variable to hold the parsed option value - pass in the default value -
	     \param set the set of allowed values for the option
	     \param short_opt short option name (or 0)
	     \param long_opt long option name (must not be NULL)
	     \param long_help long help string (must not be NULL)
	     \param short_help short help string
	     \param required if this is set to true, extra checking will be done weather
	     the option is really set
         */

	CCmdSetOption(std::string& val, const std::set<std::string>& set, char short_opt, const char
                        *long_opt, const char *long_help, const char *short_help, bool required);
private:
	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help_really(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;

	std::string& _M_value;
	const std::set<std::string> _M_set;
};


class EXPORT_CORE CHelpOption: public CCmdOptionValue {
public:
         /** Constructor of the command option
	     \param callback to call when help option is requested
	     \param short_opt short option name (or 0)
	     \param long_opt long option name (must not be NULL)
	     \param long_help long help string (must not be NULL)
         */

	class Callback {
	public:
		virtual void print(std::ostream& os) const = 0;
	};

	CHelpOption(Callback *cb, char short_opt, const char*long_opt, const char *long_help);
	void print(std::ostream& os) const;

private:
	std::unique_ptr<Callback> _M_callback;
	virtual bool do_set_value_really(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;

};

/**
   Convinience function: Create a standard option
   \param value val variable to hold the parsed option value - pass in the default value -
   exception: \a bool values always default to \a false
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param required if this is set to true, extra checking will be done weather
   the option is really set
   \returns the option warped into a \a boost::shared_ptr
*/
template <typename T>
PCmdOption make_opt(T& value, const char *long_opt, char short_opt, const char *long_help,
		    const char *short_help, bool required= false)
{
	return PCmdOption(new TCmdOption<T>(value, short_opt, long_opt, long_help, short_help, required ));
}

/**
   Convinience function: Create a table lookup option
   \retval val variable to hold the parsed and translated option value
   \param map the lookup table for the option
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param required if this is set to true, extra checking will be done weather
   the option is really set
   \returns the option warped into a \a boost::shared_ptr
 */

template <typename T>
PCmdOption make_opt(T& value, const TDictMap<T>& map, const char *long_opt, char short_opt,
		    const char *long_help, const char *short_help, bool required= false)
{
	return PCmdOption(new TCmdDictOption<T>(value, map, short_opt, long_opt,
                          long_help, short_help, required ));
}

/**
   Convinience function: Create a flag lookup option
   \retval val variable to hold the parsed and translated option value
   \param map the lookup table for the option flags
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param required if this is set to true, extra checking will be done weather
   the option is really set
   \returns the option warped into a \a boost::shared_ptr
 */

PCmdOption  EXPORT_CORE make_opt(int& value, const CFlagString& map, const char *long_opt, char short_opt,
				 const char *long_help, const char *short_help, bool required= false);


/**
   Convinience function: Create a set restricted option
   \retval val variable to hold the parsed and translated option value
   \param set the set of allowed values
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param required if this is set to true, extra checking will be done weather
   the option is really set
   \returns the option warped into a \a boost::shared_ptr
 */
PCmdOption EXPORT_CORE make_opt(std::string& value, const std::set<std::string>& set,
                                const char *long_opt, char short_opt, const char *long_help,
                                const char *short_help, bool required = false);


PCmdOption make_help_opt(const char *long_opt, char short_opt, const char *long_help, 
			 CHelpOption::Callback* cb); 


/**
   The class to hold the list of options
*/
class EXPORT_CORE CCmdOptionList {
 public:
        /**
	   Constructor creates the options list and adds some defaut options like
	   --help, --verbose, --copyright, and --usage
        */
	CCmdOptionList(const std::string& general_help);
	CCmdOptionList()__attribute__((deprecated));

        /// cleanup
	~CCmdOptionList();

        /** add a new option to the option list
	    \param opt the option to add
        */
	void push_back(PCmdOption opt);

        /** Add a new option to an option group
	    \param group option group to add this option to
	    \param opt the option to add
        */
	void add(const std::string& group, PCmdOption opt);

        /** the work routine, can take the arguemnts straight from \a main
	    \param argc number of arguments
	    \param args array of arguments strings
        */
	void parse(size_t argc, const char *args[], bool has_additional = true);

        /** the work routine, can take the arguemnts straight from \a main
	    \param argc number of arguments
	    \param args array of arguments strings
        */
        void parse(size_t argc, char *args[], bool has_additional = true);

        /// \returns a vector of the remaining arguments
	const std::vector<const char *>& get_remaining() const;

        /** \returns the values of all arguments as a history record to support tracking
	    of changes on data worked on by \a mia
        */
	CHistoryRecord get_values() const;
 private:
	int handle_shortargs(const char *arg, size_t argc, const char *args[]);
	struct CCmdOptionListData *_M_impl;
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
                        const char *short_help, bool required):
        CCmdOptionValue(short_opt, long_opt, long_help, short_help, required),
        _M_value(val)
{
        __dispatch_opt<T>::init(_M_value);
}

template <typename T>
bool TCmdOption<T>::do_set_value_really(const char *svalue)
{
        return __dispatch_opt<T>::apply(svalue, _M_value);
}

template <typename T>
        size_t TCmdOption<T>::do_get_needed_args() const
{
        return __dispatch_opt<T>::size(_M_value);
}

template <typename T>
void TCmdOption<T>::do_write_value(std::ostream& os) const
{
        __dispatch_opt<T>::apply( os, _M_value, required());
}


template <typename T>
        const std::string TCmdOption<T>::do_get_value_as_string() const
{
        return __dispatch_opt<T>::get_as_string(_M_value);
}


//
// Implementation of the dictionary option
//

template <typename T>
TCmdDictOption<T>::TCmdDictOption( T& val, const TDictMap<T>& map, char short_opt, const char *long_opt, const char *long_help,
                                   const char *short_help, bool required ) :
                CCmdOptionValue( short_opt, long_opt, long_help, short_help, required ),
                _M_value( val ),
_M_map( map ) {}

template <typename T>
bool TCmdDictOption<T>::do_set_value_really( const char *svalue )
{
        _M_value = _M_map.get_value( svalue );
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
        os << "=" << _M_map.get_name( _M_value );
}

template <typename T>
const std::string TCmdDictOption<T>::do_get_value_as_string() const
{
        return _M_map.get_name( _M_value );
}

template <typename T>
void TCmdDictOption<T>::do_get_long_help_really( std::ostream& os ) const
{
        const std::set<std::string> names = _M_map.get_name_set();
        if ( names.size() > 0 ) {
                os << "\n(" ;
                std::set<std::string>::const_iterator i = names.begin();
                os << *i;
                ++i;
                while ( i != names.end() )
                        os << '|' << *i++;
                os << ")";
        }
}

NS_MIA_END

#endif
