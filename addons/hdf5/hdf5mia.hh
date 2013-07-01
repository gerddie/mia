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

#include <hdf5.h>

NS_MIA_BEGIN

class  H5Handle {
protected: 
        typedef void (FCloseHandle)(hid_t hid); 
        
        H5Handle(hid_t hid,  FCloseHandle close_handle);

        // a copy would result in multiple destruction of the handle 
        H5Handle(const H5Handle& orig) = delete;
        H5Handle& operator = (const H5Handle& orig) = delete;

        ~H5Handle(); 
        
private:
        hid_t m_hid; 
        FCloseHandle m_close_handle; 
}; 

class H5Dataset: public H5Handle {
        H5Dataset(hid_t hid);
public: 
        typedef shared_ptr<H5Dataset> Pointer; 
        static Pointer create(hid_t file_id, const char *name, hid_t type_id, hid_t space_id, hid_t dcpl_id); 
        static Pointer open(hid_t file_id, const char *name); 
}; 

class H5Attribute: public H5Handle {
        H5Attribute(hid_t hid);
public: 
        typedef shared_ptr<H5Attribute> Pointer; 
        static Pointer create(hid_t loc_id, const char *attr_name, hid_t type_id, hid_t space_id, hid_t acpl_id ); 
};

class H5File: public H5Handle {
        H5File(hid_t hid);
public: 
        typedef shared_ptr<H5File> Pointer; 
        static Pointer create(const char *filename, unsigned access_mode, hid_t create_prp, hid_t  access_prp);
        static Pointer open(const char *filename);
};


class H5Space: public H5Handle {
        H5Space(hid_t hid);
public: 
        typedef shared_ptr<H5Space> Pointer; 
        static Pointer create(int rank, const hsize_t *dims);
};

template <typename T>
struct h5_iotypes {
        static const hid_t file_datatype =  0;
        static const hid_t mem_datatype = 0;
};

#define MIA_TO_H5_TYPE(T, FILE_TYPE, MEM_TYPE)                          \
        template <>                                                     \
        struct Mia_to_h5_types<T> {                                     \
                static const hid_t file_datatype =  FILE_TYPE;          \
                static const hid_t mem_datatype = MEM_TYPE;             \
        };                                                              \


MIA_TO_H5_TYPE(bool,           H5T_STD_B8LE, H5T_NATIVE_B8); 

MIA_TO_H5_TYPE(signed char,    H5T_STD_I8LE, H5T_NATIVE_SCHAR); 
MIA_TO_H5_TYPE(unsigned char,  H5T_STD_U8LE, H5T_NATIVE_UCHAR); 

MIA_TO_H5_TYPE(signed short,   H5T_STD_I16LE, H5T_NATIVE_SHORT); 
MIA_TO_H5_TYPE(unsigned short, H5T_STD_u16LE, H5T_NATIVE_USHORT); 

MIA_TO_H5_TYPE(signed int,     H5T_STD_I32LE, H5T_NATIVE_INT); 
MIA_TO_H5_TYPE(unsigned int,   H5T_STD_U32LE, H5T_NATIVE_UINT); 

#ifdef LONG_64BIT
MIA_TO_H5_TYPE(signed long,   H5T_STD_I64LE, H5T_NATIVE_LONG); 
MIA_TO_H5_TYPE(unsigned long, H5T_STD_U64LE, H5T_NATIVE_ULONG); 
#endif        

MIA_TO_H5_TYPE(float,  H5T_IEEE_F32LE,  H5T_NATIVE_FLOAT); 
MIA_TO_H5_TYPE(double, H5T_IEEE_F64LE,  H5T_NATIVE_DOUBLE);

#undef MIA_TO_H5_TYPE

NS_MIA_END

#endif 
