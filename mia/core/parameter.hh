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

#ifndef ph_parameters_hh
#define ph_parameters_hh

#include <string>
#include <map>
#include <ostream>
#include <sstream>
#include <mia/core/dictmap.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN

/**
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
	*/
	const char *type() const;
	/**
	   \returns the description string of the parameter
	*/
	void descr(std::ostream& os) const;

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
	virtual bool do_set(const std::string& str_value) = 0;

	bool m_required;
	const char *m_type;
	const char *m_descr;
};


/**
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
	virtual void adjust(T& value);
	T& m_value;
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
	T m_min;
	T m_max;

};

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
	T& m_value;
	const TDictMap<T> m_dict;

};

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
	T& m_value;
	const std::set<T> m_valid_set;

};


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
	T& m_value;
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
/// an integer parameter (with range)
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
CDictParameter<T>::CDictParameter(T& value, const TDictMap<T> dict, const char *descr):
	CParameter("dict", false, descr),
	m_value(value),
	m_dict(dict)
{
}

template <typename T>
void CDictParameter<T>::do_descr(std::ostream& os) const
{
	const std::set<std::string> names = m_dict.get_name_set();
	os << '(';

	std::set<std::string>::const_iterator i = names.begin();
	std::set<std::string>::const_iterator e = names.end();
	if ( i == e ) {
		throw std::invalid_argument("Dictmap without entries");
	}
	os << *i;
	++i;

	while (i != e)
	       os << '|' << *i++;
	os << ')';
}

template <typename T>
bool CDictParameter<T>::do_set(const std::string& str_value)
{
	m_value = m_dict.get_value(str_value.c_str());
	return true;
}

template <typename T>
CSetParameter<T>::CSetParameter(T& value, const std::set<T>& valid_set, const char *descr):
	CParameter("set", false, descr),
	m_value(value),
	m_valid_set(valid_set)
{
	if (m_valid_set.empty())
		throw std::invalid_argument("CSetParameter initialized with empty set");
}

template <typename T>
void CSetParameter<T>::do_descr(std::ostream& os) const
{
	typename std::set<T>::const_iterator i = m_valid_set.begin();
	typename std::set<T>::const_iterator e = m_valid_set.end();

	assert ( i != e );

	os << *i;
	++i;

	while (i != e)
	       os << '|' << *i++;
	os << ')';
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
	m_value(value)
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


NS_MIA_END

#endif
