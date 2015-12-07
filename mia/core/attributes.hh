/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_attributes_hh
#define mia_core_attributes_hh

#include <mia/core/msgstream.hh>
#include <mia/core/errormacro.hh>
#include <map>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <iomanip>
#include <boost/any.hpp>
#include <boost/ref.hpp>
#include <boost/lexical_cast.hpp>
#include <mia/core/attributetype.hh>

NS_MIA_BEGIN

/**
   \ingroup basic
   \brief The class of all attributes of data that is considered to ve meta-data. 
   
   CAttribute is the base class used for generric attributes of images and similar
   entities. This class is abstract and most likely you want to use one of the 
   provided specializations of the class TAttribute or create your own specializations.
*/
class EXPORT_CORE CAttribute {
public:
	/// virtual destructor since derived classes may define dynamic objetcs
	virtual ~CAttribute();

	/// returns the value as a atring
	std::string as_string() const;

	/** compare this attribute to another one
        \param other other attribute to compare to
        \returns true if the attributes are of the same type and hold the same value, false otherwiese
        */
	bool is_equal(const CAttribute& other) const;

	/** compare this attribute to another one for a strict ordering
	    \param other other attribute to compare to
	    \returns a result dependend on the actual implementation of the private virtual method
	    \a do_is_less

	 */
	bool is_less(const CAttribute& other) const; 

	/// \returns a descriptive name of the type
	virtual const char *typedescr() const = 0;

	/// \returns a  type is
	virtual int type_id() const = 0; 
private:
	virtual std::string do_as_string() const = 0;

	virtual bool do_is_equal(const CAttribute& other) const = 0;

	virtual bool do_is_less(const CAttribute& other) const = 0;
};


inline
std::ostream& operator << (std::ostream& os, const CAttribute& attr) {
	os << attr.as_string();
	return os;
};

inline bool operator == (const CAttribute& a, const CAttribute& b)
{
	return a.is_equal(b);
}


/// define the shared pointer wrapped attribute pointer
typedef std::shared_ptr<CAttribute > PAttribute;

/** 
   \ingroup basic

    \brief Class of an attribute that holds data of type  \a T

    This class is the templated derivative of CAttribute that provides the container
    for most attributes you will come across. The value is stored read-only.
    It implements implements the abstract methods typedescr, do_as_string, do_is_equal, do_is_less
    For this type, is_equal returns true if the \a other attribute has the same type and holds the
    same value.
    is_less returns true of either the type is equal and the value is less, or with different types
    if a string-compare strcmp between the type descriptions returns -1.
 */
template <typename T>
class EXPORT_CORE TAttribute : public CAttribute {
public:
	//! \name Constructors
        //@{
	/**
	   Construct the attribute by setting its value to
	   \param value
	 */
	TAttribute(typename ::boost::reference_wrapper<T>::type value);
	//@}


	/**
	   provide a transparent conversion to the content type
	   \remark should we really use this?
	 */
	operator T()const;

	/// \returns typeid(T).name(), and is, therefore, dependend on the compiler
	virtual const char *typedescr() const;

	virtual int type_id() const; 
protected:
	/// @returns the value of the attribute 
	const T& get_value() const;
private:
	virtual std::string do_as_string() const;
	virtual bool do_is_equal(const CAttribute& other) const;
	virtual bool do_is_less(const CAttribute& other) const;

	T m_value;
};

/**
   \ingroup basic
    
   Helper function to get the value of an attribute. Thr function throws a bad_cast exception,
   if the attribute doesn't hold a value ofthe requested type T
   \tparam T target type
   \param attr attribute to be read
 */

template <typename T>
T EXPORT_CORE get_attribute_as(const CAttribute& attr) {
	const TAttribute<T>& a = dynamic_cast<const TAttribute<T>&>(attr);
	return a;
}

/** 
    \ingroup basic 
    \brief an integer attribute
*/
typedef TAttribute<int> CIntAttribute;

/** 
    \ingroup basic 
    \brief a vector of integers attribute
*/
typedef TAttribute<std::vector<int> > CVIntAttribute;

/** 
    \ingroup basic 
    \brief a float attribute
*/
typedef TAttribute<float> CFloatAttribute;

/** 
    \ingroup basic 
    brief a vector of floats attribute
*/
typedef TAttribute<std::vector<float> > CVFloatAttribute;

/** 
    \ingroup basic 
    \brief a double attribute
*/
typedef TAttribute<double> CDoubleAttribute;

/** 
    \ingroup basic 
    \brief a vector of doubles attribute
*/
typedef TAttribute<std::vector<double> > CVDoubleAttribute;


/** 
    \ingroup basic 
    \brief a string attribute
*/
typedef TAttribute<std::string> CStringAttribute;

/** 
    \ingroup basic 
    \brief a vector of strings attribute
*/
typedef TAttribute<std::vector<std::string> > CVStringAttribute;

/** 
    \ingroup basic 
    \brief A name:attribute map
*/
typedef std::map<std::string, PAttribute> CAttributeMap;

template <> 
struct attribute_type<CAttributeMap> : public EAttributeType { 
        static const int value = 1000;
}; 


/** 
    \ingroup basic 
    \brief providing the possibility to nest attribute lists
*/
typedef TAttribute<CAttributeMap> CAttributeList;

/** 
    \ingroup basic 
    \brief another pointer-usage easy maker
*/
typedef std::shared_ptr<CAttributeMap > PAttributeMap;


/**
   \ingroup basic 
   \brief Facility to write an attribute map to a stream
   \param os output stream
   \param data map of values
   \returns a reference to the output stream
 */
EXPORT_CORE  std::ostream& operator << (std::ostream& os, const CAttributeMap& data);


/** 
    \ingroup basic 
    \brief A collection of attributes 
    
    This is the base class for all data that uses attributes. It provides all the needed functions to store 
    and retrive attributes. 
*/
class EXPORT_CORE CAttributedData {
public:

	//! \name Constructors
        //@{

	CAttributedData();
	CAttributedData(const CAttributedData& org);

	/**
	   Constructor that initialises with a certain attribute map
	   \param attr the map to be initialised with
	 */
	CAttributedData(PAttributeMap attr);

	//@}

	/// Assignemt operator 
	CAttributedData& operator =(const CAttributedData& org);

	/** \param key
	    \returns the attribute with name \a name or \a NULL (wrapped as shared pointer)
                     if the attribute with \a name is not found
	*/
	const PAttribute get_attribute(const std::string& key) const;

	/**
	   \returns the begin iterator to the attributes 
	 */
	CAttributeMap::const_iterator begin_attributes() const; 
	
	/**
	   \returns the end iterator to the attributes 
	*/
	CAttributeMap::const_iterator end_attributes() const; 

	/**
	   Sets the attribute \a name to value \a attr. If \a attr is \a NULL, then
	   the attribute is removed from the list (or not added)
	   \param key
	   \param attr
	*/
	void set_attribute(const std::string& key, PAttribute attr);


	/**
	   Insersts or overwrites the attributes given in the input range 
	   \param begin 
	   \param end
	*/
	void set_attributes(CAttributeMap::const_iterator begin, CAttributeMap::const_iterator end);

	/**
	   Set an attribute using one of the defined translators
	   \param key
	   \param value
	 */
	void set_attribute(const std::string& key, const std::string& value);


        /**
	   Set an attribute, generic version.  
	   \tparam T type of the attribute value to be set 
	   \param key
	   \param value
	 */
	template <typename T> 
	void set_attribute(const std::string& key, const T& value);

	/**
	   Set an attribute using one of the defined translators
	   \param key
	   \param value
	 */
	void set_attribute(const std::string& key, const char* value);

	
	/// returns the requested attribute as string, returns an empty string if attribute doesn't exist
	const std::string get_attribute_as_string(const std::string& key)const;


	/**
	   Look for a certain attribute and try to cast it to the output type. 
	   If the attribute is not found, a std::invalid_argument exception is thrown. 
	   If the cast fails then std::bad_cast exception will be thrown.
	   @param key the key of the attribute to look up. 
	   @returns the value of the attribute 
	*/
	template <typename T>
	const T get_attribute_as(const std::string& key)const;

	/**
	   Look for a certain attribute and try to cast it to the output type. 
	   If the attribute is not found, or the cast goes wrong use the default value
	   In the latter case a warning is written out. 
	   @param key the key of the attribute to look up. 
	   @param default_value the default value 
	   @returns the value of the attribute 
	*/
	template <typename T>
	const T get_attribute_as(const std::string& key, T default_value)const;

	/**
	   Delete the attribute with a given key from the list 
	   @param key 
	 */

	void delete_attribute(const std::string& key);

	/**
	   See if a certain attribute exists 
	   @param key
	   @returns true if attribute exists, false otherwise
	*/
	bool has_attribute(const std::string& key)const;

	/// @cond FRIENDSDOC
	friend EXPORT_CORE bool operator == (const CAttributedData& a, const CAttributedData& b);
	/// @endcond 

	void print(std::ostream& os) const  {
		os << *m_attr; 
	}
private:
	PAttributeMap m_attr;
};


inline std::ostream& operator << (std::ostream& os, const CAttributedData& data)
{
	data.print(os); 
	return os; 
}



/**
   \ingroup basic 

   \brief Compare two attribute data instances \a a  and \a b
   \returns \a true, if \a a and \a b hold the same set of attributes with the same values,
             \a false otherwise
*/

EXPORT_CORE bool operator == (const CAttributeMap& am, const CAttributeMap& bm);


/**
   \ingroup basic 
   @brief A class to translate an attribute from a string.
   
   This class is the base class to translate attributes from their typed value to a string and back. 
*/

class EXPORT_CORE CAttrTranslator {
public:
        /// The virtual destructor just ensures virtual destruction and silences a warning
        virtual ~CAttrTranslator() {};

	/**
	   \param value the string value of the object
	   \returns a shared pointer to the newly created attribute
	*/
	PAttribute from_string(const std::string& value) const;
private:
	virtual PAttribute do_from_string(const std::string& value) const = 0;
protected:
	CAttrTranslator();

	/**
	   Register this translator to handle attributes with the given key 
	   @param key 
	 */
	bool do_register(const std::string& key);
};

/**
   \ingroup basic 
    
   \brief A singelton class to translate strings to attributes based on keys.
   
   This class provides a singleton to translate strings to attributes. For the translation to take
   place for each attribute key a CAttrTranslator needs to be registered first.
*/
class EXPORT_CORE CStringAttrTranslatorMap {
public:
	/**
	   Converts the input string \a value to a PAttribute by using the translator that was registered for the
	   given \a key.
	   \param key
	   \param value
	   \returns the respective attribute
	 */
	PAttribute to_attr(const std::string& key, const std::string& value) const;

	/// \returns an instance to the translator map singleton
	static CStringAttrTranslatorMap& instance();
private:
	friend class  CAttrTranslator;
	CStringAttrTranslatorMap();
	/**
	   Add a new translator to the map. If there is already another translator registered for the given \a key, then
	   throw an \a invalid_argument exception.
	   \param key a key that is used to pick the translator
	   \param t the translator object
	 */

	bool add(const std::string& key, CAttrTranslator * t);

	typedef std::map<std::string, std::shared_ptr<CAttrTranslator>> CMap;
	CMap m_translators;
};


/**
   \ingroup basic 
   
   convenience function to set an attribute in an attribute map:
   \remark review its use
   \tparam type of the attribute value to be added
   \param attributes map to set the value in
   \param key
   \param value
 */

template <typename T>
void EXPORT_CORE add_attribute(CAttributeMap& attributes, const std::string& key, T value)
{
	cvdebug() << "add attribute " << key << " of type " << typeid(T).name() << " and value '" << value << "'\n";
	attributes[key] = PAttribute(new TAttribute<T>(value));
}

template <>
inline void EXPORT_CORE add_attribute(CAttributeMap& attributes, const std::string& key, const char *value)
{
	attributes[key] =  CStringAttrTranslatorMap::instance().to_attr(key, value);
	cvdebug() << "add_attribute '"  << key
		  << "' to '" << value << "' of type '"
		  << attributes[key]->typedescr() << "'\n";
}



/** 
   \ingroup basic 
   
   \brief Generic string vs. attribute translator singleton

   This class defines a generic translator between strings and a specific attribute type.
   All translaters are registered to a global map of type CStringAttrTranslatorMap
   that selects the conversion  based on a key. The global map is implemented as a singleton
   can be accessed via CStringAttrTranslatorMap::instance()
 */
template <typename T>
class EXPORT_CORE TTranslator: public CAttrTranslator {
public:
	/**
	   Register this translator for attributed for the given \a key. The translator is handled as a
	   singleton and is stateless.
	   Any translator type can be registered multiple times but keys must be different
	   if the target translation type is different.
	 */
	static  bool register_for(const std::string& key);
private:
	virtual PAttribute do_from_string(const std::string& value) const;
};


// template implementation

template <typename T>
TAttribute<T>::TAttribute(typename ::boost::reference_wrapper<T>::type value):
	m_value(value)
{
}

template <typename T>
TAttribute<T>::operator T() const
{
	return m_value;
}

template <typename T>
const T& TAttribute<T>::get_value() const
{
	return m_value;
}

template <typename T>
const char *TAttribute<T>::typedescr() const
{
	return typeid(T).name();
}

template <typename T>
int TAttribute<T>::type_id() const
{
	static_assert(attribute_type<T>::value != EAttributeType::attr_unknown, 
		      "You must provide a type specialization for attribute_type<T>"); 
	
	return attribute_type<T>::value; 
}

/**
   @cond INTERNAL 
   @ingroup traits 
   
   A trait to translate between a string and a value  
   \remark this should replace the parameter translation methods 
*/

template <typename T, bool is_floating>
struct __convert_to_string
{
	static std::string apply(const typename ::boost::reference_wrapper<T>::type value) {
		return boost::lexical_cast<std::string>(value);
	}
}; 

template <typename T>
struct __convert_to_string<T, true> {
	static std::string apply(const typename ::boost::reference_wrapper<T>::type value) {
		std::stringstream sval;
		sval << std::setprecision(10) << value; 
		return sval.str();
	}
}; 


template <typename T>
struct dispatch_attr_string {
	static std::string val2string(const typename ::boost::reference_wrapper<T>::type value) {
		return __convert_to_string<T, std::is_floating_point<T>::value>::apply(value);
	}
	static T string2val(const std::string& str) {
		T v;
		std::istringstream svalue(str);
		svalue >> v;
		return v;
	}
};


template <typename T>
struct dispatch_attr_string<std::vector<T> > {
	static std::string val2string(const std::vector<T>& value) {
		std::stringstream sval;
		sval << value.size();
		for (size_t i = 0; i < value.size(); ++i)
			sval << " "
			     <<  __convert_to_string<T, std::is_floating_point<T>::value>::apply(value[i]); 
	        return sval.str();
	}
	static std::vector<T> string2val(const std::string& str) {
		size_t s;
		std::istringstream svalue(str);
		std::vector<T> v; 
		svalue >> s;
		if (s > v.max_size())
			throw create_exception<std::runtime_error>("string2val: try to create a vector of size ", 
								   s, " but support only size ", v.max_size()); 
		v.resize(s);
		for (size_t i = 0; i < s; ++i)
			svalue >> v[i];
		if (svalue.fail()) {
			std::stringstream msg;
			msg << "string2val: unable to convert '" << str << "'";
			throw std::invalid_argument(msg.str());
		}
		return v;
	}
};


template <>
struct dispatch_attr_string<std::vector<bool> > {
	static std::string val2string(const std::vector<bool>& value) {
		std::stringstream sval;
		sval << value.size();
		for (size_t i = 0; i < value.size(); ++i)
			sval << " " << value[i];
		return sval.str();
	}
	static std::vector<bool> string2val(const std::string& str) {
		size_t s;
		std::istringstream svalue(str);
		svalue >> s;
		std::vector<bool> v(s);

		// Added override for coverity
		// 
		// Since s is used as the size for the new vector, a large
		// value will result in a std::bad_alloc exception. This
		// is not worse than bailing out because s is larger than
		// an abitrary set boundary that youle be cheked here.
		// 
		// coverity[tainted_scalar] 
		for (size_t i = 0; i < s; ++i) {
			bool value;
			svalue >> value;
			v[i] = value;
		}
		if (svalue.fail()) {
			std::stringstream msg;
			msg << "string2val: unable to convert '" << str << "'";
			throw std::invalid_argument(msg.str());
		}
		return v;
	}
};

template <>
struct dispatch_attr_string<unsigned char> {
	static std::string val2string(unsigned char value) {
		std::stringstream sval;
		sval << (unsigned int)value;
		return sval.str();
	}
	static unsigned char string2val(const std::string& str) {
		unsigned int v;
		std::istringstream svalue(str);
		svalue >> v;
		return (unsigned char)v;
	}
};

template <>
struct dispatch_attr_string<signed char> {
	static std::string val2string(signed char value) {
		std::stringstream sval;
		sval << (signed int)value;
		return sval.str();
	}
	static signed char string2val(const std::string& str) {
		int v;
		std::istringstream svalue(str);
		svalue >> v;
		return (signed char)v;
	}
};

template <>
struct dispatch_attr_string<std::string> {
	static std::string val2string(std::string value) {
		return value;
	}
	static std::string string2val(const std::string& str) {
		return str;
	}
};

template <>
struct dispatch_attr_string<CAttributeMap> {
	static std::string val2string(const CAttributeMap& /*value*/) {
		throw std::invalid_argument("Conversion of a CAttributeMap to a string not implemented");
	}
	static CAttributeMap string2val(const std::string& /*str*/) {
		throw std::invalid_argument("Conversion of a string to a CAttributeMap not implemented");
	}
};

/// @endcond

template <typename T> 
void CAttributedData::set_attribute(const std::string& key, const T& value)
{
	add_attribute(*m_attr, key, value); 
}


template <typename T>
std::string TAttribute<T>::do_as_string() const
{
	return dispatch_attr_string<T>::val2string(m_value);
}

template <typename T>
bool TAttribute<T>::do_is_equal(const CAttribute& other) const
{
	const TAttribute<T>* o = dynamic_cast<const TAttribute<T> *>(&other);
	if (!o) {
		cvdebug() << "TAttribute<T>::do_is_equal:Cast to "
			  << typeid(const TAttribute<T>*).name()
			  << "failed\n";
		return false;
	}
	return m_value == o->m_value;
}

template <typename T>
bool TAttribute<T>::do_is_less(const CAttribute& other) const
{
	const TAttribute<T>* o = dynamic_cast<const TAttribute<T> *>(&other);
	if (o)
		return m_value < o->m_value;

	return strcmp(typedescr(), other.typedescr()) < 0;
}

template <typename T>
const T CAttributedData::get_attribute_as(const std::string& key)const
{
	PAttribute attr = get_attribute(key);
	if (attr)
		return dynamic_cast<const TAttribute<T>&>(*attr);
	else
		throw create_exception<std::invalid_argument>("CAttributedData: no attribute '", key, "' found");
}

template <typename T>
const T CAttributedData::get_attribute_as(const std::string& key, T default_value)const
{
	PAttribute pattr = get_attribute(key);
	if (!pattr) 
		return default_value; 
	auto attr = dynamic_cast<const TAttribute<T> *>(pattr.get());
	if (!attr) {
		cvwarn() << "Attribute '" << key << "'exists but is not of the expected type, returning default\n";
		return default_value; 
	}
	return *attr; 
}


typedef TTranslator<double> CDoubleTranslator;
typedef TTranslator<std::vector<double> > CVDoubleTranslator;

typedef TTranslator<float> CFloatTranslator;
typedef TTranslator<std::vector<float> > CVFloatTranslator;

#ifdef LONG_64BIT
typedef TTranslator<unsigned long> CULTranslator;
typedef TTranslator<std::vector<unsigned long> > CVULTranslator;

typedef TTranslator<signed long> CSLTranslator;
typedef TTranslator<std::vector<signed long> > CVSLTranslator;
#endif

typedef TTranslator<unsigned int> CUITranslator;
typedef TTranslator<std::vector<unsigned int> > CVUITranslator;

typedef TTranslator<signed int>   CSITranslator;
typedef TTranslator<std::vector<signed int> > CVSITranslator;

typedef TTranslator<unsigned short> CUSTranslator;
typedef TTranslator<std::vector<unsigned short> > CVUSTranslator;

typedef TTranslator<signed short> CSSTranslator;
typedef TTranslator<std::vector<signed short> > CVSSTranslator;

typedef TTranslator<unsigned char> CUBTranslator;
typedef TTranslator<std::vector<unsigned char> > CVUBTranslator;

typedef TTranslator<signed char> CSBTranslator;
typedef TTranslator<std::vector<signed char> > CVSBTranslator;

typedef TTranslator<bool> CBitTranslator;
typedef TTranslator<std::vector<bool> > CVBitTranslator;

NS_MIA_END

#endif
