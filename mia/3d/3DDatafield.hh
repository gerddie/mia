/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * Gert Wollny <gert.wollny at web.de>
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

// $Id: 3DDatafield.hh 831 2006-02-06 18:32:09Z write1 $

/*! \brief A templated datafield class

The class holds all types of data stored in three dimensional fields.

\file 3DDatafield.hh
\author Gert Wollny <wollny@cbs.mpg.de>

*/
#ifndef __MIA_3DDATAFIELD_HH
#define __MIA_3DDATAFIELD_HH 1

#include <cstdio>
#include <vector>
#include <cmath>
#include <cassert>

#include <mia/core/shared_ptr.hh>

#include <mia/3d/3DVector.hh>
#include <mia/3d/defines3d.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

/**
   A templated class of a 3D data field.
*/
template <class T>
class  EXPORT_3D T3DDatafield {

        typedef std::shared_ptr<std::vector<T>  >  ref_data_type;

        /** Size of the field */
        C3DBounds  _M_size;

        /** helper: product of Size.x * Size.y */
        long int  _M_xy;

        /** Pointer to the Field of Data hold by this class */
        ref_data_type _M_data;

        /** helper: represents the zero-value */
        const static T Zero;


public:

        /** makes a single reference of the data, after calling this, it is save to write to the data field
         */
        void make_single_ref();

	/// a shortcut data type
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
        typedef typename std::vector<T>::const_reference const_reference;
        typedef typename std::vector<T>::reference reference;
        typedef typename std::vector<T>::const_pointer const_pointer;
        typedef typename std::vector<T>::pointer pointer;
        typedef typename std::vector<T>::value_type value_type;
        typedef typename std::vector<T>::size_type size_type;
        typedef typename std::vector<T>::difference_type difference_type;
	typedef C3DBounds dimsize_type;

	T3DDatafield();

        /** Constructor to create empty Datafield if given size */
        T3DDatafield(const C3DBounds& _Size);

        /** Constructor to create Datafield if given size and with initialization data
            \param size the size of the 3D-field
            \param data to use for initialization
         */
        T3DDatafield(const C3DBounds& size, const T *data);


        /** copy - Constructor */
        T3DDatafield(const T3DDatafield<T>& org);

        /// make sure the destructor is virtual
        virtual ~T3DDatafield();

        /**
            Gradient calculation using tri-linear interpolation
            \param p position where to evaluate the gradient
        */
	template <typename Out>
        T3DVector<Out> get_gradient(const T3DVector<float >& p) const;

        /** calculate gradient of data field at a grid point */
        template <typename Out>
	T3DVector<Out> get_gradient(size_t  x, size_t  y, size_t  z) const;

        /** calculate the gradient at a grid point given by a linear location */
        template <typename Out>
	T3DVector<Out> get_gradient(int index) const;

        /** Interpolate the value of Field at p default uses tri-linear interpolation */
        value_type get_interpol_val_at(const T3DVector<float >& p) const;

        /* some rough interpolation using barycentric coordinates, needs less addition and
            multiplications then tri-linear interp. but is usally of low quality
            \remark this function may vanish
        value_type get_barycent_interpol_val_at(const T3DVector<float >& p) const;
	*/

        /** just as the name says */
        value_type get_trilin_interpol_val_at(const T3DVector<float >& p) const;

        /** Get the average over a given Block
         Attn: Type T must be able to hold the Sum of all Elements in Block */
        value_type get_block_avrg(const C3DBounds& Start, const C3DBounds& BlockSize) const;

        /** Assignment operator  -
            \remark it just copys a pointer to the data and increases its reference count,
            before writing it is necesary to call \a make_single_ref
        */
        T3DDatafield& operator = (const T3DDatafield& org);

        /** \returns the 3D-size of the data field */
        const C3DBounds&  get_size() const
        {
                return _M_size;
        }

        /** Set alle elements of the field to T() == Zero*/
        void clear();

        /** \returns the number of elements in the datafield */
        size_type size()const
        {
                return _M_data->size();
        }

	void swap(T3DDatafield& other);

        /** \returns the average over the whole datafield*/
        value_type get_avg();

        /** Strip average from data
         \returns the stripped average */
        value_type strip_avg();


        /** interpolating access operator */
        value_type operator()(const T3DVector<float >& pos)const;

        /** read-only indx operator */
        const_reference operator()(size_t  x, size_t  y, size_t  z) const
	{
        	// Look if we are inside, and give reference, else give the zero
	        if (x < _M_size.x && y < _M_size.y && z < _M_size.z) {
	                const std::vector<T>& data = *_M_data;
	                return data[x+ _M_size.x * (y  + _M_size.y * z)];
	        }
		return Zero;
	}


        /** alternate read-only indx operator */
        const_reference operator()(const C3DBounds& l)const
        {
                return (*this)(l.x,l.y,l.z);
        }

        /** Index operator witch gives write access */
        reference operator()(size_t  x, size_t  y, size_t  z)
	{
        	// Look if we are inside, and give reference, else throw exception
	        // since write access is wanted
	        assert(x < _M_size.x && y < _M_size.y && z < _M_size.z);
		return (*_M_data)[x + _M_size.x *(y + _M_size.y * z)];
	}



        /** Alternate index operator witch gives write access */
        reference operator()(const C3DBounds& l)
        {
                return (*this)(l.x,l.y,l.z);
        }

        /** Get some Data along some line parallel to X axis */
        void get_data_line_x(int y, int z, std::vector<T>& buffer)const;

        /** Get some Data along some line parallel to Y axis */
        void get_data_line_y(int x, int z, std::vector<T>& buffer)const;

        /** Get some Data along some line parallel to Z axis */
        void get_data_line_z(int x, int y, std::vector<T>& buffer)const;

        /** Put some Data along some line parallel to X axis */
        void put_data_line_x(int y, int z, const std::vector<T> &buffer);

        /** Put some Data along some line parallel to Y axis */
        void put_data_line_y(int x, int z, const std::vector<T> &buffer);

        /** Put some Data along some line parallel to Z axis */
        void put_data_line_z(int x, int y, const std::vector<T> &buffer);

        /** Mask the data field with a given mask */
        template <class TMask>
        void mask(const TMask& m);

	T2DDatafield<T> get_data_plane_xy(size_t  z)const;

        T2DDatafield<T> get_data_plane_yz(size_t  x)const;

        T2DDatafield<T> get_data_plane_xz(size_t  y)const;

	void put_data_plane_xy(size_t  z, const T2DDatafield<T>& p);

        void put_data_plane_yz(size_t  x, const T2DDatafield<T>& p);

        void put_data_plane_xz(size_t  y, const T2DDatafield<T>& p);

        /** \returns an read only forward iterator over the whole data field */
        const_iterator begin()const
        {
                return _M_data->begin();
        }

	const_iterator begin_at(size_t x, size_t y, size_t z)const
        {
                return _M_data->begin() + (z * _M_size.y + y) * _M_size.x + x;
        }


        const_iterator end()const
        {
                return _M_data->end();
        }

        /** \returns an read/write random access iterator over the whole data
            field pointing at the beginning of the data.
            The functions ensures, that the field uses a single referenced datafield */
        iterator begin()
        {
                make_single_ref();
                return _M_data->begin();
        }

	iterator begin_at(size_t x, size_t y, size_t z)
        {
		make_single_ref();
		return _M_data->begin() + (z * _M_size.y + y) * _M_size.x + x;
        }

	/** \returns an read/write random access iterator over the whole data
            field pointing at the end of the data.
            The functions ensures, that the field uses a single referenced datafield */
        iterator end()
        {
                make_single_ref();
                return _M_data->end();
        }

        /** a linear read only access operator */
        const_reference operator[](int i)const
        {
                return (*_M_data)[i];
        }

        /** A linear read/write access operator. The refcount of Data must be 1,
            else the program will abort with a failed assertion (if assert is enabled)
        */
        reference operator[](int i)
        {
		assert(_M_data.unique());
                return (*_M_data)[i];
        }


        /** \returns the element count of one z slice */
        size_t  get_plane_size_xy()const
        {
                return _M_xy;
        };

private:
};


/// a data field of float values
typedef T3DDatafield<float>  C3DFDatafield;

/// a data field of 32 bit unsigned int values
typedef T3DDatafield<unsigned int> C3DUIDatafield;

/// a data field of 32 bit signed int values
typedef T3DDatafield<int>  C3DIDatafield;


/// a data field of 32 bit unsigned int values
typedef T3DDatafield<unsigned long> C3DULDatafield;

/// a data field of 32 bit signed int values
typedef T3DDatafield<long>  C3DLDatafield;

	/// a data field of float values
typedef T3DDatafield<unsigned char>  C3DUBDatafield;

	/// a data field of float values
typedef T3DDatafield<bool>  C3DBitDatafield;


// some implementations

template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(size_t  x, size_t  y, size_t  z) const
{
	const std::vector<T>& data = *_M_data;
	const int sizex = _M_size.x;
	// Look if we are inside the used space
	if (x - 1 < _M_size.x - 2 &&  y - 1 < _M_size.y - 2 &&  z - 1 < _M_size.z - 2) {

                // Lookup all neccessary Values
		const T *H  = &data[x + _M_size.x * (y + _M_size.y * z)];

		return T3DVector<Out> (Out((H[1] - H[-1]) * 0.5),
				       Out((H[sizex] - H[-sizex]) * 0.5),
				       Out((H[_M_xy] - H[-_M_xy]) * 0.5));
	}

	return T3DVector<Out>();
}

template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(int hardcode) const
{
	const int sizex = _M_size.x;
	// Lookup all neccessary Values
	const T *H  = &(*_M_data)[hardcode];

	return T3DVector<Out> (Out((H[1] - H[-1]) * 0.5),
			       Out((H[sizex] - H[-sizex]) * 0.5),
			       Out((H[_M_xy] - H[-_M_xy]) * 0.5));
}


template <>
template <typename Out>
T3DVector<Out> T3DDatafield<bool>::get_gradient(int hardcode) const
{

	// Lookup all neccessary Values
	return T3DVector<Out> (Out(((*_M_data)[hardcode + 1] - (*_M_data)[hardcode -1]) * 0.5),
			       Out(((*_M_data)[hardcode + _M_size.x] - (*_M_data)[hardcode -_M_size.x]) * 0.5),
			       Out(((*_M_data)[hardcode + _M_xy] - (*_M_data)[hardcode -_M_xy]) * 0.5));
}

template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(const T3DVector<float >& p) const
{
        // This will become really funny
	const int sizex = _M_size.x;
        const std::vector<T>& data = *_M_data;
        // Calculate the int coordinates near the POI
        // and the distances
        size_t  x = size_t (p.x);
        float  dx = p.x - x;
        float  xm = 1 - dx;
        size_t  y = size_t (p.y);
        float  dy = p.y - y;
        float  ym = 1 - dy;
        size_t  z = size_t (p.z);
        float  dz = p.z - z;
        float  zm = 1 - dz;

	// Look if we are inside the used space
        if (x-1 < _M_size.x-3 &&  y -1 < _M_size.y-3 && z - 1 < _M_size.z-3 ) {
                // Lookup all neccessary Values
                const T *H000  = &data[x + sizex * y + _M_xy * z];

                const T* H_100 = &H000[-_M_xy];
                const T* H_101 = &H_100[1];
                const T* H_110 = &H_100[sizex];
                const T* H_111 = &H_110[1];

                const T* H0_10 = &H000[-sizex];
                const T* H0_11 = &H0_10[1];

                const T* H00_1 = &H000[-1];
                const T* H001  = &H000[ 1];
                const T* H002  = &H000[ 2];


                const T* H010  = &H000[sizex];
                const T* H011  = &H010[ 1];
                const T* H012  = &H010[ 2];
                const T* H01_1 = &H010[-1];

                const T* H020 = &H010[sizex];
                const T* H021 = &H020[ 1];

                const T* H100 = &H000[_M_xy];

                const T* H1_10 = &H100[sizex];
                const T* H1_11 = &H1_10[1];

                const T* H10_1 = &H100[-1];
                const T* H101  = &H100[ 1];
                const T* H102  = &H100[ 2];

                const T* H110  = &H100[sizex];
                const T* H111  = &H110[ 1];
                const T* H112  = &H110[ 2];
                const T* H11_1 = &H110[-1];

                const T* H120 = &H110[sizex];
                const T* H121 = &H120[ 1];

                const T* H200 = &H100[_M_xy];
                const T* H201 = &H200[1];
                const T* H210 = &H200[sizex];
                const T* H211 = &H210[1];

                // use trilinear interpolation to calc the gradient
                return T3DVector<Out> (
			Out((zm * (ym * (dx * (*H002 - *H000) + xm * (*H001 - *H00_1))+
				   dy * (dx * (*H012 - *H010) + xm * (*H011 - *H01_1)))+
			     dz * (ym * (dx * (*H102 - *H100) + xm * (*H101 - *H10_1))+
				   dy * (dx * (*H112 - *H110) + xm * (*H111 - *H11_1)))) * 0.5),

			Out((zm * (ym * (xm * (*H010 - *H0_10) + dx * (*H011 - *H0_11))+
				   dy * (xm * (*H020 - *H000)  + dx * (*H021 - *H001)))+
			     dz * (ym * (xm * (*H110 - *H1_10) + dx * (*H111 - *H1_11))+
				   dy * (xm * (*H120 - *H100)  + dx * (*H121 - *H101)))) * 0.5),
			Out((zm * (ym * (xm * (*H100 - *H_100) + dx * (*H101 - *H_101))+
				   dy * (xm * (*H110 - *H_110) + dx * (*H111 - *H_111)))+
			     dz * (ym * (xm * (*H200 - *H000)  + dx * (*H201 - *H001))+
				   dy * (xm * (*H210 - *H010)  + dx * (*H211 - *H011)))) * 0.5));
        }
        return T3DVector<Out>();

}

#ifndef mia3d_EXPORTS

extern template class  EXPORT_3D T3DDatafield<double>;
extern template class  EXPORT_3D T3DDatafield<float>;
extern template class  EXPORT_3D T3DDatafield<unsigned int>;
extern template class  EXPORT_3D T3DDatafield<int>;

#ifdef HAVE_INT64
extern template class  EXPORT_3D T3DDatafield<mia_uint64>;
extern template class  EXPORT_3D T3DDatafield<mia_int64>;
#endif
extern template class  EXPORT_3D T3DDatafield<short>;
extern template class  EXPORT_3D T3DDatafield<unsigned short>;
extern template class  EXPORT_3D T3DDatafield<unsigned char >;
extern template class  EXPORT_3D T3DDatafield<signed char >;
extern template class  T3DDatafield<bool>;

extern template class EXPORT_3D T2DDatafield<T3DVector<double> >;
extern template class EXPORT_3D T2DDatafield<T3DVector<float> >;

#endif

NS_MIA_END

#endif
