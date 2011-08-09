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

#include "vista4mia.hh"

#include <stdexcept>


using namespace std;
using namespace boost;


NS_MIA_BEGIN
void copy_attr_list(VAttrList target, const CAttributeMap& attributes);




template <>
void add_attribute(CAttributeMap& attributes, const string& name, VString value)
{
	cvdebug() << "add attribute " << name << " of type 'string' and value '" << value << "'\n";
	attributes[name] = CStringAttrTranslatorMap::instance().to_attr(name, value);
}

template <>
void add_attribute(CAttributeMap& attributes, const string& name, VBit value)
{
	cvdebug() << "add attribute " << name << " of type " << typeid(VBit).name() << " and value '" << value << "'\n";
	attributes[name] = PAttribute(new TAttribute<bool>(value));
}



VISTA4MIA_EXPORT void copy_attr_list(CAttributeMap& attributes, const VAttrList in_list)
{
	VAttrListPosn pos;
	VFirstAttr(in_list, &pos);
	while (VAttrExists(&pos)){
		std::string name(VGetAttrName(&pos));
		cvdebug() << "got " << name << " " << VGetAttrRepn(&pos) << "\n";
		switch (VGetAttrRepn(&pos)) {
		case VStringRepn:{
			VString s;
			VGetAttrValue(&pos,NULL,VStringRepn,&s);
			add_attribute(attributes, name, s);
		}break;
		case VFloatRepn:{
			float f;
			VGetAttrValue(&pos,NULL,VFloatRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VDoubleRepn:{
			double f;
			VGetAttrValue(&pos,NULL,VDoubleRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VShortRepn:{
			short f;
			VGetAttrValue(&pos,NULL,VShortRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VLongRepn:{
			long f;
			VGetAttrValue(&pos,NULL,VLongRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VUByteRepn:{
			unsigned char f;
			VGetAttrValue(&pos,NULL,VUByteRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VSByteRepn:{
			signed char f;
			VGetAttrValue(&pos,NULL,VSByteRepn,&f);
			add_attribute(attributes, name, f);
		}break;
		case VBitRepn:{
			VBit b;
			VGetAttrValue(&pos,NULL,VBitRepn,&b);
			add_attribute(attributes, name, b);
		}break;
		default:
			cvwarn() << "Not yet implemented\n";
		}
		VNextAttr(&pos);
	}
}


VISTA4MIA_EXPORT void copy_attr_list(VAttrList target, const CAttributeMap& attributes)
{
	for (CAttributeMap::const_iterator i = attributes.begin();
	     i != attributes.end(); ++i) {
		VSetAttr(target, i->first.c_str(), NULL, VStringRepn, i->second->as_string().c_str());
	}
}


CVAttrList::CVAttrList(VAttrList list):m_list(list)
{
}

CVAttrList::~CVAttrList()
{
	VDestroyAttrList(m_list);
}

CVAttrList::operator VAttrList()
{
	return m_list; 
}

VAttrList CVAttrList::operator ->()
{
	return m_list; 
}

bool CVAttrList::operator !() const
{
	return m_list == NULL; 
}

NS_MIA_END
