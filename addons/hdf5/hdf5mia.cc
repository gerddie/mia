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

H5Handle::H5Handle(hid_t hid,  FCloseHandle close_handle);:
m_hid(hid), 
        m_close_handle(close_handle)
{
}

H5Handle::~H5Handle() 
{
        m_close_handle(m_hid); 
}

H5Handle::operator hid_t() 
{
        return m_hid; 
}

H5Dataset::H5Dataset(hid_t hid):H5Handle(hid, H5Dclose) 
{
}

H5Dataset::Pointer H5Dataset::create(hid_t loc_id, const char *name, hid_t type_id, hid_t space_id, hid_t dcpl_id)
{
        auto id = H5Dcreate(loc_id, name, type_id, space_id, dcpl_id); 
        return Pointer(new H5Dataset(id)); 
}
 
H5Attribute::H5Attribute(hid_t hid):H5Handle(hid, H5Aclose) 
{
}

H5Attribute::Pointer H5Attribute::create(hid_t loc_id, const char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id )
{
        auto id = H5Acreate(loc_id, attr_name, type_id, space_id, acpl_id); 
        return Pointer(new H5Attribute(id));
}

H5File::H5File(hid_t hid):H5Handle(hid, H5Fclose) 
{
}

H5File::Pointer H5File::create(const char *filename, unsigned access_mode, hid_t create_prp, hid_t  access_prp)
{
        auto id = H5Fcreate(filename, access_mode, create_prp, access_prp); 
        return Pointer((id >= 0) ? new H5File(id) : nullptr); 
}

H5File::Pointer H5File::open(const char *filename, unsigned flags)
{
        auto id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT); 
        return Pointer( (id >= 0) ? new H5File(id) : nullptr); 
}

H5Space::H5Space(hid_t hid):H5Handle(hid, H5Sclose) 
{
}


Pointer H5Space::create(int rank, const hsize_t *dims)
{
        auto id = H5Screate(rank, dims, NULL); 
        return Pointer(new H5Space(id)); 
}

NS_MIA_END
