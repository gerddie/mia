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

H5Handle::H5Handle(hid_t hid,  FCloseHandle close_handle):
	m_close_handle(close_handle), 
	m_hid(hid)
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

H5Dataset::H5Dataset(hid_t hid, H5Space::Pointer dataspace):
	H5Handle(hid, H5Dclose), 
	m_dataspace(dataspace)
{
}

H5Dataset::Pointer H5Dataset::create(hid_t loc_id, const char *name, hid_t type_id, H5Space::Pointer dataspace)
{
        auto id = H5Dcreate(loc_id, name, type_id, *dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); 
        return Pointer(new H5Dataset(id, dataspace)); 
}


int H5Dataset::add_attributes(const CAttributedData& attr)
{
	for (auto i = attr.begin_attributes(); i != attr.end_attributes(); ++i) {
		H5Attribute::create(*this, i->first.c_str(), *i->second); 
	}
	return 0; 
}

int H5Dataset::write(hid_t internal_type, void *data) 
{
	

	return H5Dwrite(*this, *m_dataspace, internal_type , H5S_ALL, H5P_DEFAULT, data);
};

H5Attribute::H5Attribute(hid_t hid):
	H5Handle(hid, H5Aclose) 
{
}


// non-vector types 
#define H5MiaAttributeTranslator_SCALAR_SPECIAL(T)				\
	template <>							\
	struct H5MiaAttributeTranslator<T> {				\
	static H5Attribute::Pointer apply(hid_t loc_id, const char *attr_name, const TAttribute<T>& attr) { \
		auto file_datatype = Mia_to_h5_types<T>::file_datatype(); \
		auto mem_datatype = Mia_to_h5_types<T>::mem_datatype();	\
									\
		auto space = H5Space::create();				\
		auto id = H5Acreate(loc_id, attr_name, file_datatype, *space, H5P_DEFAULT, H5P_DEFAULT); \
		T value = attr;					\
		H5Awrite(id, mem_datatype, &value); 			\
		return H5Attribute::Pointer(new H5Attribute(id));	\
	}								\
	}; 


H5MiaAttributeTranslator_SCALAR_SPECIAL(signed char); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(unsigned char); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(signed short); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(unsigned short); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(signed int); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(unsigned int); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(signed long); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(unsigned long); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(float); 
H5MiaAttributeTranslator_SCALAR_SPECIAL(double); 

#undef H5MiaAttributeTranslator_SCALAR_SPECIAL

// non-vector types 
#define H5MiaAttributeTranslator_VECTOR_SPECIAL(T)			\
	template <>							\
	struct H5MiaAttributeTranslator<vector<T> > {			\
		static H5Attribute::Pointer apply(hid_t loc_id, const char *attr_name, const TAttribute<vector<T>>& attr) { \
			auto file_datatype = Mia_to_h5_types<T>::file_datatype(); \
			auto mem_datatype = Mia_to_h5_types<T>::mem_datatype(); \
			const std::vector<T>& value = attr;		\
			hsize_t dim = value.size();				\
			auto space = H5Space::create(1, &dim);		\
			auto id = H5Acreate(loc_id, attr_name, file_datatype, *space, H5P_DEFAULT, H5P_DEFAULT); \
			H5Awrite(id, mem_datatype, &value[0]);	\
			return H5Attribute::Pointer(new H5Attribute(id)); \
		}							\
	}; 
H5MiaAttributeTranslator_VECTOR_SPECIAL(signed char); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(unsigned char); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(signed short); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(unsigned short); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(signed int); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(unsigned int); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(signed long); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(unsigned long); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(float); 
H5MiaAttributeTranslator_VECTOR_SPECIAL(double); 

#undef H5MiaAttributeTranslator_VECTOR_SPECIAL

template <typename T>
H5Attribute::Pointer create_h5attr(hid_t loc_id, const char *attr_name, const TAttribute<T>& attr) 
{
	return H5MiaAttributeTranslator<T>::apply(loc_id, attr_name,  attr); 
}

#warning This will not work because it can not be extended 

H5Attribute::Pointer H5Attribute::create(hid_t id, const char *attr_name, const CAttribute& attr )
{
#define call_create(type) create_h5attr(id, attr_name, dynamic_cast<const TAttribute<type>&>(attr))
	
	int type_id = attr.type_id(); 
	if (EAttributeType::is_vector(type_id)) {
		switch (EAttributeType::scalar_type(type_id)) {
		case EAttributeType::attr_uchar: return call_create(vector<unsigned char>);
		case EAttributeType::attr_schar: return call_create(vector<signed char>);
		case EAttributeType::attr_ushort:return call_create(vector<unsigned short>);
		case EAttributeType::attr_sshort:return call_create(vector<signed short>);
		case EAttributeType::attr_uint:  return call_create(vector<unsigned int>);
		case EAttributeType::attr_sint:  return call_create(vector<signed int>);
		case EAttributeType::attr_ulong: return call_create(vector<unsigned long>);
		case EAttributeType::attr_slong: return call_create(vector<signed long>);
		case EAttributeType::attr_float: return call_create(vector<float>);
		case EAttributeType::attr_double:return call_create(vector<double>); 
		default:
			cvwarn() << "don't know how to translate attribute " << attr_name << " of type " << type_id 
				 << ", dropping attribute\n"; 
		}
	} else {
		switch (type_id) {
		case EAttributeType::attr_uchar: return call_create(unsigned char);
		case EAttributeType::attr_schar: return call_create(signed char);
		case EAttributeType::attr_ushort:return call_create(unsigned short);
		case EAttributeType::attr_sshort:return call_create(signed short);
		case EAttributeType::attr_uint:  return call_create(unsigned int);
		case EAttributeType::attr_sint:  return call_create(signed int);
		case EAttributeType::attr_ulong: return call_create(unsigned long);
		case EAttributeType::attr_slong: return call_create(signed long);
		case EAttributeType::attr_float: return call_create(float);
		case EAttributeType::attr_double:return call_create(double); 
		default:
			cvwarn() << "don't know how to translate attribute " << attr_name << " of type " << type_id 
				 << ", dropping attribute\n"; 
		}
	}
#undef call_create
	return H5Attribute::Pointer(); 
}


H5File::H5File(hid_t hid):H5Handle(hid, H5Fclose) 
{
}

H5File::Pointer H5File::create(const char *filename, unsigned access_mode)
{
        auto id = H5Fcreate(filename, access_mode, H5P_DEFAULT, H5P_DEFAULT); 
        return Pointer((id >= 0) ? new H5File(id) : nullptr); 
}

H5File::Pointer H5File::open(const char *filename)
{
        auto id = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT); 
        return Pointer( (id >= 0) ? new H5File(id) : nullptr); 
}

H5Space::H5Space(hid_t hid):H5Handle(hid, H5Sclose) 
{
}


H5Space::Pointer H5Space::create(int rank, const hsize_t *dims)
{
        auto id = H5Screate_simple(rank, dims, NULL); 
        return Pointer(new H5Space(id)); 
}

H5Space::Pointer H5Space::create()
{
	return Pointer(new H5Space(H5Screate(H5S_SCALAR)));  
}

NS_MIA_END
