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


/**
   This file defines the helper classes needed to translate between HDF5 
   and MIA data sets. 

   This code is currently very messy. 

   The H5 classes encapsulate the HDF5 C interface, because the C++ interface is not thread save, and therefore 
   not installed in GNU/Debian. 

*/

#ifndef addons_hdf5_hdf5mia_hh
#define addons_hdf5_hdf5mia_hh

#include <mia/core/attributes.hh> 
#include <mia/core/singular_refobj.hh> 
#include <miaconfig.h> 
#include <hdf5.h>

NS_MIA_BEGIN

/**
   This is the excapsulation of the basic HDF5 handle id. All handles need to be 
   freed with H5Xclose, but it is not well documented whether the handles are 
   ref-counted internally. Therefore, we ref-count each created handle, reference 
   its parent, and destroy it only if noone is referencing it anymore. 
   On top of that 'X' is different for each type of HDF5 object which makes it necessary to 
   keep the specific close function with the handle. On top of  that there are default 
   handles that don't need to be destroyed with makes the  whole thing a bit inconsistent, 
   i.e. sometimes the here defined intefaces accept plain HDF5 types and sometimes the 
   wrapper is needed. However, the wrapper also provides an automatic conversion to the handle type. 
*/
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

struct H5TypeHandle: public H5Handle {
        H5TypeHandle(hid_t hid);
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
	
	CAttributedData read_attributes() const; 

	void  read_and_append_attributes(CAttributedData& target) const; 
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

class H5Space: public H5Base {
public: 
	explicit H5Space (hid_t id); 
	H5Space() = default; 
	static H5Space create();
	static H5Space create(hsize_t dim1);
	static H5Space create(unsigned rank, const hsize_t *dims);
	static H5Space create(const std::vector<hsize_t>& dims);
	
	std::vector<hsize_t> get_size() const; 
}; 

class H5Group: public H5Base {
public: 
	explicit H5Group (hid_t id); 
	H5Group() = default;
	static H5Base create_or_open_hierarchy(const H5Base& parent, std::string& relative_name, bool create); 

	static H5Base open(const H5Base& parent, const std::string& relative_name); 
}; 


class H5Type: public H5Base {
public: 
	explicit H5Type (hid_t id); 
	H5Type() = default; 
	
	H5Type get_native_type() const;

	int get_mia_type_id() const; 
}; 

class H5Dataset: public H5Base {
	H5Dataset (hid_t id, const H5Space& space, const char *name); 
public: 
	H5Dataset() = default; 
	static H5Dataset create(const H5Base& parent, const char *name, hid_t type_id, const H5Space& space);

	static H5Dataset open(const H5Base& parent, const char *name);

	template <typename Iterator> 
	void  write(Iterator begin, Iterator end);
	
	template <typename Iterator> 
	void  read(Iterator begin, Iterator end) const;

	std::vector <hsize_t> get_size() const; 
	
private: 

	template <typename Iterator, typename T> 
	friend struct __dispatch_h5dataset_rw; 
	
	void  write( hid_t type_id, const void *data);
	void  read( hid_t type_id, void *data) const;

	H5Space m_space; 
	std::string m_name; 
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

template <typename T>
struct Mia_to_h5_types<std::vector<T>>  {
        static hid_t file_datatype() {
		return Mia_to_h5_types<T>::file_datatype(); 
	}
        static hid_t mem_datatype(){
		return Mia_to_h5_types<T>::mem_datatype(); 
	}
};


template <typename Image> 
typename Image::Pointer read_image(typename Image::dimsize_type& size, const H5Dataset& dataset)
{
	typedef typename Image::dimsize_type Bounds; 
	typedef typename Image::value_type value_type; 

	Image *result = new Image(size); 
	typename Image::Pointer presult(result); 
	
	dataset.read_and_append_attributes(*result);
	dataset.read(result->begin(), result->end());
	
	return presult; 
}

template <typename Iterator, typename T> 
struct __dispatch_h5dataset_rw {
	static void apply_write(H5Dataset& id, Iterator begin, Iterator MIA_PARAM_UNUSED(end)) {
		TRACE_FUNCTION; 
		id.write(Mia_to_h5_types<T>::mem_datatype(), &begin[0]); 
	}
	static void apply_read(const H5Dataset& id, Iterator begin, Iterator MIA_PARAM_UNUSED(end)) {
		TRACE_FUNCTION; 
		id.read(Mia_to_h5_types<T>::mem_datatype(), &begin[0]); 
	}
}; 

template <typename Iterator> 
struct __dispatch_h5dataset_rw<Iterator, bool> {
	static void apply_write(H5Dataset& id, Iterator begin, Iterator end) {
		TRACE_FUNCTION; 
		std::vector<char> help(std::distance( begin, end)); 
		copy(begin, end, help.begin()); 
		id.write(Mia_to_h5_types<bool>::mem_datatype(), &help[0]); 
		
	}
	static void apply_read(const H5Dataset& id, Iterator begin, Iterator end) {
		TRACE_FUNCTION; 
		std::vector<char> help(std::distance(begin, end)); 
		id.read(Mia_to_h5_types<bool>::mem_datatype(), &help[0]); 
		copy(help.begin(), help.end(), begin);
	}
}; 


template <typename Iterator> 
void  H5Dataset::write(Iterator begin, Iterator end)
{
	typedef __dispatch_h5dataset_rw<Iterator, typename Iterator::value_type> h5dataset_rw; 
	h5dataset_rw::apply_write(*this, begin, end); 
}

template <typename Iterator> 
void  H5Dataset::read(Iterator begin, Iterator end)const
{
	typedef __dispatch_h5dataset_rw<Iterator, typename Iterator::value_type> h5dataset_rw; 
	h5dataset_rw::apply_read(*this, begin, end); 
}



NS_MIA_END

#endif 
