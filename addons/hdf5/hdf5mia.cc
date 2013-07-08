/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <addons/hdf5/hdf5mia.hh>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <stack>
NS_MIA_BEGIN

using std::vector; 
using std::string; 
using std::stack; 
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


H5Handle::H5Handle(hid_t hid, const TSingleReferencedObject<hid_t>::Destructor& d):
	TSingleReferencedObject<hid_t>(hid, d)
{
}

void H5Handle::set_parent(const H5Handle& parent)
{
	m_parent = parent; 
}


struct H5SpaceDestructor : public TSingleReferencedObject<hid_t>::Destructor {	
	virtual void operator ()(hid_t& handle)const {			
		herr_t err = H5Sclose(handle);				
		if (err != 0) {						
			throw std::runtime_error("H5SpaceDestructor: error closing handle."); 
		}				
	}
};						

static const H5SpaceDestructor H5SpaceDestructor; 


#define H5Destructor(TYPE, CALL)					\
	struct TYPE : public TSingleReferencedObject<hid_t>::Destructor {				\
		virtual void operator ()(hid_t& handle)const {		\
			herr_t err = CALL(handle);			\
			if (err != 0) {					\
				throw std::runtime_error(#TYPE ": error closing handle."); \
			}						\
		}							\
	};								\
	static const TYPE TYPE;						\

H5Destructor(H5GroupDestructor, H5Gclose);

//H5Destructor(H5SpaceDestructor, H5Sclose);

H5Destructor(H5DatasetDestructor, H5Dclose);
H5Destructor(H5FileDestructor, H5Fclose);
H5Destructor(H5AttributeDestructor, H5Aclose);
H5Destructor(H5PropertyDestructor, H5Pclose); 
H5Destructor(H5TypeDestructor, H5Tclose); 

H5SpaceHandle::H5SpaceHandle(hid_t hid):
	H5Handle(hid, H5SpaceDestructor)
{
}

H5TypeHandle::H5TypeHandle(hid_t hid):
	H5Handle(hid, H5TypeDestructor)
{
}


H5GroupHandle::H5GroupHandle(hid_t hid):
	H5Handle(hid, H5GroupDestructor)
{
}

H5DatasetHandle::H5DatasetHandle(hid_t hid):
	H5Handle(hid, H5DatasetDestructor)
{
}

H5AttributeHandle::H5AttributeHandle(hid_t hid):
	H5Handle(hid, H5AttributeDestructor)
{
}

H5PropertyHandle::H5PropertyHandle(hid_t hid):
	H5Handle(hid, H5PropertyDestructor)
{
}	

H5FileHandle::H5FileHandle(hid_t hid):
	H5Handle(hid, H5FileDestructor)
{
}

H5Base::H5Base(const H5Handle& handle):
	m_handle(handle)
{
}

void H5Base::set_parent(const H5Base& parent)
{
	m_handle.set_parent(parent.get_handle()); 
}

const H5Handle& H5Base::get_handle() const
{
	return m_handle; 
}

struct SIterateData {
	CAttributedData& list; 
	H5Base locator_id; 
}; 


herr_t convert_attribute_cb(hid_t MIA_PARAM_UNUSED(location_id), const char *attr_name, 
			    const H5A_info_t *MIA_PARAM_UNUSED(ainfo), void *op_data)
{
	SIterateData& iter_data = *reinterpret_cast<SIterateData *>(op_data); 
	cvdebug() << "convert_attribute_cb: read '" << attr_name << "'\n"; 
	auto pattr = H5Attribute::read(iter_data.locator_id, attr_name); 
	if (pattr) 
		iter_data.list.set_attribute(attr_name, pattr); 
	
	return 0; 
}

CAttributedData H5Base::read_attributes() const
{
	CAttributedData result; 
	SIterateData id = {result, *this}; 
	H5Aiterate2(*this, H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, NULL, convert_attribute_cb, &id);
	return result; 
}

H5Base::operator hid_t() const
{
	return m_handle; 
}

template <typename I> 
static void check_id(hid_t id, const char *domain, const char *action, I info) 
{
	if (id < 0) {
		throw create_exception<invalid_argument>(domain, ": error in ", action, ":", info);  
	}
}

H5Property::H5Property(hid_t id):
	H5Base(H5PropertyHandle(id))
{
}

H5Property H5Property::create(hid_t cls)
{
	auto id = H5Pcreate(cls); 
	check_id(id, "H5Property", "Create", cls); 
	return H5Property(id); 
}


H5File::H5File(hid_t id):
	H5Base(H5FileHandle(id))
{
}

H5File H5File::create(const char *name, unsigned flags, hid_t  creation_prop, hid_t access_prop)
{
	auto id = H5Fcreate(name, flags, creation_prop, access_prop); 
	check_id(id, "H5File", "Create", name);
	return H5File(id); 
}
	
H5File H5File::open(const char *name, unsigned flags, hid_t access_prop)
{
	auto id = H5Fopen(name, flags, access_prop); 
	check_id(id, "H5File", "Open", name);
	return H5File(id);
}


H5Space::H5Space (hid_t id):
	H5Base(H5SpaceHandle(id))
{
}
	
H5Space H5Space::create() 
{
	return H5Space(H5Screate(H5S_SCALAR)); 
}

H5Space H5Space::create(hsize_t dim1)
{
	auto id = H5Screate_simple(1, &dim1, NULL); 
	check_id(id, "H5Space", "create_simple 1d", dim1);
	return H5Space(id);
}

H5Space H5Space::create(unsigned rank, const hsize_t *dims)
{
	auto id = H5Screate_simple(rank, dims, NULL); 
	check_id(id, "H5Space", "create_simple", rank);
	return H5Space(id); 
}

H5Space H5Space::create(const std::vector<hsize_t>& dims)
{
	return create(dims.size(), &dims[0]); 
}

std::vector<hsize_t> H5Space::get_size() const
{
	int  dims = H5Sget_simple_extent_ndims(*this);
	if (dims < 0) 
		throw create_exception<runtime_error>("H5Dataset::get_size: error reading dimensions");
	
	vector <hsize_t> result(dims); 
	if (dims > 0) {
		auto status = H5Sget_simple_extent_dims(*this,  &result[0], NULL);
		if (status < 0) 
			throw create_exception<runtime_error>("H5Dataset::get_size: error reading dimensions");
	}
	return result; 
}

H5Group::H5Group (hid_t id):
	H5Base(H5GroupHandle(id))
{
}


struct SilenceH5Errors {
	SilenceH5Errors();
	~SilenceH5Errors(); 
private:
	H5E_auto2_t  old_func;
	void *old_client_data;
}; 

SilenceH5Errors::SilenceH5Errors()
{
	H5Eget_auto(H5E_DEFAULT, &old_func, &old_client_data);
	H5Eset_auto(H5E_DEFAULT, NULL, NULL); 
}

SilenceH5Errors::~SilenceH5Errors()
{
	H5Eset_auto(H5E_DEFAULT, old_func, old_client_data);
}


H5Base H5Group::create_or_open_hierarchy(const H5Base& parent, string& relative_name, bool create)
{
	H5Base pp = parent; 
	// only deal with fully qualified names
	assert(relative_name[0] == '/'); 

	size_t last_slash = relative_name.find_last_of('/'); 
	
	// only root group required no need to do anything 
	if (last_slash > 0)  {
		SilenceH5Errors err; 
		string base = relative_name.substr(0, last_slash); 
		cvdebug() << "Base: " << base << "\n"; 
		
		relative_name = relative_name.substr(last_slash+1);
		
		stack<string> path; 
		last_slash = base.find_last_of('/'); 
		while (last_slash > 0)  {
			auto tail = base.substr(last_slash+1); 

			path.push(base.substr(last_slash+1)); 
			base = base.substr(0, last_slash); 
			cvdebug() << "tail:" << tail << " "
				  << "base:" << base << "\n"; 
			last_slash = base.find_last_of('/'); 
		}
		path.push(base.substr(1)); 
		
		
		while (!path.empty()) {

			auto name = path.top(); 
			path.pop(); 

			cvdebug() << "do group '" << name <<"'\n";
			// it needs to be tested whether multiple slashes are to be merged
			if (name.empty())
				continue; 
			auto id = H5Gopen(parent, name.c_str(), H5P_DEFAULT); 
			if (id < 0 && create)
				id = H5Gcreate(parent, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
			
			H5Group p(id); 
			p.set_parent(pp); 
			pp = p; 
		}
	}
	return pp; 
}

H5Type::H5Type (hid_t id):
	H5Base(H5TypeHandle(id))
{
}

H5Type H5Type::get_native_type() const
{
	auto id = H5Tget_native_type(*this, H5T_DIR_ASCEND);
	if (id < 0) {
		throw runtime_error("H5Type::get_native_type, Unable to deduct mem_type for reading data");
	}
	return H5Type(id); 
}

int H5Type::get_mia_type_id() const
{
	H5T_class_t cls =  H5Tget_class(*this); 
	size_t size = H5Tget_size(*this); 

	switch (cls) {
	case H5T_INTEGER: 
		switch (size) {
		case 1: return 2 | H5Tget_sign(*this ); 
		case 2: return 4 | H5Tget_sign(*this ); 
		case 4: return 6 | H5Tget_sign(*this ); 
		case 8: return 8 | H5Tget_sign(*this ); 			
		default: 
			return EAttributeType::attr_unknown;
		}
			
	case H5T_FLOAT: 
		if (size == 4) 
			return EAttributeType::attr_float; 
		else if (size == 8) 
			return EAttributeType::attr_double; 
		else 
			return EAttributeType::attr_unknown;
	case H5T_STRING:
		return EAttributeType::attr_string; 
	default: 
		return EAttributeType::attr_unknown;
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

	return 	H5AttributeTranslatorMap::instance().translate(attr); 
}


template <typename T> 
class H5TAttributeTranslator: public H5AttributeTranslator {
	H5Attribute apply(const H5Base& parent, const char *name, const CAttribute& attr) const; 
	virtual PAttribute apply(const H5Attribute& attr ) const; 
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
PAttribute H5TAttributeTranslator<T>::apply(const H5Attribute& attr) const
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
		throw create_exception<runtime_error>("H5TAttributeTranslator: ", dim.size(), 
						      " attributes not supported by generic translator"); 
	}
}

class H5TAttributeStringTranslator: public H5AttributeTranslator {
	H5Attribute apply(const H5Base& parent, const char *name, const CAttribute& attr) const; 
	virtual PAttribute apply(const H5Attribute& attr ) const; 
	
	H5Attribute apply(const H5Base& parent, const char *name, const CStringAttribute& attr) const; 
	H5Attribute apply(const H5Base& parent, const char *name, const CVStringAttribute& attr) const; 

};

H5Attribute H5TAttributeStringTranslator::apply(const H5Base& parent, const char *name, const CStringAttribute& attr) const
{
	const string value = attr; 
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
		return apply(parent, name, dynamic_cast<const CStringAttribute&>(__attr)); 
	}
}

PAttribute H5TAttributeStringTranslator::apply(const H5Attribute& attr ) const
{
	PAttribute result; 
	auto size = attr.get_size(); 
	auto space = attr.get_space(); 
	
	if (size.size() > 1) {
		cvwarn() << "Suport for " << size.size() << "-dimensional string attributes not implemented."
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
		result.reset(new CStringAttribute(string(rdata[0]))); 
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
		if (itranslator != m_map.end()) {
			return *itranslator->second; 
		}else{
			throw create_exception<invalid_argument>("No translator for type id ", type_id, " scalar", scalar_type_id); 
		}
	}
}

PAttribute  H5AttributeTranslatorMap::translate(const H5Attribute& attr)
{
	return get_translator(attr.get_type().get_mia_type_id()).apply(attr); 
}

H5Attribute H5AttributeTranslatorMap::translate(const H5Base& parent, const char *name, const CAttribute& attr)
{
	return get_translator(attr.type_id()).apply(parent, name, attr); 
}

H5Dataset::H5Dataset (hid_t id, const H5Space& space):
	H5Base(H5DatasetHandle(id)), 
	m_space(space)
{
}

H5Dataset H5Dataset::create(const H5Base& parent, const char *name, hid_t type_id, const H5Space& space)
{
	string relative_name(name); 
	H5Base p = H5Group::create_or_open_hierarchy(parent, relative_name, true); 
	
	auto id =  H5Dcreate(p, relative_name.c_str(), type_id, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Dataset", "create", relative_name);
	H5Dataset set(id, space); 
	set.set_parent(p);
	return set;
}

H5Dataset H5Dataset::open(const H5Base& parent, const char *name)
{
	string relative_name(name); 
	H5Base p = H5Group::create_or_open_hierarchy(parent, relative_name, false); 


	auto id =  H5Dopen(p, relative_name.c_str(), H5P_DEFAULT);
	check_id(id, "H5Dataset", "open", relative_name);

	int space_id = H5Dget_space(id); 
	check_id(space_id, "H5Dataset", "get space", name);
	H5Space space(space_id); 
	H5Dataset set(id, space); 
	set.set_parent(p);
	return set;
}

void  H5Dataset::write( hid_t type_id, const void *data)
{
	auto err =  H5Dwrite(*this, type_id, m_space,  H5S_ALL, H5P_DEFAULT, data);
	if (err < 0) {
		throw create_exception<runtime_error>("H5Dataset::write: error writing data set TODO:display name"); 
	}
}

void  H5Dataset::read( hid_t type_id, void *data)
{
	auto err =  H5Dread(*this, type_id, m_space,  H5S_ALL, H5P_DEFAULT, data);
	if (err < 0) {
		throw create_exception<runtime_error>("H5Dataset::read: error reading data set TODO:display name"); 
	}
}

vector <hsize_t> H5Dataset::get_size() const
{
	return m_space.get_size(); 
}




NS_MIA_END
