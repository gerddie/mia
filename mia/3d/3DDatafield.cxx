/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __3ddatafield_cxx
#define __3ddatafield_cxx


#include <cstring>
#include <mia/core/msgstream.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/3d/3DDatafield.hh>


NS_MIA_BEGIN

template <typename T>
const unsigned int T3DDatafield<T>::m_elements = 
	sizeof(T) / sizeof(typename T3DDatafield<T>::atomic_type); 

template <typename T>
T3DDatafield<T>::T3DDatafield():
	m_size(C3DBounds(0,0,0)), 
	m_xy(0), 
	m_data(new std::vector<T>(0))
{
}

template <typename T>
void T3DDatafield<T>::swap(T3DDatafield& other)
{
	::std::swap(m_size, other.m_size); 
	::std::swap(m_xy, other.m_xy); 
	m_data.swap(other.m_data); 
}

template <typename T>
T3DDatafield<T>::T3DDatafield(const C3DBounds& size ):
	m_size(size),
	m_xy(size.x * size.y), 
	m_data(new std::vector<T>(size.x * size.y * size.z))
{
}

template <typename T>
T3DDatafield<T>::T3DDatafield(const C3DBounds& size, const T *data):
	m_size(size), 
	m_xy(size.x * size.y), 
	m_data(new std::vector<T>(size.x * size.y * size.z))
{
	std::copy(data, data + m_data->size(), m_data->begin()); 
}
	
template <typename T>
T3DDatafield<T>::~T3DDatafield()
{
}

template <typename T>
void T3DDatafield<T>::make_single_ref()
{
	if (!m_data.unique())
		m_data = ref_data_type( new std::vector<T>(*m_data) );
}

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_interpol_val_at(const T3DVector<float >& p) const
{
        return get_trilin_interpol_val_at(p);
}

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::operator()(const T3DVector<float >& pos)const
{
        return get_trilin_interpol_val_at(pos);
}

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_trilin_interpol_val_at(const T3DVector<float >& p) const
{
        // Calculate the coordinates and the distances
        size_t  x = (size_t )p.x;
        size_t  y = (size_t )p.y;
        size_t  z = (size_t )p.z;
        float  fx = p.x-x;
        float  fy = p.y-y;
        float  fz = p.z-z;

        float  dx = 1-fx;
        float  dy = 1-fy;
        float  dz = 1-fz;

        if ( x < m_size.x-1 && y  < m_size.y -1 && z < m_size.z -1 ) {

                const T *ptr = &(*this)[x + m_size.x * (y +m_size.y * z)];
                const T *ptr_h = &ptr[m_size.x];
                const T *ptr2 = &ptr[m_xy];
                const T *ptr2_h = &ptr2[m_size.x];
                const T a1 = T(dx * ptr[0]    + fx * ptr[1]);
                const T a3 = T(dx * ptr_h[0]  + fx * ptr_h[1]);
                const T a5 = T(dx * ptr2[0]   + fx * ptr2[1]);
                const T a7 = T(dx * ptr2_h[0] + fx * ptr2_h[1]);
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        } else {
                const T a1 = T(dx * (*this)(x  , y  , z  ) + fx * (*this)(x+1, y  , z  ));
                const T a3 = T(dx * (*this)(x  , y+1, z  ) + fx * (*this)(x+1, y+1, z  ));
                const T a5 = T(dx * (*this)(x  , y  , z+1) + fx * (*this)(x+1, y  , z+1));
                const T a7 = T(dx * (*this)(x  , y+1, z+1) + fx * (*this)(x+1, y+1, z+1));
		const T b1 = T(dy * a1 + fy * a3);
		const T b2 = T(dy * a5 + fy * a7);
		return  T(dz * b1 + fz * b2);
        }
}

template <typename T>
struct __copy_dispatch {
	typedef typename atomic_data<T>::type atomic_type; 
	typedef std::vector<atomic_type> atomic_type_vector;
	typedef std::vector<T> type_vector;
	
	static void apply_write(type_vector& dest, size_t dest_start, 
				const atomic_type_vector& src, 
				size_t src_start, size_t nelm) {
		static_assert(__has_trivial_copy(T), "this copy mechanism can only be used if T has "
			      "a trivial assignment operator"); 
		memcpy(&dest[dest_start], &src[src_start], nelm * sizeof(T));
	}
	static void apply_read(atomic_type_vector& dest, size_t dest_start, 
			       const type_vector& src, size_t src_start, 
			       size_t nelm) {
		static_assert(__has_trivial_copy(T), "this copy mechanism can only be used if T has "
			      "a trivial assignment operator"); 
		memcpy(&dest[dest_start], &src[src_start], nelm * sizeof(T));
	}
};

template <>
struct __copy_dispatch<bool> {
	static void apply_write(std::vector<bool>& dest, size_t dest_start, 
				const std::vector<bool>& src, size_t src_start, size_t nelm) {
		copy(src.begin() + src_start, src.begin() + src_start + nelm, dest.begin() + dest_start); 
	}
	static void apply_read(std::vector<bool>& dest, size_t dest_start, 
			       const std::vector<bool>& src, size_t src_start, 
			       size_t nelm) {
		copy(src.begin() + src_start, src.begin() + src_start + nelm, 
		     dest.begin() + dest_start); 
	}

};

template <typename T, bool trivial_copy> 
struct __mia_copy_dispatch_1 {
	static void apply_read(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		std::copy(src.begin() + start, src.begin() + start + n, dest.begin()); 
	}
	static void apply_write(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		std::copy(src.begin(), src.begin() + n, dest.begin() + start); 
	}
}; 

template <typename T> 
struct __mia_copy_dispatch_1<T, true> {
	static void apply_read(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		memcpy(&dest[0], &src[start], n * sizeof(T)); 
	}
	static void apply_write(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		memcpy(&dest[start], &src[0], n * sizeof(T)); 
	}
}; 

template <typename T> 
struct __mia_copy_dispatch {
	static void apply_read(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		__mia_copy_dispatch_1<T, __has_trivial_copy(T)>::apply_read(dest, src, start, n); 
	}
	static void apply_write(std::vector<T>& dest, const std::vector<T>& src, size_t start, size_t n) {
		__mia_copy_dispatch_1<T, __has_trivial_copy(T)>::apply_write(dest, src, start, n);
	}
}; 


template <> 
struct __mia_copy_dispatch<bool> {
	static void apply_read(std::vector<bool>& dest, const std::vector<bool>& src, size_t start, size_t n) {
		std::copy(src.begin() + start, 
			  src.begin() + start + n, dest.begin()); 
	}
	static void apply_write(std::vector<bool>& dest, const std::vector<bool>& src, size_t start, size_t n) {
		std::copy(src.begin(), src.begin() + n, dest.begin() + start); 
	}
}; 




template <typename T>
void T3DDatafield<T>::get_data_line_x(int y, int z, std::vector<T>& result)const
{
        result.resize(m_size.x);
	const int start = m_size.x * (y + z * m_size.y); 
	__mia_copy_dispatch<T>::apply_read(result, *m_data, start, m_size.x); 
}

template <typename T>
void T3DDatafield<T>::get_data_line_y(int x, int z, std::vector<T>& result)const
{
        result.resize(m_size.y);
	size_t start = x + m_size.x * m_size.y * z;

	typename std::vector<T>::const_iterator i = m_data->begin() + start; 
	
	for (typename std::vector<T>::iterator k = result.begin(); k != result.end(); ++k, i += m_size.x)
                *k = *i;
}

template <typename T>
void T3DDatafield<T>::get_data_line_z(int x, int y, std::vector<T>& result)const
{
        result.resize(m_size.z);
	size_t start = x + m_size.x * y;


	typename std::vector<T>::const_iterator i = m_data->begin() + start;
	typename std::vector<T>::iterator k = result.begin(); 
	
	if (m_size.z > 8) {
		const size_t xy = m_xy; 
		const size_t xy2 = m_xy << 1; 
		const size_t xy3 = m_xy + xy2; 
		const size_t xy4 =  xy2 << 1; 
		int  z = m_size.z - 4; 
		while ( z > 0) {
			k[0] = i[0];
			k[1] = i[xy];
			k[2] = i[xy2];
			k[3] = i[xy3];
			z -= 4; 
			k += 4;
			i += xy4;
		}
	}
		
	for (; k != result.end(); ++k, i += m_xy)
		*k = *i;

}


template <typename T>
void T3DDatafield<T>::put_data_line_x(int y, int z, const std::vector<T>& input)
{
        assert(input.size() == m_size.x);
	make_single_ref();
	size_t start = m_size.x * (y + z * m_size.y); 
	__mia_copy_dispatch<T>::apply_write(*m_data, input, start, m_size.x); 		
}

template <typename T>
void T3DDatafield<T>::put_data_line_y(int x, int z, const std::vector<T>& input)
{
        assert(input.size() == m_size.y);
	
	make_single_ref();
	size_t start= x + m_size.x * m_size.y * z;
	
	typename std::vector<T>::iterator k = m_data->begin() + start; 

	for (typename std::vector<T>::const_iterator i = input.begin(); 
	     i != input.end(); ++i, k += m_size.x)
		*k = *i; 
}

template <typename T>
void T3DDatafield<T>::put_data_line_z(int x, int y, const std::vector<T>& input)
{
        assert(input.size() == m_size.z);

	make_single_ref();
	size_t start= x + m_size.x * y;
	
	typename std::vector<T>::iterator k = m_data->begin() + start; 

	for (typename std::vector<T>::const_iterator i = input.begin(); 
	     i != input.end(); ++i, k += m_xy)
		 *k = *i; 
}


template <typename T>
T3DDatafield<T>& T3DDatafield<T>::operator = (const T3DDatafield<T>& org)
{
        if (&org == this) {
                return *this;
        }
	
        m_size = org.m_size;
        m_xy = org.m_xy;
        m_data = org.m_data;

        return *this;
}

template <typename T>
T3DDatafield<T>::T3DDatafield(const T3DDatafield<T>& org):
	m_size(org.m_size),
	m_xy(org.m_xy),
	m_data(org.m_data)
{
}

template <typename T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_xy(size_t  z)const
{
	assert(z < get_size().z); 
	T2DDatafield<T> result = T2DDatafield<T>(C2DBounds(get_size().x, get_size().y)); 
	copy(begin_at(0,0,z), begin_at(0,0,z) + result.size(), result.begin()); 
	return result; 
}



template <typename T>
void T3DDatafield<T>::read_zslice_flat(size_t z, std::vector<atomic_type>& buffer)const
{
	assert(z < get_size().z); 
	assert(m_xy * m_elements <= buffer.size()); 
	__copy_dispatch<T>::apply_read(buffer, 0, *m_data, z * m_xy, m_xy); 
}


template <typename T>
void T3DDatafield<T>::write_zslice_flat(size_t z, const std::vector<atomic_type>& buffer)
{
	assert(z < get_size().z); 
	assert(m_xy * m_elements <= buffer.size()); 
	__copy_dispatch<T>::apply_write(*m_data, z * m_xy, buffer, 0, m_xy); 
}


template <typename T>
void T3DDatafield<T>::read_yslice_flat(size_t y, std::vector<atomic_type>& buffer)const
{
	assert(y < get_size().y); 
	assert(get_size().x * get_size().z * m_elements <= buffer.size()); 
	
	const size_t offset = y * get_size().x; 
	for (size_t z = 0; z < get_size().z; ++z) {
		__copy_dispatch<T>::apply_read(buffer, z * get_size().x  * m_elements, 
					       *m_data, offset + z * m_xy, get_size().x); 
	}
}

template <typename T>
void T3DDatafield<T>::write_yslice_flat(size_t y, const std::vector<atomic_type>& buffer )
{
	assert(y < get_size().y); 
	assert(get_size().x * get_size().z * m_elements <= buffer.size()); 
	
	const size_t offset = y * get_size().x; 
	for (size_t z = 0; z < get_size().z; ++z) {
		__copy_dispatch<T>::apply_write(*m_data, offset + z * m_xy,
						buffer, z * get_size().x * m_elements, 
					        get_size().x); 
	}
}

template <typename T>
void T3DDatafield<T>::read_xslice_flat(size_t x, std::vector<atomic_type>& buffer)const
{
	assert(x < get_size().x); 
	const size_t slice_size = get_size().y * get_size().z; 
	assert(slice_size * m_elements <= buffer.size()); 
	
	size_t offset = x; 
	size_t doffs =  get_size().x; 
	for (size_t i = 0; i < slice_size; ++i, offset += doffs) {
		__copy_dispatch<T>::apply_read(buffer, m_elements * i, 
					       *m_data, offset, 1); 
	}
}

template <typename T>
void T3DDatafield<T>::write_xslice_flat(size_t x, const std::vector<atomic_type>& buffer)
{
	assert(x < get_size().x); 
	const size_t slice_size = get_size().y * get_size().z; 
	assert(slice_size * m_elements <= buffer.size()); 
	
	size_t offset = x; 
	size_t doffs =  get_size().x; 
	for (size_t i = 0; i < slice_size; ++i, offset += doffs) {
		__copy_dispatch<T>::apply_write(*m_data, offset, 
						buffer, m_elements * i, 1);
	}
}



template <typename T>
void T3DDatafield<T>::put_data_plane_xy(size_t  z, const T2DDatafield<T>& p)
{
	assert(z < get_size().z); 
	assert(p.get_size().x == get_size().x && p.get_size().y == get_size().y); 
	copy(p.begin(), p.end(), begin_at(0,0,z)); 
}

template <typename T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_yz(size_t  x)const
{
	assert(x < get_size().x); 

	C2DBounds size(get_size().y, get_size().z); 
	const size_t dx = get_size().x; 

	T2DDatafield<T> result(size); 
	typename T2DDatafield<T>::iterator r = result.begin(); 
	typename T3DDatafield<T>::const_iterator i = begin_at(x,0,0); 

	for (size_t z = 0; z < size.y; ++z) 
		for (size_t y = 0; y < size.x; ++y, ++r, i += dx) {
			*r = *i; 
		}
	return result; 
}

template <typename T>
void T3DDatafield<T>::put_data_plane_yz(size_t  x, const T2DDatafield<T>& p)
{
	assert(p.get_size().x == get_size().y && p.get_size().y == get_size().z); 
	assert(x < get_size().x); 

	const C2DBounds& size = p.get_size(); 
	const size_t dx = get_size().x; 

	typename T2DDatafield<T>::const_iterator i = p.begin(); 
	typename T3DDatafield<T>::iterator r = begin_at(x,0,0); 

	for (size_t z = 0; z < size.y; ++z) 
		for (size_t y = 0; y < size.x; ++y, ++i, r += dx) {
			*r = *i; 
		}

}

template <typename T>
T2DDatafield<T> T3DDatafield<T>::get_data_plane_xz(size_t  y)const
{
	assert(y < get_size().y); 
	
	C2DBounds size(get_size().x, get_size().z); 
	const size_t dxy = get_size().y * size.x; 

	T2DDatafield<T> result(size); 
	typename T2DDatafield<T>::iterator r = result.begin(); 
	typename T3DDatafield<T>::const_iterator i = begin_at(0, y, 0); 

	for (size_t z = 0; z < size.y; ++z, i += dxy, r += size.x) {
		copy(i, i + size.x, r);  
	}	
	return result; 
}

template <typename T>
void  T3DDatafield<T>::put_data_plane_xz(size_t  y, const T2DDatafield<T>& p)
{
	assert(y < get_size().y); 
	assert(p.get_size().x == get_size().x && p.get_size().y == get_size().z); 

	const size_t dxy = get_size().y * get_size().x; 

	const C2DBounds& size = p.get_size(); 

	T2DDatafield<T> result(size); 
	typename T3DDatafield<T>::iterator r = begin_at(0, y, 0); 
	typename T2DDatafield<T>::const_iterator i = p.begin(); 

	for (size_t z = 0; z < size.y; ++z, i += size.x, r += dxy) {
		copy(i, i + size.x, r);  
	}
}

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_avg()
{
        T Avg = T();
        // first calculate avrg.
        const_iterator i = begin();
        const_iterator e = end();
        while ( i != e ) {
                Avg += *i++;
        }
        Avg /= (m_size.x * m_size.y *m_size.z);

        return Avg;
}

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::strip_avg()
{
        make_single_ref();
        T Avg = get_avg();
        // first calculate avrg.
        iterator r = begin();
        iterator e = end();

        while ( r != e ) {
                *r++ -= Avg;
        }
        return Avg;
}

template <typename T>
void T3DDatafield<T>::clear()
{
        make_single_ref();
	std::fill(m_data->begin(), m_data->end(), T()); 
}

template <typename T>
const T T3DDatafield<T>::Zero = T();

template <typename T>
typename T3DDatafield<T>::value_type
T3DDatafield<T>::get_block_avrg(const C3DBounds& Start, const C3DBounds& BlockSize) const
{
        T Result = T(); // Not the best approach
        size_t  S = BlockSize.x * BlockSize.y * BlockSize.z;
        C3DBounds End = Start+BlockSize;
        for (size_t  z = Start.z; z < End.z; z++) {
                for (size_t  y = Start.y; y < End.y; y++) {
                        for (size_t  x = Start.x; x < End.x; x++) {
                                Result += (*this)(x,y,z);
                        }
                }
        }
        return Result / S;
}

template <typename T>
typename T3DDatafield<T>::range_iterator 
T3DDatafield<T>::begin_range(const C3DBounds& begin, const C3DBounds& end)
{
	return range_iterator(begin, get_size(), begin, end, 
			      begin_at(begin.x, begin.y, begin.z)); 
}

template <typename T>
typename T3DDatafield<T>::range_iterator 
T3DDatafield<T>::end_range(const C3DBounds& begin, const C3DBounds& end)
{
	return range_iterator(end, get_size(), begin, end, 
			      begin_at(end.x, end.y, end.z)); 
}

template <typename T>
typename T3DDatafield<T>::const_range_iterator 
T3DDatafield<T>::begin_range(const C3DBounds& begin, const C3DBounds& end)const
{
	return const_range_iterator(begin, get_size(), begin, end, 
				    begin_at(begin.x, begin.y, begin.z)); 
}

template <typename T>
typename T3DDatafield<T>::const_range_iterator 
T3DDatafield<T>::end_range(const C3DBounds& begin, const C3DBounds& end)const
{
	return const_range_iterator(end, get_size(), begin, end, 
				    begin_at(end.x, end.y, end.z)); 
}


NS_MIA_END

#endif
