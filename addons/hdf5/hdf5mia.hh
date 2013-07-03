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
#include <miaconfig.h> 
#include <hdf5.h>

NS_MIA_BEGIN


class  H5Handle {
public: 
	operator hid_t(); 	
protected: 
        typedef herr_t (*FCloseHandle)(hid_t hid); 
        
        H5Handle(hid_t hid,  FCloseHandle close_handle);

        // a copy would result in multiple destruction of the handle 
        H5Handle(const H5Handle& orig) = delete;
        H5Handle& operator = (const H5Handle& orig) = delete;

        ~H5Handle(); 
        

private:
        FCloseHandle m_close_handle; 
        hid_t m_hid; 
}; 


class H5Space: public H5Handle {
        H5Space(hid_t hid);
public: 
        typedef std::shared_ptr<H5Space> Pointer; 
        static Pointer create(int rank, const hsize_t *dims);
        static Pointer create();
};


template <typename T> 
struct H5MiaAttributeTranslator {
	static void apply(hid_t loc_id, const char *attr_name, const TAttribute<T>& attr) {
		static_assert(sizeof(T) == 0, "H5MiaAttributeTranslator must be specialized for type T"); 
	}
}; 

class H5Attribute: public H5Handle {
public: 
        H5Attribute(hid_t hid);
        typedef std::shared_ptr<H5Attribute> Pointer; 
        static Pointer create(hid_t loc_id, const char *attr_name, const CAttribute& attr);

};

class H5Dataset: public H5Handle {
        H5Dataset(hid_t hid, H5Space::Pointer m_dataspace);
public: 
        typedef std::shared_ptr<H5Dataset> Pointer; 
        static Pointer create(hid_t file_id, const char *name, hid_t type_id, H5Space::Pointer dataspace); 
        static Pointer open(hid_t file_id, const char *name); 
	
	int add_attributes(const CAttributedData& attr); 
	int write(hid_t internal_type, void *data); 
private: 
	H5Space::Pointer m_dataspace; 
}; 



class H5File: public H5Handle {
        H5File(hid_t hid);
public: 
        typedef std::shared_ptr<H5File> Pointer; 
        static Pointer create(const char *filename, unsigned access_mode);
        static Pointer open(const char *filename);
};

template <typename T>
struct Mia_to_h5_types {
        static hid_t file_datatype() {
		static_assert(sizeof(T) == 0, "Mia_to_h5_types needs to be specialized for T"); 
		return -1; 
	}
        static hid_t mem_datatype(){
		static_assert(sizeof(T) == 0, "Mia_to_h5_types needs to be specialized for T"); 
		return -1; 
	}
};

#define MIA_TO_H5_TYPE(T, FILE_TYPE, MEM_TYPE)                          \
        template <>                                                     \
        struct Mia_to_h5_types<T> {                                     \
		static hid_t file_datatype(){ return FILE_TYPE;}	\
                static hid_t mem_datatype(){ return MEM_TYPE;}		\
        };                                                              \


MIA_TO_H5_TYPE(bool,           H5T_STD_B8LE, H5T_NATIVE_B8); 

MIA_TO_H5_TYPE(signed char,    H5T_STD_I8LE, H5T_NATIVE_SCHAR); 
MIA_TO_H5_TYPE(unsigned char,  H5T_STD_U8LE, H5T_NATIVE_UCHAR); 

MIA_TO_H5_TYPE(signed short,   H5T_STD_I16LE, H5T_NATIVE_SHORT); 
MIA_TO_H5_TYPE(unsigned short, H5T_STD_U16LE, H5T_NATIVE_USHORT); 

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
