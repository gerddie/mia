/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <stdexcept>
#include <mia/core/attributes.hh>
#include <mia/core/msgstream.hh>

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
	_M_attr(new CAttributeMap())
{
}

CAttributedData::CAttributedData(PAttributeMap attr):
	_M_attr(attr)
{
}

CAttributedData::CAttributedData(const CAttributedData& org):
	_M_attr(org._M_attr)
{
}

CAttributedData& CAttributedData::operator =(const CAttributedData& org)
{
	if (this != &org)
		_M_attr = org._M_attr;
	return *this;
}


const PAttribute CAttributedData::get_attribute(const std::string& name) const
{
	CAttributeMap::const_iterator i = _M_attr->find(name);

	if ( i != _M_attr->end() )
	     return i->second;
	return PAttribute();
}


PAttributeMap CAttributedData::get_attribute_list()
{
	if ( !_M_attr.unique() )
		_M_attr = PAttributeMap(new CAttributeMap(*_M_attr));
	return _M_attr;
}

const PAttributeMap CAttributedData::get_attribute_list() const
{
	return _M_attr;
}

bool CAttributedData::has_attribute(const std::string& name)const
{
	CAttributeMap::const_iterator i = _M_attr->find(name);
	return (i != _M_attr->end());
}

void CAttributedData::set_attribute(const std::string& name, PAttribute attr)
{
	if ( !_M_attr.unique() )
		_M_attr = PAttributeMap(new CAttributeMap(*_M_attr));
	(*_M_attr)[name] = attr;
}

std::ostream& operator << (std::ostream& os, const CAttributeMap& data)
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

	if ( !_M_attr.unique() )
		_M_attr = PAttributeMap(new CAttributeMap(*_M_attr));
	_M_attr->erase(key);
}

void CAttributedData::set_attribute(const std::string& name, const std::string& value)
{
	PAttribute attr =  CStringAttrTranslatorMap::instance().to_attr(name, value);
	cvdebug() << "CAttributedData::set_attribute: set '"  << name
		  << "' to '" << value << "' of type '"
		  << attr->typedescr() << "'\n";

	set_attribute(name, attr);
}

const string CAttributedData::get_attribute_as_string(const std::string& name)const
{
	CAttributeMap::const_iterator i = _M_attr->find(name);
	if ( i != _M_attr->end() )
		return i->second->as_string();
	else
		return "";
}

CStringAttrTranslatorMap::CStringAttrTranslatorMap()
{
}


void CStringAttrTranslatorMap::add(const string& key, const CAttrTranslator* const t)
{
	CMap::const_iterator k = _M_translators.find(key);
	if ( k != _M_translators.end()) {
		if ( typeid(*t) != typeid(*k->second))
			throw invalid_argument(string("translator with key '") + key + ("' already defined otherwise"));
		else
			return;
	}
	cvdebug() << "add translator type '" << typeid(*t).name() << "' for '" << key << "'\n";
	_M_translators.insert(pair<string, const CAttrTranslator* const>(key,t));
}

PAttribute CStringAttrTranslatorMap::to_attr(const string& key, const string& value) const
{
	map<string, const CAttrTranslator * const>::const_iterator i = _M_translators.find(key);
	if ( i != _M_translators.end())
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

void CAttrTranslator::do_register(const std::string& key)
{
	CStringAttrTranslatorMap::instance().add(key, this);
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
			cvdebug() << "Attribute '"<< ai->first << "' has a different value\n";
			return false;
		}

	}
	return true;
}

bool operator == (const CAttributedData& a, const CAttributedData& b)
{
	return  *a._M_attr == *b._M_attr;
}

template <>
void add_attribute(CAttributeMap& attributes, const std::string& name, const char * value)
{
	cvdebug() << "add attribute " << name << " of type 'const char*' and value '" << value << "'\n";
	attributes[name] = PAttribute(new TAttribute<string>(value));
}


template class EXPORT_CORE  TTranslator<double>;
template class EXPORT_CORE  TTranslator<std::vector<double> >;

template class EXPORT_CORE  TTranslator<float>;
template class EXPORT_CORE  TTranslator<std::vector<float> >;

#ifdef HAVE_INT64
template class EXPORT_CORE  TTranslator<mia_uint64>;
template class EXPORT_CORE  TTranslator<std::vector<mia_uint64> >;

template class EXPORT_CORE  TTranslator<mia_int64>;
template class EXPORT_CORE  TTranslator<std::vector<mia_int64> >;
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
