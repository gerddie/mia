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

NS_MIA_BEGIN

using std::vector; 
using std::invalid_argument; 


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

H5SpaceHandle::H5SpaceHandle(hid_t hid):
	H5Handle(hid, H5SpaceDestructor)
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



NS_MIA_END
