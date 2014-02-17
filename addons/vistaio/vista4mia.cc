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

#include "vista4mia.hh"

#include <stdexcept>


using namespace std;
using namespace boost;


NS_MIA_BEGIN

void vistaio_add_attribute(CAttributedData& attr, const string& name, VistaIOString value)
{
	cvdebug() << "add attribute " << name << " of type 'string' and value '" << value << "'\n";
	attr.set_attribute(name, CStringAttrTranslatorMap::instance().to_attr(name, value)); 
}

void vistaio_add_attribute(CAttributedData& attr, const string& name, VistaIOBit value)
{
	cvdebug() << "add attribute " << name << " of type " << typeid(VistaIOBit).name() << " and value '" << value << "'\n";
	attr.set_attribute(name, PAttribute(new TAttribute<bool>(value))); 
}



VISTA4MIA_EXPORT void copy_attr_list(CAttributedData& attributes, const VistaIOAttrList in_list)
{
	VistaIOAttrListPosn pos;
	VistaIOFirstAttr(in_list, &pos);
	while (VistaIOAttrExists(&pos)){
		std::string name(VistaIOGetAttrName(&pos));
		cvdebug() << "got " << name << " " << VistaIOGetAttrRepn(&pos) << "\n";
		switch (VistaIOGetAttrRepn(&pos)) {
		case VistaIOStringRepn:{
			VistaIOString s;
			VistaIOGetAttrValue(&pos,NULL,VistaIOStringRepn,&s);
			vistaio_add_attribute(attributes, name, s);
		}break;
		case VistaIOFloatRepn:{
			float f;
			VistaIOGetAttrValue(&pos,NULL,VistaIOFloatRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIODoubleRepn:{
			double f;
			VistaIOGetAttrValue(&pos,NULL,VistaIODoubleRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIOShortRepn:{
			short f;
			VistaIOGetAttrValue(&pos,NULL,VistaIOShortRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIOLongRepn:{
			long f;
			VistaIOGetAttrValue(&pos,NULL,VistaIOLongRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIOUByteRepn:{
			unsigned char f;
			VistaIOGetAttrValue(&pos,NULL,VistaIOUByteRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIOSByteRepn:{
			signed char f;
			VistaIOGetAttrValue(&pos,NULL,VistaIOSByteRepn,&f);
			vistaio_add_attribute(attributes, name, f);
		}break;
		case VistaIOBitRepn:{
			VistaIOBit b;
			VistaIOGetAttrValue(&pos,NULL,VistaIOBitRepn,&b);
			vistaio_add_attribute(attributes, name, b);
		}break;
		default:
			cvwarn() << "Not yet implemented\n";
		}
		VistaIONextAttr(&pos);
	}
}


VISTA4MIA_EXPORT void copy_attr_list(VistaIOAttrList target, const CAttributedData& data)
{
	for (auto i = data.begin_attributes(); i != data.end_attributes(); ++i) {
		VistaIOSetAttr(target, i->first.c_str(), NULL, VistaIOStringRepn, i->second->as_string().c_str());
	}
}


CVAttrList::CVAttrList(VistaIOAttrList list):m_list(list)
{
}

CVAttrList::~CVAttrList()
{
	VistaIODestroyAttrList(m_list);
}

CVAttrList::operator VistaIOAttrList()
{
	return m_list; 
}

VistaIOAttrList CVAttrList::operator ->()
{
	return m_list; 
}

bool CVAttrList::operator !() const
{
	return m_list == NULL; 
}

NS_MIA_END
