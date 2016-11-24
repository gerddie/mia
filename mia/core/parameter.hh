/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Pub
lic License as published by
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
#include <istream>
#include <sstream>
#include <memory>
#include <mia/core/flags.hh>
#include <mia/core/dictmap.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/handlerbase.hh>
#include <mia/core/factory_trait.hh>
#include <mia/core/cmdoptionflags.hh>

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
	void get_help_xml(CXMLElement& root) const;


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
	virtual void do_get_help_xml(CXMLElement& self) const;
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



/**
   \ingroup cmdline
   \brief Scalar parameter with a expected value boundaries 

   A scalar parameter that supports specifying boundaries. These boundaries can be one-sided 
   or on both sides, and the boundaries can be included in the range or not. 
   If the user tries to set the parameter to a value outside the range, 
   the set method will throw an \a invalid_argument exception
*/

enum class EParameterBounds : int {
	bf_none = 0, 
	bf_min = 1,  
	bf_min_open = 3, 
	bf_min_closed = 5, 
	bf_min_flags = 7, 
	bf_max = 0x10, 
	bf_max_open = 0x30, 
	bf_max_closed =  0x50, 
	bf_max_flags = 0x70,
	bf_closed_interval = 0x55, 
	bf_open_interval = 0x33 
	}; 

IMPLEMENT_FLAG_OPERATIONS(EParameterBounds); 


EXPORT_CORE std::ostream& operator << (std::ostream& os, EParameterBounds flags); 

template <typename T>
class EXPORT_CORE TBoundedParameter : public CTParameter<T> {

public:
	template <typename S> 
	struct boundary {
		typedef S value_type; 
	};
	
	template <typename S> 
		struct boundary<std::vector<S>> {
		typedef S value_type; 
	};

	typedef typename boundary<T>::value_type boundary_type; 

	
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param flags boundary flags 
	   \param boundaries the boundaries of the parameter. Depending on the flags 
	   it expects one or two values. If two values are given the first value is interpreted 
	   as the lower boundary, 
	   \param flags boundary flags 
	   \param required set to \a true if the parameter has to be set by the user
	   \param descr a description of the parameter
	 */
	TBoundedParameter(T& value, EParameterBounds flags, const std::vector<boundary_type>& boundaries,
			  bool required, const char *descr);
protected:
	/**
	   the implementation of the description-function
	 */
	void do_descr(std::ostream& os) const;
private:
	
	virtual void adjust(T& value);
	virtual void do_get_help_xml(CXMLElement& self) const;
	boundary_type m_min;
	boundary_type m_max;
	EParameterBounds m_flags; 
};

template <typename T>
CParameter *make_param(T& value, bool required, const char *descr)
{
	return new CTParameter<T>(value, required, descr); 
}


template <typename T, typename S>
CParameter *make_lo_param(T& value, S lower_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_open,
		{static_cast<T>(lower_bound)}, required, descr); 
}

template <typename T>
CParameter *make_positive_param(T& value, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_open, {T()}, required, descr); 
}

template <typename T, typename S>
CParameter *make_lc_param(T& value, S lower_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_closed,
		{static_cast<T>(lower_bound)}, required, descr); 
}


template <typename T>
CParameter *make_nonnegative_param(T& value, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_closed, {T()}, required, descr); 
}


template <typename T, typename S>
CParameter *make_uo_param(T& value, S upper_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_open,
		{static_cast<T>(upper_bound)}, required, descr); 
}

template <typename T, typename S>
CParameter *make_uc_param(T& value, S upper_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_closed,
		{static_cast<T>(upper_bound)}, required, descr); 
}

template <typename T, typename S1, typename S2>
CParameter *make_ci_param(T& value, S1 lower_bound, S2 upper_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_closed_interval,
		{static_cast<T>(lower_bound), static_cast<T>(upper_bound)}, required, descr); 
}

template <typename T, typename S1, typename S2>
CParameter *make_oi_param(T& value, S1 lower_bound, S2 upper_bound, bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_open_interval,
		{static_cast<T>(lower_bound), static_cast<T>(upper_bound)}, required, descr); 
}

template <typename T, typename S1, typename S2>
CParameter *make_coi_param(T& value, S1 lower_bound, S2 upper_bound,bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_min_closed | EParameterBounds::bf_max_open,
		{static_cast<T>(lower_bound), static_cast<T>(upper_bound)}, required, descr); 
}

template <typename T, typename S1, typename S2>
CParameter *make_oci_param(T& value, S1 lower_bound, S2 upper_bound,bool required, const char *descr)
{
	return new TBoundedParameter<T>(value, EParameterBounds::bf_max_closed | EParameterBounds::bf_min_open,
		{static_cast<T>(lower_bound), static_cast<T>(upper_bound)}, required, descr); 
}



/**
   \ingroup cmdline
   \brief Dictionary parameter

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
	CDictParameter(T& value, const TDictMap<T>& dict, const char *descr, bool required = false);
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
	virtual void do_get_help_xml(CXMLElement& self) const;
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
	virtual void do_get_help_xml(CXMLElement& self) const;

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
	void do_get_help_xml(CXMLElement& self) const; 
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
	CStringParameter(std::string& value, CCmdOptionFlags flags, const char *descr, 
			 const CPluginHandlerBase *plugin_hint = nullptr); 

private: 
	virtual void do_reset();
	virtual bool do_set(const std::string& str_value);
	virtual std::string do_get_default_value() const; 
	virtual std::string do_get_value_as_string() const; 

	virtual void do_descr(std::ostream& os) const;
	virtual void do_get_help_xml(CXMLElement& self) const;
	virtual void do_add_dependend_handler(HandlerHelpMap& handler_map)const; 


	std::string& m_value;
	std::string m_default_value; 
	CCmdOptionFlags m_flags; 
	const CPluginHandlerBase *m_plugin_hint; 
}; 


/// boolean parameter
typedef CTParameter<bool> CBoolParameter;


/// an unsigned short parameter (with possible boundaries)
typedef TBoundedParameter<uint16_t> CUSBoundedParameter;
/// an unsigned int parameter (with possible boundaries)
typedef TBoundedParameter<uint32_t> CUIBoundedParameter;
/// an unsigned long parameter (with possible boundaries)
typedef TBoundedParameter<uint64_t> CULBoundedParameter;

/// an signed short parameter (with possible boundaries)
typedef TBoundedParameter<int16_t> CSSBoundedParameter;
/// an signed int parameter (with possible boundaries)
typedef TBoundedParameter<int32_t>   CSIBoundedParameter;
/// an signed long parameter (with possible boundaries)
typedef TBoundedParameter<int64_t>  CSLBoundedParameter;

/// an float parameter, single accuracy (with possible boundaries)
typedef TBoundedParameter<float> CFBoundedParameter;
/// an float parameter, double accuracy (with possible boundaries)
typedef TBoundedParameter<double> CDBoundedParameter; 

/// an unsigned short parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<uint16_t>> CVUSBoundedParameter;
/// an unsigned int parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<uint32_t>> CVUIBoundedParameter;
/// an unsigned long parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<uint64_t>> CVULBoundedParameter;

/// an signed short parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<int16_t>> CVSSBoundedParameter;
/// an signed int parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<int32_t>> CVSIBoundedParameter;
/// an signed long parameter (with possible boundaries)
typedef TBoundedParameter<std::vector<int64_t>> CVSLBoundedParameter;

/// an float parameter, single accuracy (with possible boundaries)
typedef TBoundedParameter<std::vector<float>> CVFBoundedParameter;
/// an float parameter, double accuracy (with possible boundaries)
typedef TBoundedParameter<std::vector<double>> CVDBoundedParameter; 


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
CDictParameter<T>::CDictParameter(T& value, const TDictMap<T>& dict, const char *descr, bool required):
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
void CDictParameter<T>::do_get_help_xml(CXMLElement& self) const
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
void TFactoryParameter<T>::do_get_help_xml(CXMLElement& self) const
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
void CSetParameter<T>::do_get_help_xml(CXMLElement& self) const
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


extern template class EXPORT_CORE TBoundedParameter<uint16_t>;
extern template class EXPORT_CORE TBoundedParameter<uint32_t>;
extern template class EXPORT_CORE TBoundedParameter<uint64_t>;
extern template class EXPORT_CORE TBoundedParameter<int16_t>;
extern template class EXPORT_CORE TBoundedParameter<int32_t>;
extern template class EXPORT_CORE TBoundedParameter<int64_t>;
extern template class EXPORT_CORE TBoundedParameter<float>;
extern template class EXPORT_CORE TBoundedParameter<double>; 

extern template class EXPORT_CORE TBoundedParameter<std::vector<uint16_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<uint32_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<uint64_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<int16_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<int32_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<int64_t>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<float>>;
extern template class EXPORT_CORE TBoundedParameter<std::vector<double>>; 

extern template class EXPORT_CORE CTParameter<std::vector<std::string>>;

extern template class EXPORT_CORE CTParameter<std::string>;
extern template class EXPORT_CORE CTParameter<bool>;

NS_MIA_END

#endif
