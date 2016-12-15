/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_core_cmdparamlineparser_hh
#define mia_core_cmdparamlineparser_hh

#include <mia/core/cmdoption.hh>
#include <mia/core/typedescr.hh>
#include <mia/core/paramoption.hh>
#include <mia/core/handlerbase.hh>

NS_MIA_BEGIN


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
	virtual void do_get_long_help_xml(std::ostream& os, CXMLElement& parent, HandlerHelpMap& handler_map) const; 
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	T& m_value;
};



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
    If this option is repeated on the command line then the values are accumulated 
    in the value vector. 

*/
template <typename T>
class TRepeatableCmdOption: public  CCmdOption{

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
	TRepeatableCmdOption(std::vector<T>& val, char short_opt, const char *long_opt, const char *long_help,
                   const char *short_help, CCmdOptionFlags flags = CCmdOptionFlags::none);

private:
	virtual void do_get_long_help_xml(std::ostream& os, CXMLElement& parent, HandlerHelpMap& handler_map) const; 
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;
	virtual const std::string do_get_value_as_string() const;
	std::vector<T>& m_value;
};




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
void TCmdOption<T>::do_get_long_help_xml(std::ostream& os, CXMLElement& parent, 
					 HandlerHelpMap& /*handler_map*/) const
{
	do_get_long_help(os);
	xmlhelp_set_attribute(parent, "type", __type_descr<T>::value);
}

template <typename T>
const std::string TCmdOption<T>::do_get_value_as_string() const
{
        return __dispatch_opt<T>::get_as_string(m_value);
}


template <typename T>
TRepeatableCmdOption<T>::TRepeatableCmdOption(std::vector<T>& val, char short_opt, const char *long_opt,
                                              const char *long_help,
                                              const char *short_help,
                                              CCmdOptionFlags flags):
        CCmdOption(short_opt, long_opt, long_help, short_help, flags),
        m_value(val)
{
        __dispatch_opt<std::vector<T>>::init(m_value);
}

template <typename T>
void TRepeatableCmdOption<T>::do_get_long_help_xml(std::ostream& os, CXMLElement& parent,
                                                   HandlerHelpMap& MIA_PARAM_UNUSED(handler_map)) const
{
	do_get_long_help(os);
	xmlhelp_set_attribute(parent, "type", __type_descr<T>::value);
	xmlhelp_set_attribute(parent, "repeatable", "1");
}

template <typename T>
bool TRepeatableCmdOption<T>::do_set_value(const char *str_value)
{
        T value; 
        bool good = __dispatch_opt<T>::apply(str_value, value);
	if (good) {
		m_value.push_back(value);
	}
	return good; 
	

}

template <typename T>
size_t TRepeatableCmdOption<T>::do_get_needed_args() const
{
        return 1;
}

template <typename T>
void TRepeatableCmdOption<T>::do_write_value(std::ostream& os) const
{
        __dispatch_opt<std::vector<T>>::apply( os, m_value, is_required());
}

template <typename T>
const std::string TRepeatableCmdOption<T>::do_get_value_as_string() const
{
        return __dispatch_opt<std::vector<T>>::get_as_string(m_value);
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
   \brief Create a repeatable option to set a vector of values
   
   The option can be given more than one time. 
   The values are accumulated in the parameter vector. 
   \tparam T type of the value hold by the vector
   \param[in,out] value at input: if not empty, number of expected values and their defaults, at output: the values given on the command line
   \param long_opt long option name (must not be NULL)
   \param short_opt short option name (or 0)
   \param help help string (must not be NULL)
   \param flags option flags indicating whether the option is required 
   \remark be aware that localization of the decimal separator of floating point values is not used, 
   it is always the full stop ".". 
*/

template <typename T>
PCmdOption make_repeatable_opt(std::vector<T>& value, const char *long_opt, char short_opt, 
			       const char *help, CCmdOptionFlags flags = CCmdOptionFlags::none)
 {
	return PCmdOption(new TRepeatableCmdOption<T>(value, short_opt, long_opt, help, 
								    long_opt, flags ));
}

NS_MIA_END

#endif 
