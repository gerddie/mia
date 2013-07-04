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

#ifndef addons_hdf5_hdf5mia_hh
#define addons_hdf5_hdf5mia_hh

#include <mia/core/attributes.hh> 
#include <mia/core/singular_refobj.hh> 
#include <miaconfig.h> 
#include <hdf5.h>

NS_MIA_BEGIN


struct H5Handle: public  TSingleReferencedObject<hid_t> {
	H5Handle() = default; 
	H5Handle(hid_t hid, const Destructor& d); 
	void set_parent(const H5Handle& parent); 
private: 
	TSingleReferencedObject<hid_t> m_parent;
}; 



struct H5SpaceHandle: public H5Handle {
        H5SpaceHandle(hid_t hid);
};

struct H5GroupHandle: public H5Handle {
        H5GroupHandle(hid_t hid);
};

struct H5DatasetHandle: public H5Handle {
        H5DatasetHandle(hid_t hid);
};

struct H5AttributeHandle: public H5Handle {
        H5AttributeHandle(hid_t hid);
};

struct H5FileHandle: public H5Handle {
        H5FileHandle(hid_t hid);
};

struct H5PropertyHandle: public H5Handle {
        H5PropertyHandle(hid_t hid);
};


class H5Base {
protected: 
	H5Base(const H5Handle& handle); 
public:
	H5Base() = default; 
	void set_parent(const H5Base& parent); 

	const H5Handle& get_handle() const;
	
	operator hid_t() const; 
private: 
	H5Handle m_handle; 
}; 


class H5Property: public H5Base {
	H5Property (hid_t id); 
public: 
	H5Property() = default; 
	static H5Property create(hid_t cls);
	
}; 

class H5File: public H5Base {
	H5File(hid_t id); 
public: 
	H5File() = default; 
	
	static H5File create(const char *name, unsigned flags, hid_t creation_prop, hid_t access_prop);
	static H5File open(const char *name, unsigned flags, hid_t access_prop);
}; 


NS_MIA_END

#endif 
