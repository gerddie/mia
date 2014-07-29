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

#ifndef addon_hdf5_hdf5mia_hh
#define addon_hdf5_hdf5mia_hh

#include <addons/hdf5/hdf5mia.hh>

NS_MIA_BEGIN

class HDF54MIA_EXPORT H5Attribute: public H5Base {
public: 
	H5Attribute(hid_t id, const H5Space& space); 
	H5Attribute() = default; 
	
	static H5Attribute write(const H5Base& parent, const char *name, const CAttribute& attr);
	static PAttribute read(const H5Base& parent, const char *name);

	H5Type get_type() const; 
	std::vector <hsize_t> get_size() const; 

	H5Space get_space()const; 
private: 
	PAttribute read_scalar();
	PAttribute read_vector();
	
	H5Space m_space; 
}; 

class HDF54MIA_EXPORT H5AttributeTranslator {
public: 
	virtual H5Attribute apply(const H5Base& parent, const char *name, const CAttribute& attr) const = 0; 
	virtual PAttribute apply(const char *name, const H5Attribute& attr ) const = 0; 
}; 
typedef std::shared_ptr<H5AttributeTranslator> PH5AttributeTranslator; 


class HDF54MIA_EXPORT H5AttributeTranslatorMap {
	H5AttributeTranslatorMap(); 

	H5AttributeTranslatorMap(const H5AttributeTranslatorMap& other) = delete; 
	H5AttributeTranslatorMap& operator = (const H5AttributeTranslatorMap& other) = delete; 
public: 
	static H5AttributeTranslatorMap& instance(); 
	void register_translator(int type_id, PH5AttributeTranslator translator);
	H5Attribute translate(const H5Base& parent, const char *name, const CAttribute& attr);
	PAttribute  translate(const char *name, const H5Attribute& parent);
private: 
	const H5AttributeTranslator& get_translator(int type_id) const; 
	typedef std::map<int,  PH5AttributeTranslator> TranslatorMap; 
	TranslatorMap m_map;
}; 

void HDF54MIA_EXPORT translate_to_hdf5_attributes(const H5Base& target, const CAttributedData& data); 

NS_MIA_END

#endif 
