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

#include <addons/hdf5/hdf5a_mia.hh>
#include <stack>
NS_MIA_BEGIN

using std::vector; 
using std::string; 
using std::stack; 
using std::invalid_argument; 
using std::runtime_error; 



H5Handle::H5Handle(hid_t hid, const TSingleReferencedObject<hid_t>::Destructor& d):
	TSingleReferencedObject<hid_t>(hid, d)
{
}

void H5Handle::set_parent(const H5Handle& parent)
{
	m_parent = parent; 
}

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
H5Destructor(H5SpaceDestructor, H5Sclose);
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
	const H5Base& locator_id; 
}; 


static herr_t convert_attribute_cb(hid_t MIA_PARAM_UNUSED(location_id), const char *attr_name, 
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
	read_and_append_attributes(result); 
	return result; 
}

void  H5Base::read_and_append_attributes(CAttributedData& target) const
{
	SIterateData id = {target, *this}; 
	H5Aiterate2(*this, H5_INDEX_CRT_ORDER, H5_ITER_NATIVE, NULL, convert_attribute_cb, &id);
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


H5Dataset::H5Dataset (hid_t id, const H5Space& space, const char *name):
	H5Base(H5DatasetHandle(id)), 
	m_space(space), 
	m_name(name)
{
}

H5Dataset H5Dataset::create(const H5Base& parent, const char *name, hid_t type_id, const H5Space& space)
{
	string relative_name(name); 
	H5Base p = H5Group::create_or_open_hierarchy(parent, relative_name, true); 
	
	auto id =  H5Dcreate(p, relative_name.c_str(), type_id, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	check_id(id, "H5Dataset", "create", relative_name);
	H5Dataset set(id, space, name); 
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
	

	H5Dataset set(id, space, name); 
	set.set_parent(p);
	return set;
}

void  H5Dataset::write( hid_t type_id, const void *data)
{
	auto err =  H5Dwrite(*this, type_id, m_space,  H5S_ALL, H5P_DEFAULT, data);
	if (err < 0) {
		throw create_exception<runtime_error>("H5Dataset::write: error writing data set '", m_name, "'"); 
	}
}

void  H5Dataset::read( hid_t type_id, void *data)
{
	auto err =  H5Dread(*this, type_id, m_space,  H5S_ALL, H5P_DEFAULT, data);
	if (err < 0) {
		throw create_exception<runtime_error>("H5Dataset::read: error reading data set  '", m_name, "'"); 
	}
}

vector <hsize_t> H5Dataset::get_size() const
{
	return m_space.get_size(); 
}

NS_MIA_END
