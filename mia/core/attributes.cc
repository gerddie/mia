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

#include <stdexcept>
#include <mia/core/attributes.hh>
#include <mia/core/msgstream.hh>

#include <mia/core/attributes.cxx>

NS_MIA_BEGIN
using namespace std;

CAttribute::~CAttribute()
{
}

std::string CAttribute::as_string() const
{
	return do_as_string();
}

bool CAttribute::is_equal(const CAttribute& other) const
{
	return do_is_equal(other);
}

bool CAttribute::is_less(const CAttribute& other) const
{
	return do_is_less(other);
}


CAttributedData::CAttributedData():
	m_attr(new CAttributeMap())
{
}

CAttributedData::CAttributedData(PAttributeMap attr):
	m_attr(attr)
{
}

CAttributedData::CAttributedData(const CAttributedData& org):
	m_attr(org.m_attr)
{
}

CAttributedData& CAttributedData::operator =(const CAttributedData& org)
{
	if (this != &org)
		m_attr = org.m_attr;
	return *this;
}


const PAttribute CAttributedData::get_attribute(const std::string& name) const
{
	CAttributeMap::const_iterator i = m_attr->find(name);

	if ( i != m_attr->end() )
	     return i->second;
	return PAttribute();
}

bool CAttributedData::has_attribute(const std::string& name)const
{
	CAttributeMap::const_iterator i = m_attr->find(name);
	return (i != m_attr->end());
}

void CAttributedData::set_attribute(const std::string& name, PAttribute attr)
{
	if ( !m_attr.unique() )
		m_attr = PAttributeMap(new CAttributeMap(*m_attr));
	(*m_attr)[name] = attr;
}

void CAttributedData::set_attributes(CAttributeMap::const_iterator begin, CAttributeMap::const_iterator end)
{
	while (begin != end){
		set_attribute(begin->first, begin->second); 
		++begin; 
	}
}

EXPORT_CORE std::ostream&  operator << (std::ostream& os, const CAttributeMap& data)
{
	os << "attribute map: [ \n";
	for (CAttributeMap::const_iterator i = data.begin();
	     i != data.end(); ++i) {
		os << "  ('" << i->first << "', '" << i->second->as_string() << "')\n";
	}
	os << "]\n";

	return os;
}

void CAttributedData::delete_attribute(const std::string& key)
{

	if ( !m_attr.unique() )
		m_attr = PAttributeMap(new CAttributeMap(*m_attr));
	m_attr->erase(key);
}

void CAttributedData::set_attribute(const std::string& name, const std::string& value)
{
	PAttribute attr =  CStringAttrTranslatorMap::instance().to_attr(name, value);
	cvdebug() << "CAttributedData::set_attribute: set '"  << name
		  << "' to '" << value << "' of type '"
		  << attr->typedescr() << "'\n";

	set_attribute(name, attr);
}

void CAttributedData::set_attribute(const std::string& key, const char* value)
{
	set_attribute(key, string(value)); 
}

const string CAttributedData::get_attribute_as_string(const std::string& name)const
{
	CAttributeMap::const_iterator i = m_attr->find(name);
	if ( i != m_attr->end() )
		return i->second->as_string();
	else
		return "";
}

CAttributeMap::const_iterator CAttributedData::begin_attributes() const
{
	return m_attr->begin(); 
}

CAttributeMap::const_iterator CAttributedData::end_attributes() const
{
	return m_attr->end(); 
}

CStringAttrTranslatorMap::CStringAttrTranslatorMap()
{
}


bool CStringAttrTranslatorMap::add(const string& key, CAttrTranslator* t)
{
	CMap::const_iterator k = m_translators.find(key);
	if ( k != m_translators.end()) {
		if ( typeid(*t) != typeid(*k->second))
			throw invalid_argument(string("translator with key '") + key + ("' already defined otherwise"));
		else
			return false;
	}
	cvdebug() << "add translator type '" << typeid(*t).name() << "' for '" << key << "'\n";
	m_translators.insert(make_pair(key,shared_ptr<CAttrTranslator>(t))); 
	return true;
}

PAttribute CStringAttrTranslatorMap::to_attr(const string& key, const string& value) const
{
	auto i = m_translators.find(key);
	if ( i != m_translators.end())
		return i->second->from_string(value);
	else
		return PAttribute(new TAttribute<string>(value));
}

CStringAttrTranslatorMap& CStringAttrTranslatorMap::instance()
{
	static CStringAttrTranslatorMap map;
	return map;
}

PAttribute CAttrTranslator::from_string(const std::string& value) const
{
	return do_from_string(value);
}

CAttrTranslator::CAttrTranslator()
{
}

bool CAttrTranslator::do_register(const std::string& key)
{
	return CStringAttrTranslatorMap::instance().add(key, this);
}

bool operator == (const CAttributeMap& am, const CAttributeMap& bm)
{
	if (am.size() != bm.size()) {
		cvdebug() << "CAttributeMap ==: Lists have different size:"
			  << am.size() << " vs. " << bm.size() << "\n";
		return false;
	}

	for (CAttributeMap::const_iterator ai = am.begin(); ai != am.end(); ++ai) {
		CAttributeMap::const_iterator bi = bm.find(ai->first);

		if (bi == bm.end()) {
			cvdebug() << "Attribute '" << ai->first << "' not in second list\n";
			return false;
		}

		if (!ai->second->is_equal(*bi->second)) {
			cvdebug() << "Attribute '"<< ai->first << "' has a different value: '"
				  << ai->second->as_string() << "', expect '" <<  bi->second->as_string()
				  << "'\n";
			return false;
		}

	}
	return true;
}

bool EXPORT_CORE operator == (const CAttributedData& a, const CAttributedData& b)
{
	return  *a.m_attr == *b.m_attr;
}


template class EXPORT_CORE  TTranslator<double>;
template class EXPORT_CORE  TTranslator<std::vector<double> >;

template class EXPORT_CORE  TTranslator<float>;
template class EXPORT_CORE  TTranslator<std::vector<float> >;

#ifdef LONG_64BIT
template class EXPORT_CORE  TTranslator<unsigned long>;
template class EXPORT_CORE  TTranslator<std::vector<unsigned long> >;

template class EXPORT_CORE  TTranslator<signed long>;
template class EXPORT_CORE  TTranslator<std::vector<signed long> >;
#endif

template class EXPORT_CORE  TTranslator<unsigned int>;
template class EXPORT_CORE  TTranslator<std::vector<unsigned int> >;

template class EXPORT_CORE  TTranslator<signed int>;
template class EXPORT_CORE  TTranslator<std::vector<signed int> >;

template class EXPORT_CORE  TTranslator<unsigned short>;
template class EXPORT_CORE  TTranslator<std::vector<unsigned short> >;

template class EXPORT_CORE  TTranslator<signed short>;
template class EXPORT_CORE  TTranslator<std::vector<signed short> >;

template class EXPORT_CORE  TTranslator<unsigned char>;
template class EXPORT_CORE  TTranslator<std::vector<unsigned char> >;

template class EXPORT_CORE  TTranslator<signed char>;
template class EXPORT_CORE  TTranslator<std::vector<signed char> >;

template class EXPORT_CORE  TTranslator<bool>;
template class EXPORT_CORE  TTranslator<std::vector<bool> >;




NS_MIA_END
