/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_parameters_hh
#define mia_core_parameters_hh

#include <string>
#include <map>
#include <ostream>
#include <sstream>
#include <mia/core/dictmap.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/handlerbase.hh>
#include <mia/core/factory_trait.hh>


namespace xmlpp {
	class Element; 
}

NS_MIA_BEGIN

/**
   \ingroup cmdline

   \brief The base class for parameters used in complex options 

   The base class for parameters that might be set based on strings.
   The main use-case is to provide parameters to plug-ins.
*/
class EXPORT_CORE CParameter {
public:
	/**
	   Base parameter type constructor
	   \param type a type description string that will show up in the help
	   \param required set to true if the parameter has to be set by the user
	   \param descr a short description of the parameter
	*/
	CParameter(const char type[], bool required, const char *descr);

	/**
	   ensure virtual destructor and avoid warnings because we have a virtual function
	*/
	virtual ~CParameter();

	/**
	   \returns the type string of the parameter
q	*/
	const char *type() const;
	/**
	   \returns the description string of the parameter
	*/
	void descr(std::ostream& os) const;


	/**
	   Get the curent parameter value as string 
	   \returns the current parameter value 
	 */
	std::string get_value_as_string() const;

	/**
	   Write the current value plus information to a stream, 
	   \param os output stream 
	 */
	void value(std::ostream& os) const;

	/**
	   \returns wheather the required flag is (still) set
	*/
	bool required_set() const;

	/**
	   set the parameter based on the string value
	*/
	bool set(const std::string& str_value);

	/// @returns the help description of the parameter 
	const char *get_descr() const;

	/**
	   clear the required flag and reset to default value
	 */
	void reset(); 

	/**
	   Interface to add the plug-in handler this parameter calls to translate the input string  
	   Only for CFactoryParameter this actually does something 
	   \param[in,out] handler_map the map to store then pointers to handlers used by this parameter 
	 */
	void add_dependend_handler(HandlerHelpMap& handler_map) const; 

	/// \returns the default value of this parameter as a string 
	std::string get_default_value() const; 

	/**
	   Add the help for this parameter to a given XML tree
	   \param root the root node to add the help entry to. 
	 */
	void get_help_xml(xmlpp::Element& root) const;


	/**
	   This command is run after the parsing has been done
	   in most cases it will do nothing, only for factory parameters 
	   and the like it must set the final parameter value from the init string
	*/
	virtual void post_set(); 
	
protected:

	/** the actual (abstract) function to write the description to a stream
	    that needs to be overwritten
	    \param os
	*/
	virtual void do_descr(std::ostream& os) const = 0;

	/// create an error message by using the given value that raises the error 
	const std::string errmsg(const std::string& err_value) const;
private:
	/** the actual (abstract) function to set the parameter that needs to be overwritten
	    \param str_value the parameter value as string
	*/
	virtual void do_add_dependend_handler(HandlerHelpMap& handler_map) const;
	virtual bool do_set(const std::string& str_value) = 0;
	virtual void do_reset() = 0;
	virtual std::string do_get_default_value() const = 0;
	virtual std::string do_get_value_as_string() const = 0;
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	bool m_required;
	bool m_is_required; 
	const char *m_type;
	const char *m_descr;
};


/**
   \ingroup cmdline
   \brief Generic type of a complex paramter 

   The (templated) typed parameter. There needs to be defined an
   \a operator  &lt;&lt; (istream& is, T& x) for the \a do_set method to work.
*/

template <typename T>
class EXPORT_CORE CTParameter : public CParameter {

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param required set to \a true if the parameter has to be set by the user
	   \param descr a description of the parameter
	 */
	CTParameter(T& value, bool required, const char *descr);

protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual void adjust(T& value);
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const;
	T& m_value;
	const T m_default_value; 
};

/**
   \ingroup cmdline
   \brief Scalar parameter with an expected value range 

   A scalar parameter that supports a bracketing range. If the user tries to set the parameter
   to a value outside the range, the set method will throw an \a invalid_argument exception
*/

template <typename T>
class EXPORT_CORE TRangeParameter : public CTParameter<T> {

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param min minimum of the parameter value range
	   \param max maximum of the parameter value range
	   \param required set to \a true if the parameter has to be set by the user
	   \param descr a description of the parameter
	 */
	TRangeParameter(T& value, T min, T max, bool required, const char *descr);
protected:
	/**
	   the implementation of the description-function
	 */
	void do_descr(std::ostream& os) const;
private:
	virtual void adjust(T& value);
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	T m_min;
	T m_max;

};


/**
   \ingroup cmdline
   \brief Dictionary paramater

   The (templated) parameter that takes its value froma restricted Dictionary.
   \tparam the enumerate that is used by the dictionary 
*/

template <typename T>
class CDictParameter : public CParameter{

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param dict dictionary for parameter translation
	   \param descr a description of the parameter
	   \param required set if this parameter must be set by the end user 
	 */
	CDictParameter(T& value, const TDictMap<T> dict, const char *descr, bool required = false);
protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const;
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	T& m_value;
	T m_default_value; 
	const TDictMap<T> m_dict;

};


/**
   \ingroup cmdline
   \brief A parameter that get's initialized by a factory to a shared or unique pointer 

   This parameter type is used for parameters that are created by a factory 
   that uses a plug-in handler for instance creation. 
   \tparam F the plugin handler type used to create the parameter value 
*/
template <typename F>
class TFactoryParameter : public CParameter{

public:
	/** Constructor if the parameter tales a shared pointer 
	    The constructor should take an empty F::ProductPtr and add the default value as initializer string init
	    in order to avoid calling the plug-in hanlder for creation before the true desired value of the parameter is known. 
	    
	    \param value reference to the shared pointer parameter handled by this parameter object
	    \param init an init string used as the default parameter to create the value 
	    \param required set to true when the parameter is required 
	    \param descr a description of the parameter
	    \remark Don't call this constructor directly and use one of the make_param variants. It automatically takes care 
	    of the type instanciation and pointer variant handiong. 
	 */
	TFactoryParameter(typename F::ProductPtr& value, const std::string& init, bool required, const char *descr);

	/** Constructor if the parameter tales a unique pointer 
	    The constructor should take an empty F::ProductPtr and add the default value as initializer string init
	    in order to avoid calling the plug-in hanlder for creation before the true desired value of the parameter is known. 
	    \param value reference to the to the unique pointer parameter handled by this parameter object
	    \param init an init string used as the default parameter to create the value 
	    \param required set to true when the parameter is required 
	    \param descr a description of the parameter

	    \remark Don't call this constructor directly and use one of the make_param variants. It automatically takes care 
	    of the type instanciation and pointer variant handiong. 
	 */
	TFactoryParameter(typename F::UniqueProduct& value, const std::string& init, bool required, const char *descr);
private:
	virtual void do_descr(std::ostream& os) const;
	virtual void do_add_dependend_handler(HandlerHelpMap& handler_map)const; 
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const;
	virtual void do_get_help_xml(xmlpp::Element& self) const;

	typename F::ProductPtr dummy_shared_value; 
	typename F::UniqueProduct dummy_unique_value; 

	typename F::ProductPtr& m_shared_value;
	typename F::UniqueProduct& m_unique_value;

	virtual void post_set(); 
	
	std::string m_string_value; 
	std::string m_default_value; 
	bool m_unique; 

	
};



/**
   \ingroup cmdline
   \brief A parameter that can only assume values out of a limited set 

   This parameter type is used for parameters that that can only assume values out of a 
   limited set of values. Other than the  CDictParameter this parameter may handle any type 
   T that can be streamed.  
   \tparam T the value type of the parameter 
*/

template <typename T>
class CSetParameter : public CParameter{

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param valid_set dictionary for parameter translation
	   \param descr a description of the parameter
	   \param required boolean to indicate whether the user must set the parameter 
	 */
	CSetParameter(T& value, const std::set<T>& valid_set, const char *descr, bool required = false);
protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const;
	void do_get_help_xml(xmlpp::Element& self) const; 
	T& m_value;
	T m_default_value; 
	const std::set<T> m_valid_set;

};

/**
   \ingroup cmdline
   \brief A parameter that can assume any value of the given value type 

   This parameter type is used for parameters that are only restricted by the value type
   T that must be streamable.
   \tparam T type of the parameter value 
*/

template <typename T>
class TParameter : public CParameter{

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param required set to \a true if the parameter has to be set by the user
	   \param descr a description of the parameter
	 */
	TParameter(T& value, bool required, const char *descr);
protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual void do_reset();
	virtual bool do_set(const std::string& str_value);
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const; 

	T& m_value;
	T m_default_value; 
};


/// an string parameter
class EXPORT_CORE CStringParameter: public  CParameter {
public: 
	CStringParameter(std::string& value, bool required, const char *descr, 
			 const CPluginHandlerBase *plugin_hint = NULL); 

private: 
	virtual void do_reset();
	virtual bool do_set(const std::string& str_value);
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const; 

	virtual void do_descr(std::ostream& os) const;
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	virtual void do_add_dependend_handler(HandlerHelpMap& handler_map)const; 


	std::string& m_value;
	std::string m_default_value; 
	const CPluginHandlerBase *m_plugin_hint; 
}; 


/// an integer parameter (with range)
typedef TRangeParameter<int> CIntParameter;
/// an unsigned integer parameter (with range)
typedef TRangeParameter<unsigned int> CUIntParameter;
/// a float parameter (with range)
typedef TRangeParameter<float> CFloatParameter;
/// a double parameter (with range)
typedef TRangeParameter<double> CDoubleParameter;
/// boolean parameter
typedef CTParameter<bool> CBoolParameter;

/**    
      \ingroup cmdline
      \brief create a factory parameter that initializes to a std::shared_ptr
      
      Creates a TFactoryParameter accurding to the given parameters. The advantage over calling the 
      TactoryParameter cunstructor is, that type deduction is done automatically. 
      \tparam the type of the factory created parameter value 
      \param value the shared_ptr value to be set through this parameter. Best is to pass an empty shared pointer and 
      leave the initialization to the parameter handling 
      \param init the default initialization string for the parameter, pass "" if there is no default. 
      \param required set to true if the user must set this parameter 
      \param descr a help description of the parameter 
 */


template <typename T> 
CParameter *make_param(std::shared_ptr<T>& value, const std::string& init,  bool required, const char *descr) 
{                       
	typedef typename FactoryTrait<T>::type F;  
	return new TFactoryParameter<F>(value, init, required, descr);
	
}

/**    
      \ingroup cmdline
      \brief create a factory parameter that initializes to a std::unique_ptr
      
      Creates a TFactoryParameter accurding to the given parameters. The advantage over calling the 
      TactoryParameter cunstructor is that type deduction is done automatically. 
      \tparam the type of the factory created parameter value 
      \param value the unique_ptr value to be set through this parameter. Best is to pass an empty unique pointer and 
      leave the initialization to the parameter handling 
      \param init the default initialization string for the parameter, pass "" if there is no default. 
      \param required set to true if the user must set this parameter 
      \param descr a help description of the parameter 
*/

template <typename T> 
CParameter *make_param(std::unique_ptr<T>& value, const std::string& init,  bool required, const char *descr) 
{                       
	typedef typename FactoryTrait<T>::type F;  
	return new TFactoryParameter<F>(value, init, required, descr);
	
}


template <typename T> 
CParameter *make_param(T& value, bool required, const char *descr) 
{                       
	return new TParameter<T>(value, required, descr);
}



//// implementations 

/** @cond INTERNAL 
 */

template <typename T> 
struct __dispatch_param_translate {
	static std::string apply(T x)  {
		std::ostringstream s; 
		s << x; 
		return s.str(); 
	}
}; 

template <> 
struct __dispatch_param_translate<std::string> {
	static std::string apply(const std::string& x)  {
		return x; 
	}
}; 

template <> 
struct __dispatch_param_translate<const char *> {
	static std::string apply(const char * x)  {
		return std::string(x); 
	}
}; 

/// @endcond 

template <typename T>
CDictParameter<T>::CDictParameter(T& value, const TDictMap<T> dict, const char *descr, bool required):
	CParameter("dict", required, descr),
	m_value(value),
	m_default_value(value),
	m_dict(dict)
{
}

template <typename T>
void CDictParameter<T>::do_descr(std::ostream& os) const
{
	for (auto i = m_dict.get_help_begin(); i != m_dict.get_help_end(); ++i) {
		os << "\n  " << i->second.first << ": " << i->second.second; 
	}
}

template <typename T>
void CDictParameter<T>::do_get_help_xml(xmlpp::Element& self) const
{
	TRACE_FUNCTION; 
	auto dict = self.add_child("dict"); 
	for (auto i = m_dict.get_help_begin(); i != m_dict.get_help_end(); ++i) {
		auto v = dict->add_child("value"); 
		v->set_attribute("name", i->second.first);
		v->set_child_text(i->second.second); 
	}
}

template <typename T>
bool CDictParameter<T>::do_set(const std::string& str_value)
{
	m_value = m_dict.get_value(str_value.c_str());
	return true;
}

template <typename T>
void CDictParameter<T>::do_reset()
{
	m_value = m_default_value;
}

template <typename T>
std::string CDictParameter<T>::do_get_default_value() const
{
	return m_dict.get_name(m_default_value); 
}

template <typename T>
std::string CDictParameter<T>::do_get_value_as_string() const
{
	return m_dict.get_name(m_value); 
}

template <typename F>
TFactoryParameter<F>::TFactoryParameter(typename F::ProductPtr& value,
					const std::string& init, bool required, const char *descr):
	CParameter("factory", required, descr),
	m_shared_value(value),
	m_unique_value(dummy_unique_value),
	m_string_value(init), 
	m_default_value(init), 
	m_unique(false)
{
}

template <typename F>
TFactoryParameter<F>::TFactoryParameter(typename F::UniqueProduct& value, const std::string& init, bool required, const char *descr):
	CParameter("factory", required, descr),
	m_shared_value(dummy_shared_value),
	m_unique_value(value),
	m_string_value(init), 
	m_default_value(init), 
	m_unique(true) 
{
}

	

template <typename T>
void TFactoryParameter<T>::do_descr(std::ostream& os) const
{
	os << "For a list of available plug-ins see run 'mia-plugin-help " 
	   << T::instance().get_descriptor() << "'"; 
}

template <typename T>
void TFactoryParameter<T>::do_get_help_xml(xmlpp::Element& self) const
{
	auto dict = self.add_child("factory"); 
	dict->set_attribute("name", T::instance().get_descriptor());
}

template <typename T>
bool TFactoryParameter<T>::do_set(const std::string& str_value)
{
	m_string_value = str_value; 
	return true;
}

template <typename T>
void TFactoryParameter<T>::post_set()
{
	if (!m_string_value.empty()) {
		if (m_unique)
			m_unique_value = T::instance().produce_unique(m_string_value);
		else
			m_shared_value = T::instance().produce(m_string_value);
	}
}

template <typename T>
void TFactoryParameter<T>::do_reset()
{
	m_string_value = m_default_value;
}

template <typename T>
void TFactoryParameter<T>::do_add_dependend_handler(HandlerHelpMap& handler_map)const
{
	// add recursively all dependent handlers 
	if (handler_map.find(T::instance().get_descriptor()) ==  handler_map.end()){
		handler_map[T::instance().get_descriptor()] = &T::instance(); 
		for (auto i = T::instance().begin(); i != T::instance().end(); ++i) 
			i->second->add_dependend_handlers(handler_map); 
	}
}

template <typename T>
std::string TFactoryParameter<T>::do_get_default_value() const
{
	return m_default_value; 
}

template <typename T>
std::string TFactoryParameter<T>::do_get_value_as_string() const
{
	if (m_unique && m_unique_value) 
		return m_unique_value->get_init_string(); 
	if (!m_unique && m_shared_value) 
		return m_shared_value->get_init_string(); 
	return m_string_value; 
}

template <typename T>
CSetParameter<T>::CSetParameter(T& value, const std::set<T>& valid_set, const char *descr, bool required):
	CParameter("set", required, descr),
	m_value(value),
	m_default_value(value),
	m_valid_set(valid_set)
{
	if (m_valid_set.empty())
		throw std::invalid_argument("CSetParameter initialized with empty set");
}


template <typename T>
std::string CSetParameter<T>::do_get_default_value() const
{
	return __dispatch_param_translate<T>::apply(m_default_value); 
}

template <typename T>
std::string CSetParameter<T>::do_get_value_as_string() const
{
	return __dispatch_param_translate<T>::apply(m_value);
}

template <typename T>
void CSetParameter<T>::do_descr(std::ostream& os) const
{
	auto i = m_valid_set.begin();
	auto e = m_valid_set.end();

	assert ( i != e );

	os << "  Supported values are (" << *i;
	++i;

	while (i != e)
	       os << '|' << *i++;
	os << ')';
}

template <typename T>
void CSetParameter<T>::do_get_help_xml(xmlpp::Element& self) const
{
	auto set = self.add_child("set"); 
	for (auto i = m_valid_set.begin(); i != m_valid_set.end(); ++i) {
		auto v = set->add_child("value"); 
		v->set_attribute("name", __dispatch_param_translate<T>::apply(*i));   
	}
}

template <typename T>
void CSetParameter<T>::do_reset()
{
	m_value = m_default_value;
}

template <typename T>
bool CSetParameter<T>::do_set(const std::string& str_value)
{
	std::stringstream s(str_value);
	T val;
	s >> val;
	if (s.fail() ||  m_valid_set.find(val) == m_valid_set.end()) {
		throw std::invalid_argument(errmsg(str_value));
	}
	m_value = val;
	return true;
}



template <typename T>
TParameter<T>::TParameter(T& value, bool required, const char *descr):
	CParameter("streamable",  required, descr),
	m_value(value),
	m_default_value(value)
{
}



template <typename T>
void TParameter<T>::do_descr(std::ostream& os) const
{
	os << m_value;
}

template <typename T>
bool TParameter<T>::do_set(const std::string& str_value)
{
	std::stringstream s(str_value);
	s >> m_value;
	if (s.fail())
		throw std::invalid_argument(errmsg(str_value));
	return true;
}

template <typename T>
void TParameter<T>::do_reset()
{
	m_value = m_default_value;
}

template <typename T>
std::string TParameter<T>::do_get_default_value() const
{
	std::ostringstream s; 
	s << m_default_value; 
	auto str = s.str(); 
	if (str.find(',') != std::string::npos) {
		std::ostringstream s2; 
		s2 << '[' << str << ']'; 
		str =  s2.str(); 
	}
	return str; 
}

template <typename T>
std::string TParameter<T>::do_get_value_as_string() const
{
	return __dispatch_param_translate<T>::apply(m_value);	
}

NS_MIA_END

#endif
