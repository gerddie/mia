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
   \ingroup infrastructure 

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

	void get_help_xml(xmlpp::Element& root) const;
	
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
	virtual void do_reset(); 
	virtual std::string do_get_default_value() const = 0; 
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	bool m_required;
	bool m_is_required; 
	const char *m_type;
	const char *m_descr;
};


/**
   \ingroup infrastructure 
   \brief Generic type of a complex paramter 

   The (templated) typed parameter. There needs to be defined an
   \a operator  &lt;&lt; (istream& is, T& x) for the \a do_set method to work.
*/

template <typename T, const char * const type>
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
	T& m_value;
	T m_default_value; 
};

/**
   \brief Scalar parameter with an expected value range 

   A scalar parameter that supports a bracketing range. If the user tries to set the parameter
   to a value outside the range, the set method will throw an \a invalid_argument exception
*/

template <typename T, const char * const type>
class EXPORT_CORE TRangeParameter : public CTParameter<T, type> {

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
   \ingroup infrastructure 
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
	 */
	CDictParameter(T& value, const TDictMap<T> dict, const char *descr);
protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	T& m_value;
	T m_default_value; 
	const TDictMap<T> m_dict;

};


/**
   \ingroup infrastructure 
   \brief A parameter that get's initialized by a factory 

   This parameter type is used for parameters that are created by a factory 
   that uses a plug-in handler for instance creation. 
   \tparam F the plugin handler type used to create the parameter value 
*/
template <typename F>
class CFactoryParameter : public CParameter{

public:
	/** Constructor
	   \param value reference to the parameter handled by this parameter object
	   \param required set to true when the parameter is required 
	   \param descr a description of the parameter
	 */
	CFactoryParameter(typename F::ProductPtr& value, bool required, const char *descr);
private:
	virtual void do_descr(std::ostream& os) const;
	virtual void do_add_dependend_handler(HandlerHelpMap& handler_map)const; 
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	virtual void do_get_help_xml(xmlpp::Element& self) const;
	typename F::ProductPtr& m_value;
	typename F::ProductPtr m_default_value; 
};

/**
   \ingroup infrastructure 
   \brief A parameter that can only assume values out ofa limited set 

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
	 */
	CSetParameter(T& value, const std::set<T>& valid_set, const char *descr);
protected:
	/**
	   the implementation of the description-function
	 */
	virtual void do_descr(std::ostream& os) const;
private:
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	void do_get_help_xml(xmlpp::Element& self) const; 
	T& m_value;
	T m_default_value; 
	const std::set<T> m_valid_set;

};

/**
   \ingroup infrastructure 
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
	virtual bool do_set(const std::string& str_value);
	virtual void do_reset();
	virtual std::string do_get_default_value() const; 
	T& m_value;
	T m_default_value; 
};


/// template parameter string for unsigned int parameter type 
extern const char  type_str_uint[5];

/// template parameter string for int parameter type 
extern const char  type_str_int[4];

/// template parameter string for float parameter type 
extern const char  type_str_float[6];

/// template parameter string for double parameter type 
extern const char  type_str_double[7];

/// template parameter string for string parameter type 
extern const char  type_str_string[7];

/// template parameter string for boolean parameter type 
extern const char  type_str_bool[5];

/// an integer parameter (with range)
typedef TRangeParameter<int, type_str_int> CIntParameter;
/// an unsigned integer parameter (with range)
typedef TRangeParameter<unsigned int, type_str_uint> CUIntParameter;
/// a float parameter (with range)
typedef TRangeParameter<float, type_str_float> CFloatParameter;
/// a double parameter (with range)
typedef TRangeParameter<double, type_str_double> CDoubleParameter;
/// an string parameter
typedef CTParameter<std::string,type_str_string> CStringParameter;
/// boolean parameter
typedef CTParameter<bool, type_str_bool> CBoolParameter;



template <typename T> 
CParameter *make_param(std::shared_ptr<T>& value, const std::string& init,  bool required, const char *descr) 
{                       
	typedef typename FactoryTrait<T>::type F;  
	if (!init.empty()) 
		value = F::instance().produce(init.c_str()); 
	return new CFactoryParameter<F>(value, required, descr);
	
}

//// implementations 

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

template <typename T>
CDictParameter<T>::CDictParameter(T& value, const TDictMap<T> dict, const char *descr):
	CParameter("dict", false, descr),
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
CFactoryParameter<T>::CFactoryParameter(typename T::ProductPtr& value, bool required, const char *descr):
	CParameter("factory", required, descr),
	m_value(value),
	m_default_value(value)
{
}

template <typename T>
void CFactoryParameter<T>::do_descr(std::ostream& os) const
{
	os << "For a list of available plug-ins see run 'mia-plugin-help " 
	   << T::instance().get_descriptor() << "'"; 
}

template <typename T>
void CFactoryParameter<T>::do_get_help_xml(xmlpp::Element& self) const
{
	auto dict = self.add_child("factory"); 
	dict->set_attribute("name", T::instance().get_descriptor());
}

template <typename T>
bool CFactoryParameter<T>::do_set(const std::string& str_value)
{
	m_value = T::instance().produce(str_value); 
	return true;
}

template <typename T>
void CFactoryParameter<T>::do_reset()
{
	m_value = m_default_value;
}

template <typename T>
void CFactoryParameter<T>::do_add_dependend_handler(HandlerHelpMap& handler_map)const
{
	handler_map[T::instance().get_descriptor()] = &T::instance(); 
}

template <typename T>
std::string CFactoryParameter<T>::do_get_default_value() const
{
	if (m_default_value) 
		return std::string("[") + m_default_value->get_init_string() + std::string("]");
	else 
		return std::string("NULL"); 
}


template <typename T>
CSetParameter<T>::CSetParameter(T& value, const std::set<T>& valid_set, const char *descr):
	CParameter("set", false, descr),
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
	return s.str(); 
}


NS_MIA_END

#endif
