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


#define VSTREAM_DOMAIN "HDF5"
#include <addons/hdf5/hdf5a_mia.hh>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>


NS_MIA_BEGIN

using std::vector; 
using std::string; 
using std::invalid_argument; 
using std::runtime_error; 


typedef boost::mpl::vector<signed char,
			   unsigned char,
			   signed short,
			   unsigned short,
			   signed int,
			   float,
			   unsigned int,
#ifdef LONG_64BIT
			   signed long,
			   unsigned long,
#endif

			   double
			   > HDF5BasicPixelTypes;

template <typename I> 
static void check_id(hid_t id, const char *domain, const char *action, I info) 
{
	if (id < 0) {
		throw create_exception<invalid_argument>(domain, ": error in ", action, ":", info);  
	}
}


H5Attribute::H5Attribute(hid_t id, const H5Space& space):
	H5Base(H5AttributeHandle(id)), 
	m_space(space)
{
}

H5Attribute H5Attribute::write(const H5Base& parent, const char *name, const CAttribute& attr)
{
	return H5AttributeTranslatorMap::instance().translate(parent, name, attr); 
}

std::vector <hsize_t> H5Attribute::get_size() const
{
	return m_space.get_size(); 
}

H5Space H5Attribute::get_space()const
{
	return m_space;
}

H5Type H5Attribute::get_type() const
{
	H5Type file_type(H5Aget_type(*this)); 
	return file_type.get_native_type(); 
}

PAttribute H5Attribute::read(const H5Base& parent, const char *name)
{
	auto id = H5Aopen(parent, name, H5P_DEFAULT); 
	check_id(id, "H5Attribute", "open", name);

	
	auto space_id = H5Aget_space(id); 
	check_id(space_id, "H5Attribute", "get_space", name);
	H5Space space(space_id); 
	H5Attribute attr(id, space); 

	return 	H5AttributeTranslatorMap::instance().translate(name, attr); 
}


template <typename T> 
class H5TAttributeTranslator: public H5AttributeTranslator {
	H5Attribute apply(const H5Base& parent, const char *name, const CAttribute& attr) const; 
	virtual PAttribute apply(const char *name, const H5Attribute& attr ) const; 
}; 

template <typename T> 
struct  __dispatch_H5TAttributeTranslator {
	static H5Attribute apply(const H5Base& parent, const char *name, const TAttribute<T>& attr);
}; 

template <typename T> 
struct  __dispatch_H5TAttributeTranslator<vector<T>> {
	static H5Attribute apply(const H5Base& parent, const char *name, const TAttribute<vector<T>>& attr);
}; 


template <typename T> 
H5Attribute  __dispatch_H5TAttributeTranslator<T>::apply(const H5Base& parent, const char *name, const TAttribute<T>& attr)
{
	auto file_datatype = Mia_to_h5_types<T>::file_datatype();      
	auto mem_datatype = Mia_to_h5_types<T>::mem_datatype();

	T value = attr;
	auto space = H5Space::create();
	
	auto id = H5Acreate(parent, name, file_datatype, space, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Attribute", "translate", name);
	H5Attribute result(id, space);

	H5Awrite(result, mem_datatype, &value);
	result.set_parent(parent); 
	return result; 
}

template <typename T> 
H5Attribute  __dispatch_H5TAttributeTranslator<vector<T>>::apply(const H5Base& parent, const char *name, 
								 const TAttribute<vector<T>>& attr) 
{
	auto file_datatype = Mia_to_h5_types<T>::file_datatype();      
	auto mem_datatype = Mia_to_h5_types<T>::mem_datatype();

	const std::vector<T>& value = attr;
	auto space = H5Space::create(value.size());
	
	auto id = H5Acreate(parent, name, file_datatype, space, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Attribute", "translate", name);
	H5Attribute result(id, space);

	H5Awrite(result, mem_datatype, &value[0]);
	result.set_parent(parent); 
	return result; 
}

template <typename T> 
H5Attribute H5TAttributeTranslator<T>::apply(const H5Base& parent, const char *name, const CAttribute& __attr) const
{
	auto type_id = __attr.type_id(); 
	if (EAttributeType::is_vector(type_id)) {
		auto& attr = dynamic_cast<const TAttribute<vector<T>>&>(__attr);
		return __dispatch_H5TAttributeTranslator<vector<T>>::apply(parent, name, attr); 
	}else {
		auto& attr = dynamic_cast<const TAttribute<T>&>(__attr);
		return __dispatch_H5TAttributeTranslator<T>::apply(parent, name, attr); 
	}
}

template <typename T> 
PAttribute H5TAttributeTranslator<T>::apply(const char *name, const H5Attribute& attr) const
{
	auto dim = attr.get_size(); 
	if (dim.empty()) {
		T result_value; 
		H5Aread(attr, Mia_to_h5_types<T>::mem_datatype(), &result_value); 
		return PAttribute(new TAttribute<T>(result_value)); 
	}else if (dim.size() == 1) {
		vector<T> data(dim[0]); 
		H5Aread(attr, Mia_to_h5_types<T>::mem_datatype(), &data[0]);
		return PAttribute(new TAttribute<vector<T>>(data));
	}else {
		throw create_exception<runtime_error>("H5TAttributeTranslator: attribute(", name, "): with ", dim.size(), 
						      " not supported not supported by translator"); 
	}
}

class H5TAttributeStringTranslator: public H5AttributeTranslator {
	H5Attribute apply(const H5Base& parent, const char *name, const CAttribute& attr) const; 
	virtual PAttribute apply(const char *name, const H5Attribute& attr ) const; 
	
	H5Attribute generic_apply(const H5Base& parent, const char *name, const CAttribute& attr) const; 
	H5Attribute apply(const H5Base& parent, const char *name, const CVStringAttribute& attr) const; 

};

H5Attribute H5TAttributeStringTranslator::generic_apply(const H5Base& parent, const char *name, const CAttribute& attr) const
{
	const string value = attr.as_string(); 
	cvdebug() << "Write string attribute '" << attr << "'\n"; 

	H5Type stype(H5Tcopy (H5T_C_S1));
	H5Tset_size(stype, H5T_VARIABLE);

	vector<hsize_t> size(1, 1); 
	auto space = H5Space::create(size);

	auto id = H5Acreate(parent, name, stype, space, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Attribute", "translate string", name);
	H5Attribute result(id, space);

	const char *s = value.c_str(); 
	if (H5Awrite(result, stype, &s) < 0) 
		cvwarn() << "error writing attribute '"<< name << "'\n"; 
	
	result.set_parent(parent); 
	cvdebug() << "done with '" << attr << "'\n"; 
	return result; 

}

H5Attribute H5TAttributeStringTranslator::apply(const H5Base& parent, const char *name, const CVStringAttribute& attr) const
{
	const vector<string> value = attr;
	vector<hsize_t> dims(1,value.size()); 
	
	H5Type stype(H5Tcopy (H5T_C_S1));
	H5Tset_size(stype, H5T_VARIABLE); 
	
	
	H5Space space = H5Space::create(dims);

	auto id = H5Acreate(parent, name, stype, space, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Attribute", "translate string vector", name);
	H5Attribute result(id, space);

	vector<const char *> wstrings(value.size());
	transform(value.begin(), value.end(), wstrings.begin(), 
		  [](const string& s){return s.c_str();}); 
	
	if (H5Awrite(result, stype, &wstrings[0]) < 0) 
		cvwarn() << "error writing vector<string> attribute '"<< name << "'\n"; 
	result.set_parent(parent); 
	return result; 
}

H5Attribute H5TAttributeStringTranslator::apply(const H5Base& parent, const char *name, const CAttribute& __attr) const
{
	if (EAttributeType::is_vector(__attr.type_id())) {
		return apply(parent, name, dynamic_cast<const CVStringAttribute&>(__attr)); 
	} else {
		return generic_apply(parent, name, __attr); 
	}
}

PAttribute H5TAttributeStringTranslator::apply(const char *name, const H5Attribute& attr ) const
{
	PAttribute result; 
	auto size = attr.get_size(); 
	auto space = attr.get_space(); 
	
	if (size.size() > 1) {
		cvwarn() << "Support for " << size.size() << "-dimensional string attributes not implemented."
			 <<" Reading only first dimension.\n"; 
	}
	
	H5Type memtype(H5Tcopy (H5T_C_S1));
	H5Tset_size (memtype, H5T_VARIABLE);

	if (size.empty() || size[0] == 1) { 
		vector<char *> rdata(1);
		auto status = H5Aread (attr, memtype, &rdata[0]);
		if (status < 0) {
			cvwarn() << "Error reading string attribute\n"; 
			return PAttribute(); 
		}
		cvdebug() << "post-convert '" << name << "' from '" << rdata[0] << "'\n"; 
		result = CStringAttrTranslatorMap::instance().to_attr(name, string(rdata[0])); 
		H5Dvlen_reclaim (memtype, space, H5P_DEFAULT, &rdata[0]); 
	}else { 
		vector<char *> rdata(size[0]);
		auto status = H5Aread (attr, memtype, &rdata[0]);
		if (status < 0) {
			cvwarn() << "Error reading vector<string> attribute\n"; 
			return PAttribute(); 
		}
		vector<string> result_value; 
		for (size_t i = 0; i < size[0]; ++i) {
			result_value.push_back(string(rdata[i])); 
		}
		result.reset(new CVStringAttribute(result_value)); 
		H5Dvlen_reclaim (memtype, space, H5P_DEFAULT, &rdata[0]); 
	}
	return result; 
}

struct translator_append {
	translator_append(H5AttributeTranslatorMap& map):m_map(map){}; 
	template <typename T> 
	void operator () (T& MIA_PARAM_UNUSED(dummy)) {
		m_map.register_translator(attribute_type<T>::value, 
					  PH5AttributeTranslator(new H5TAttributeTranslator<T>)); 
	}
	H5AttributeTranslatorMap& m_map; 
};

H5AttributeTranslatorMap::H5AttributeTranslatorMap()
{
	boost::mpl::for_each<HDF5BasicPixelTypes>(translator_append(*this)); 

	register_translator(attribute_type<string>::value, 
			    PH5AttributeTranslator(new H5TAttributeStringTranslator()));
}

H5AttributeTranslatorMap& H5AttributeTranslatorMap::instance()
{
	static H5AttributeTranslatorMap map; 
	return map; 

}

void H5AttributeTranslatorMap::register_translator(int type_id, PH5AttributeTranslator translator)
{
	assert(m_map.find(type_id) == m_map.end()); 
	m_map[type_id] = translator; 	
}

const H5AttributeTranslator& H5AttributeTranslatorMap::get_translator(int type_id) const
{
	auto itranslator = m_map.find(type_id); 
	if (itranslator != m_map.end()) {
		return *itranslator->second; 
	}else{
		auto scalar_type_id = EAttributeType::scalar_type(type_id); 
		auto itranslator = m_map.find(scalar_type_id); 
		if (itranslator == m_map.end()) {
			auto tid = attribute_type<string>::value; 
			itranslator = m_map.find(tid); 
			assert(itranslator != m_map.end()); 
		}
		return *itranslator->second; 
	}
}

PAttribute  H5AttributeTranslatorMap::translate(const char *name, const H5Attribute& attr)
{
	return get_translator(attr.get_type().get_mia_type_id()).apply(name, attr); 
}

H5Attribute H5AttributeTranslatorMap::translate(const H5Base& parent, const char *name, const CAttribute& attr)
{
	return get_translator(attr.type_id()).apply(parent, name, attr); 
}

void translate_to_hdf5_attributes(const H5Base& target, const CAttributedData& data)
{
	for( auto a = data.begin_attributes(); a != data.end_attributes(); ++a)
		H5AttributeTranslatorMap::instance().translate(target, a->first.c_str(), *a->second); 
}

NS_MIA_END
