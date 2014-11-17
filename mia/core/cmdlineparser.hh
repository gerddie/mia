/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_core_cmdlineparser_hh
#define mia_core_cmdlineparser_hh

//#include <miaconfig.h>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <string>
#include <iterator>
#include <mia/core/cmdoption.hh>
#include <mia/core/typedescr.hh>
#include <mia/core/paramoption.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/flagstring.hh>
#include <mia/core/handlerbase.hh>
#include <mia/core/selftestcmdoption.hh>

NS_MIA_BEGIN

/// the string defining the name of the help options 
extern EXPORT_CORE const char *g_help_optiongroup; 

/** holds the history info of a certain program call */
typedef std::map<std::string, std::string> CHistoryRecord;

/** holds the name of the program and the associated history record */
typedef std::pair<std::string, CHistoryRecord> CHistoryEntry;

/**
 \ingroup cmdline
 \brief program description entry identifiers
 
 These enumerates identify the parts of the program description
*/

enum EProgramDescriptionEntry {
	pdi_group = 0, /*<! The group the program belongs to, in the help the program will be described in this section */ 
	pdi_short = 1,  /*<! A sgort description of the program, this will be the head line in the unix man page. */
	pdi_description = 2, /*<! The full description of the program */ 
	pdi_example_descr = 3, /*<! description of the example that follows */
	pdi_example_code = 4,   /*<! The example command line without the program name */
	pdi_author = 5 /*<! Author name (if other than main MIA contributer(s) */
}; 

extern const std::map<EProgramDescriptionEntry, const char *> g_DescriptionEntryNames; 

/**
   \ingroup cmdline
   \brief the map that holds a basic program description 
   
   Map of strings to provied information about the program
   The following values should be provied: 
   (1) the program section, 
   (2) A short description of the program - this will become the whatis entry in the man page, 
   (3) A long description of the program 
   (4) A text describing an example usage 
   (5) The actual exampel call without the program name. 
   This information is used by the auto-help system to create man-pages and a cross-referenced
   help of all the programs. 
 */
typedef std::map<EProgramDescriptionEntry, const char *>  SProgramDescription; 



/** 
    \ingroup cmdline
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
                   const char *short_help, CCmdOptionFlags flags = CCmdOptionFlags::none);

private:
	virtual void do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& handler_map) const; 
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	T& m_value;
};

/** 
    \ingroup cmdline
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
                       const char *long_help, const char *short_help, 
		       CCmdOptionFlags flags  = CCmdOptionFlags::none);
private:
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual void do_get_long_help(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	int& m_value;
	const CFlagString m_map;
};


/**
   \ingroup cmdline
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
		hr_version,   /**< The version information has been requested */ 
		hr_copyright, /**< The long copyright information has been requested */
		hr_selftest   /**< The selftest was run */
	};

        /**
	   Constructor creates the options list and adds some defaut options like
	   --help, --verbose, --copyright, and --usage

	   \param description give a description of the program 
        */
	CCmdOptionList(const SProgramDescription& description); 

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

	/**
	   Add a selftest option. 

	   The selftest option runs the given self test and then exists. Additional parameters 
	   given on the command line are ignored. 
	   The option is set within the group \a Test and provides the long optionb name \a --selftest. 
	   \param [out] test_result stores the result returned by running by running the test suite 
	   \param callback the test functor that must have CSelftestCallback as a base class 
	 */
	void add_selftest(int& test_result, CSelftestCallback *callback);

        /** the work routine, can take the arguemnts straight from \a main
	    This version parses the command line and allows for additional arguments that can be 
	    read by get_remaining(). 
	    \param argc number of arguments
	    \param args array of arguments strings
	    \param additional_type will is a help string to describe the type of free parameters
	    \param additional_help If you use a plug-in handler to process the free parameters 
	    then pass the pointer to the according plug-in handler here, so that the help 
	    system can create proper documentation 
        */
	EHelpRequested parse(size_t argc, char *args[], const std::string& additional_type, 
		const CPluginHandlerBase *additional_help = NULL) 
		__attribute__((warn_unused_result));

        /** the work routine, can take the arguemnts straight from \a main
	    This version parses the command line and allows for additional arguments that can be 
	    read by get_remaining(). 
	    \param argc number of arguments
	    \param args array of arguments strings
	    \param additional_type will is a help string to describe the type of free parameters
	    \param additional_help If you use a plug-in handler to process the free 
	    parameters then pass the pointer to the according plug-in handler here, 
	    so that the help system can create proper documentation 

        */
	EHelpRequested parse(size_t argc, const char *args[], const std::string& additional_type, 
			     const CPluginHandlerBase *additional_help = NULL) 
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
	const std::vector<std::string>& get_remaining() const;

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


	/** This function sets a flag that indicates that data written 
	    to stdout is an actual result */
	void set_stdout_is_result(); 
private:
	EHelpRequested do_parse(size_t argc, const char *args[], bool has_additional, 
				const CPluginHandlerBase *additional_help) 
		__attribute__((warn_unused_result));
	int handle_shortargs(const char *arg, size_t argc, const char *args[]);
	struct CCmdOptionListData *m_impl;
};


// implementation of template classes and functions

/**
   @cond INTERNAL 
   @ingroup traits 
   @brief Dispatcher to handle the translation of strings to values 
   
   Dispatch the translation between a string and value. The standard case assumes that 
   we have exactly one value to translate and that the type T supports the stream in-and 
   output operators << and >> 
   \tparam the type to be translated 
   \remark most of this is now obsolete since for most types the TParamater class and its 
   derivatives/instanciantions are used 
*/
template <typename T>
struct __dispatch_opt {
	/**
	   Handle the standard initialization, normally do nothing
	*/
        static void init(T& /*value*/){
        }

	/**
	   Translate a string to a value by using the >> operator
	   \param[in] svalue the value as string 
	   \param[out] value 
	*/
        static bool  apply(const char *svalue, T& value) {
                std::istringstream sval(svalue);

                sval >> value;
                while (isspace(sval.peek())) {
                        char c;
                        sval >> c;
                }
                return sval.eof();
        }
	/// \returns the number of string elements are expected, in the standard case that is one 
	static size_t size(const T /*value*/) {
                return 1;
        }
	
	/**
	   Translate the value to a string for the help output by using the operator << and adding "=" at the beginning 
	   \param os the output stream 
	   \param value the value 
	 */
        static void apply(std::ostream& os, const T& value, bool /*required*/) {
                os << "=" << value << " ";
        }

	/**
	   translate the value to a string by using the operator >>
	   \param value the value 
	   \returns the string representation of the value 
	*/
        static const std::string get_as_string(const T& value) {
                std::ostringstream os;
                os << value;
                return os.str();
        }
};

/**
   @ingroup traits 
   @brief Dispatcher to handle the translation of vectors of to and from a string 
   \remark this specialization is still used, since no vector patameter type is defined with TParameter
*/
template <typename T>
struct __dispatch_opt< std::vector<T> > {
        static void init(std::vector<T>& /*value*/){

        }
        static bool  apply(const char *svalue, std::vector<T>& value) {
		std::string h(svalue);
		unsigned int n = 1; 
		for(std::string::iterator hb = h.begin(); hb != h.end(); ++hb)
			if (*hb == ',') {
				*hb = ' ';
				++n; 
			}
		

		if (!value.empty()) {
			if (n > value.size()) {
				throw create_exception<std::invalid_argument>("Expect only ", value.size(),  
									      " coma separated values, but '", 
									      svalue, "' provides ", n);
			}
		}else{
			value.resize(n); 
		}
		
                std::istringstream sval(h);
		auto i =  value.begin(); 
		while (!sval.eof()) {
			sval >> *i;
			++i; 
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
			for (auto i = value.begin(); i != value.end(); ++i) {
				if (i != value.begin())
					os << ",";
				os << *i;
			}
			os << " ";
		}
        }

        static const std::string get_as_string(const std::vector<T>& value) {
                std::ostringstream os;
		for (auto i = value.begin(); i != value.end(); ++i) {
			if (i != value.begin())
				os << ",";
			os << *i;
		}
                return os.str();
        }
};

/**
   @ingroup traits 
   @brief Dispatcher to handle the translation for boolen values 
   
   \todo This dispatcher is not really translating anything, since boolean values are flags. 
   which means, that now, since most other types are handled by TParameter, boolean command line parameters (flags) 
   should be implemented as a differently. 
*/
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


/**
   @ingroup traits 
   @brief Dispatcher to handle the translation for string values 
   
   \todo This dispatcher is not really translating anything, since strings are just copied
   which means, that now, since most other types are handled by TParameter, string command line parameters 
   should be implemented a differently.
*/
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
/// @endcond 


//
// Implementation of the standard option that holds a value
//
template <typename T>
TCmdOption<T>::TCmdOption(T& val, char short_opt, const char *long_opt, 
			  const char *long_help, const char *short_help, 
			  CCmdOptionFlags flags):
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
void TCmdOption<T>::do_write_value(std::ostream& os) const
{
        __dispatch_opt<T>::apply( os, m_value, is_required());
}

template <typename T>
void TCmdOption<T>::do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, 
					 HandlerHelpMap& /*handler_map*/) const
{
	do_get_long_help(os);
	parent.set_attribute("type", __type_descr<T>::value);
}

template <typename T>
const std::string TCmdOption<T>::do_get_value_as_string() const
{
        return __dispatch_opt<T>::get_as_string(m_value);
}

/**
   \ingroup cmdline
   \brief Create a standard option that sets a value of the give type 
   
   Create a standard option that translates a string given on the command line into a value. 
   The translation back-end is implemented with CTParameter. 
   
   \tparam T the type of thevalue, it must support to be read from (operator >>) and written (operator  << ) to a stream, 
   as the back-end uses these opterator to translate. 
   \param[in,out] value at input the default value, at output the parsed value if it was given. 
   \param short_opt short option name (or 0)
   \param long_opt long option name (must not be NULL)
   \param help long help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a boost::shared_ptr
*/
template <typename T>
PCmdOption make_opt(T& value, const char *long_opt, char short_opt, 
		    const char *help, CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	bool required = has_flag(flags, CCmdOptionFlags::required); 
	return PCmdOption(new CParamOption( short_opt, long_opt, new CTParameter<T>(value, required, help))); 
}

/**
   \ingroup cmdline
   \brief Create an option of a scalar value that is expecte to be within a given range 
   
   If the given value does not fit into the range it will be adjusted accordingly 
   \tparam T type of the value to be parsed, supported are float, double, long, int, short, unsigned long, unsigned int, unsigned short. 
   \tparam Tmin type of the given minmum of the range, a conversion to type T must exist
   \tparam Tmax type of the given maximum of the range, a conversion to type T must exist
   \param value value variable to hold the parsed option value - pass in the default value -
   \param min start of the value range the option can be set to 
   \param max end of the value range the option can be set to 
   \param short_opt short option name (or 0)
   \param long_opt long option name (must not be NULL)
   \param help long help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
*/
template <typename T, typename Tmin, typename Tmax>
PCmdOption make_opt(T& value, Tmin min, Tmax max,  const char *long_opt, char short_opt, 
		    const char *help, CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	bool required = has_flag(flags, CCmdOptionFlags::required); 
	return PCmdOption(new CParamOption( short_opt, long_opt, 
					    new TRangeParameter<T>(value, min, max, required, help)));
}

/**
   \ingroup cmdline
   \brief Create an option to set a vector of values, 
   
   The parameters on the command line will be separated by ',' and without spaces (or protected from the shell by usinh "". 
   If the vetcor comes with a pre-allocated size, then the numer of given values must correspond to this size, 
   Otherwise the size of the vector is deducted from the given command line parameter,. 
   \tparam T type of the value hold by the vector
   \param[in,out] value at input: if not empty, number of expected values and their defaults, at output: the values given on the command line
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help help string (must not be NULL)
   \param flags option flags indicating whether the option is required 
   \remark be aware that localization of the decimal separator of floating point values is not supported, 
   it is always the full stop ".". 
*/

template <typename T>
PCmdOption make_opt(std::vector<T>& value, const char *long_opt, char short_opt, 
		    const char *help, CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	return PCmdOption(new TCmdOption<std::vector<T> >(value, short_opt, long_opt, help, 
							  long_opt, flags ));
}

/**
   \ingroup cmdline
   \brief Create an option that represents a flag. 
   
   It is always assumed to be not set  if the according option is not given at the command line 
   \param[out] value the boolean that carries the result 
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help help string (must not be NULL)
   \param flags option flags
   \returns the option warped into a \a boost::shared_ptr

*/

PCmdOption make_opt(bool& value, const char *long_opt, char short_opt, const char *help, 
		    CCmdOptionFlags flags = CCmdOptionFlags::none); 

/**
   \ingroup cmdline
   \brief Create a table lookup option
   
   Create an option that uses a table to translate between the string given on the command line and 
   the actual value. 
   \tparam T the type of the value 
   \param[in,out] value variable to hold the parsed and translated option value. At entry, the 
             value must be set to a valid dictionary entry. 
   \param map the lookup table for the option
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help help string (must not be NULL)
   \returns the option warped into a \a boost::shared_ptr
*/
template <typename T>
PCmdOption make_opt(T& value, const TDictMap<T>& map, 
		    const char *long_opt, char short_opt, const char *help)
{
	return PCmdOption(new CParamOption( short_opt, long_opt, 
					    new CDictParameter<T>(value, map, help)));
}


/**
   \ingroup cmdline
   \brief Create a flag lookup option
   
   \param[in,out] value at input it holds the default value, at output, if the command line option was givem 
   this value is replaced by the parsed and translated option value, 
   \param map the lookup table for the option flags
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param short_help short help string
   \param flags add flags like whether the optionis required to be set 
   \returns the option warped into a \a boost::shared_ptr
   \remark Probably unnecessary 
 */

PCmdOption   make_opt(int& value, const CFlagString& map, const char *long_opt,
				 char short_opt, const char *long_help, 
				 const char *short_help, 
				 CCmdOptionFlags flags = CCmdOptionFlags::none);


/**
   \ingroup cmdline
   \brief Create an option to set a string 

   Create an option that holds a string 
   \param[in,out] value at input it holds the default value, at output, if the command line option was givem 
   this value is replaced by the parsed and translated option value, 
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param long_help long help string (must not be NULL)
   \param flags add flags like whether the optionis required to be set 
   \param plugin_hint if the string will later be used to create an object by using plug-in then pass 
	   a pointer to the corresponding plug-in handler to give a hint the help system about this connection.
   \returns the option warped into a \a boost::shared_ptr
 */
PCmdOption  make_opt(std::string& value, const char *long_opt, char short_opt, const char *long_help, 
		     CCmdOptionFlags flags = CCmdOptionFlags::none, const CPluginHandlerBase *plugin_hint = NULL); 



/**
   \ingroup cmdline
   \brief Create an oüption that only takes values from a pre-defined set 
   
   Create an option that can only take values from a given set.  
   \tparam T the type of the value to be set
   \param[in,out] value at input it holds the default value, at output, if the command line option was givem 
   this value is replaced by the parsed and translated option value, 
   \param valid_set the set of allowed values
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help long help string (must not be NULL)
   \param flags option flags indicating whether the option is required 
   \returns the option warped into a \a boost::shared_ptr
 */
template <typename T> 
PCmdOption  make_opt(T& value, const std::set<T>& valid_set,
		     const char *long_opt, char short_opt, 
		     const char *help, 
		     CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	bool required = has_flag(flags, CCmdOptionFlags::required);
	return PCmdOption(new CParamOption( short_opt, long_opt, 
					    new  CSetParameter<T>(value, valid_set, help, required))); 
}


/**
   \ingroup cmdline
   \brief Create a command line option that creates uses a factory to create an object based on the given description 

   Create a command line option that creates uses a factory to create the value based on the command line arguments given,  
   \tparam T the non-pointer type of the value
   \param[out] value at output, if the command line option was givem 
   this value is replaced by the parsed and translated option value. If not given but the default_values was given 
   then this default initializer is used to create the actual value. 
   If no default value was givem and the option is not given at the command line, then the value is not touched. 
   \param default_value default value if parameter is not given
   \param long_opt long option name
   \param short_opt short option char, set to 0 of none givn
   \param help the help string for thie option
   \param flags indicates whether theoption is required 
   \remark although passing in an initialized pointer and and empty default_value would act like the the initializer string 
   was used for the pointer, it is better to pass an empty shared pointer in order to avoid unnecessary initializations 
   of the pointer is later overwritten.
*/
template <typename T>
PCmdOption make_opt(typename std::shared_ptr<T>& value, const char *default_value, const char *long_opt, 
		    char short_opt,  const char *help,  
		    CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	bool required = has_flag(flags, CCmdOptionFlags::required);
	typedef typename FactoryTrait<T>::type F;  
	return PCmdOption(new CParamOption( short_opt, long_opt, 
					    new TFactoryParameter<F>(value, default_value, required, help))); 
}

/**
   \ingroup cmdline
   \brief Create a command line option that creates uses a factory to create an object based on the given description 
   
   Create a command line option that uses a TFactoryPluginHandler to create the actual value hold by a std::unique_ptr.
   \tparam T the non-pointer type of the value
   \param[out] value at output, if the command line option was given 
   this value is replaced by the parsed and translated option value. If not given but the default_values was given 
   then this default initializer is used to create the actual value. 
   \param default_value default value if parameter is not given
   \param long_opt long option name
   \param short_opt short option char, set to 0 of none givn
   \param help the help string for thie option
   \param flags option flags indicating whether the option is required 
   \remark although passing in an initialized pointer and and empty default_value would act like the the initializer string 
   was used for the pointer, it is better to pass an empty shared pointer in order to avoid unnecessary initializations 
   of the pointer is later overwritten.
*/
template <typename T>
PCmdOption make_opt(typename std::unique_ptr<T>& value, const char *default_value, const char *long_opt, 
		    char short_opt,  const char *help, CCmdOptionFlags flags = CCmdOptionFlags::none)
{
	bool required = has_flag(flags, CCmdOptionFlags::required);
	typedef typename FactoryTrait<T>::type F;  
	return PCmdOption(new CParamOption( short_opt, long_opt, 
					    new TFactoryParameter<F>(value, default_value, required, help))); 
}


NS_MIA_END

#endif
