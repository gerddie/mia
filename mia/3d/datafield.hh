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

#ifndef __MIA_3DDATAFIELD_HH
#define __MIA_3DDATAFIELD_HH 1

#include <cstdio>
#include <vector>
#include <cmath>
#include <cassert>

#include <mia/3d/vector.hh>
#include <mia/3d/defines3d.hh>
#include <mia/3d/iterator.hh>
#include <mia/2d/datafield.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/typedescr.hh>
#include <miaconfig.h>

NS_MIA_BEGIN

/**
   @ingroup basic 
   \brief A templated class of a 3D data field.
*/
template <class T>
class  EXPORT_3D T3DDatafield {

        typedef std::shared_ptr<std::vector<T>  >  ref_data_type;

        /** Size of the field */
        C3DBounds  m_size;

        /** helper: product of Size.x * Size.y */
        size_t  m_xy;

        /** Pointer to the Field of Data hold by this class */
        ref_data_type m_data;

        /** helper: represents the zero-value */
        static const T Zero;
	
	static const unsigned int m_elements; 

public:

        /** makes a single reference of the data, after calling this, it is save to write to the data field
         */
        void make_single_ref();

	/**
	   Checks whether the data hold by the data field is unique. 
	   \returns true if it is 
	 */
	bool holds_unique_data()const { 
		return m_data.unique(); 
	}
			

	/// a shortcut data type

	/// \cond SELFEXPLAINING 
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;
        typedef typename std::vector<T>::const_reference const_reference;
        typedef typename std::vector<T>::reference reference;
        typedef typename std::vector<T>::const_pointer const_pointer;
        typedef typename std::vector<T>::pointer pointer;
        typedef typename std::vector<T>::value_type value_type;
        typedef typename std::vector<T>::size_type size_type;
        typedef typename std::vector<T>::difference_type difference_type;
	typedef typename atomic_data<T>::type atomic_type; 
	typedef range3d_iterator<iterator> range_iterator; 
	typedef range3d_iterator<const_iterator> const_range_iterator; 
	typedef C3DBounds dimsize_type;
	/// \endcond 

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
                return m_size;
        }

        /** Set alle elements of the field to T() == Zero*/
        void clear();

        /** \returns the number of elements in the datafield */
        size_type size()const
        {
                return m_data->size();
        }

	/// swap the data ofthis 3DDatafield with another one 
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
	        if (x < m_size.x && y < m_size.y && z < m_size.z) {
	                const std::vector<T>& data = *m_data;
	                return data[x+ m_size.x * (y  + m_size.y * z)];
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
	        assert(x < m_size.x && y < m_size.y && z < m_size.z);
		return (*m_data)[x + m_size.x *(y + m_size.y * z)];
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

	/**
	   Read the a x-slice of the data field into a flat buffer - i.e. the 
           information about multi-dimensionality of the elements is lost. 
	   For this to work, T has to be a POD-like data type, i.e., it has no 
	   hidden elements like a virtual methods table, and, if T is a type 
	   of more then one element, all these elements have to be of the same 
	   type. Specifically, a specialization of the trait atomic_data for T 
	   must exists. 
	   \param x slice to be read 
	   \param[out] buffer Buffer where the data will be written to. It must 
	   large enough to hold size.y * size.z * number of elements 
	   
	*/
	void read_xslice_flat(size_t x, std::vector<atomic_type>& buffer) const;

	/**
	   Read the a y-slice of the data field into a flat buffer - i.e. the 
           information about multi-dimensionality of the elements is lost. 
	   For this to work, T has to be a POD-like data type, i.e., it has no 
	   hidden elements like a virtual methods table, and, if T is a type 
	   of more then one element, all these elements have to be of the same 
	   type. Specifically, a specialization of the trait atomic_data for T 
	   must exists. 
	   \param y slice to be read 
	   \param[out] buffer Buffer where the data will be written to. It must 
	   large enough to hold size.x * size.z * number of elements 
	*/
	void read_yslice_flat(size_t y, std::vector<atomic_type>& buffer) const;
	
	/**
	   Read the a z-slice of the data field into a flat buffer - i.e. the 
           information about multi-dimensionality of the elements is lost. 
	   For this to work, T has to be a POD-like data type, i.e., it has no 
	   hidden elements like a virtual methods table, and, if T is a type 
	   of more then one element, all these elements have to be of the same 
	   type. Specifically, a specialization of the trait atomic_data for T 
	   must exists. 
	   \param z slice to be read 
	   \param[out] buffer Buffer where the data will be written to. It must 
	   large enough to hold size.x * size.y * number of elements 
	*/
	void read_zslice_flat(size_t z, std::vector<atomic_type>& buffer) const;
	
	/**
	   Write a z-slice from a flat buffer to the 3D data field. For details see 
	   void read_zslice_flat(size_t z, std::vector<atomic_type>& buffer) const;
	 */
	void write_zslice_flat(size_t z, const std::vector<atomic_type>& buffer); 


	/**
	   Write a y-slice from a flat buffer to the 3D data field. For details see 
	   void read_yslice_flat(size_t y, std::vector<atomic_type>& buffer) const;
	 */
	void write_yslice_flat(size_t y, const std::vector<atomic_type>& buffer); 

	/**
	   Write a x-slice from a flat buffer to the 3D data field. For details see 
	   void read_yslice_flat(size_t x, std::vector<atomic_type>& buffer) const;
	*/
	void write_xslice_flat(size_t x, const std::vector<atomic_type>& buffer); 

	/**
	   Read a z-plane from the 3D data set.
	   \param z
	   \returns the copied data in a 2D data field 
	*/
	T2DDatafield<T> get_data_plane_xy(size_t  z)const;
	
	/**
	   Read a x-plane from the 3D data set.
	   \param x
	   \returns the copied data in a 2D data field 
	*/
	T2DDatafield<T> get_data_plane_yz(size_t  x)const;

	/**
	   Read a y-plane from the 3D data set.
	   \param y
	   \returns the copied data in a 2D data field 
	*/
	T2DDatafield<T> get_data_plane_xz(size_t  y)const;

	/**
	   write a z-plane to the 3D data set.
	   \param z
	   \param p plane data, must be of dimensions (size.x, size.y)
	*/
	void put_data_plane_xy(size_t  z, const T2DDatafield<T>& p);

	/**
	   write a x-plane to the 3D data set.
	   \param x
	   \param p plane data, must be of dimensions (size.y, size.z)
	*/
        void put_data_plane_yz(size_t  x, const T2DDatafield<T>& p);

	/**
	   write a y-plane to the 3D data set.
	   \param y
	   \param p plane data, must be of dimensions (size.x, size.z)
	*/
        void put_data_plane_xz(size_t  y, const T2DDatafield<T>& p);

        /** \returns an read only forward iterator over the whole data field */
        const_iterator begin()const
        {
                return m_data->begin();
        }
	
	/**
	   \returns an read only forward iterator over data field starting at (x,y,z)
	 */
	const_iterator begin_at(size_t x, size_t y, size_t z)const
        {
                return m_data->begin() + (z * m_size.y + y) * m_size.x + x;
        }


	/**
	   \returns the end iterator to the 3D data field 
	 */
        const_iterator end()const
        {
                return m_data->end();
        }

        /** \returns an read/write random access iterator over the whole data
            field pointing at the beginning of the data.
            The functions ensures, that the field uses a single referenced datafield */
        iterator begin()
        {
                make_single_ref();
                return m_data->begin();
        }

        /** \returns an read/write forward iterator over a subset of the data. 
            The functions ensures, that the field uses a single referenced datafield */
        range_iterator begin_range(const C3DBounds& begin, const C3DBounds& end); 

        /** \returns the end of a read/write forward iterator over a subset of the data. */
        range_iterator end_range(const C3DBounds& begin, const C3DBounds& end); 


        /** \returns an read/write forward iterator over a subset of the data. 
            The functions ensures, that the field uses a single referenced datafield */
        const_range_iterator begin_range(const C3DBounds& begin, const C3DBounds& end)const; 

        /** \returns the end of a read/write forward iterator over a subset of the data. */
        const_range_iterator end_range(const C3DBounds& begin, const C3DBounds& end)const; 


	/**
	   Obtain an iterator at position (x,y,z)
	   The functions ensures, that the field uses a single referenced datafield
	   \param x
	   \param y
	   \param z
	   \returns the iterator 
	 */
	iterator begin_at(size_t x, size_t y, size_t z)
        {
		make_single_ref();
		return m_data->begin() + (z * m_size.y + y) * m_size.x + x;
        }

	/** \returns an read/write random access iterator over the whole data
            field pointing at the end of the data.
            The functions ensures, that the field uses a single referenced datafield */
        iterator end()
        {
                make_single_ref();
                return m_data->end();
        }

        /** a linear read only access operator */
        const_reference operator[](int i)const
        {
                return (*m_data)[i];
        }

        /** A linear read/write access operator. The refcount of Data must be 1,
            else the program will abort with a failed assertion (if assert is enabled)
        */
        reference operator[](int i)
        {
		assert(m_data.unique());
                return (*m_data)[i];
        }


        /** \returns the element count of one z slice */
        size_t  get_plane_size_xy()const
        {
                return m_xy;
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

/// 3D size parameter type 
typedef  CTParameter<C3DBounds> C3DBoundsParameter;

/// 3D vector parameter type 
typedef  CTParameter<C3DFVector> C3DFVectorParameter;

/// @cond NEVER 
DECLARE_TYPE_DESCR(C3DBounds); 
DECLARE_TYPE_DESCR(C3DFVector); 
/// @endcond 

// some implementations

template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(size_t  x, size_t  y, size_t  z) const
{
	const std::vector<T>& data = *m_data;
	const int sizex = m_size.x;
	// Look if we are inside the used space
	if (x - 1 < m_size.x - 2 &&  y - 1 < m_size.y - 2 &&  z - 1 < m_size.z - 2) {

                // Lookup all neccessary Values
		const T *H  = &data[x + m_size.x * (y + m_size.y * z)];

		return T3DVector<Out> (Out((H[1] - H[-1]) * 0.5),
				       Out((H[sizex] - H[-sizex]) * 0.5),
				       Out((H[m_xy] - H[-m_xy]) * 0.5));
	}

	return T3DVector<Out>();
}


template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(int hardcode) const
{
	const int sizex = m_size.x;
	// Lookup all neccessary Values
	const T *H  = &(*m_data)[hardcode];

	return T3DVector<Out> (Out((H[1] - H[-1]) * 0.5),
			       Out((H[sizex] - H[-sizex]) * 0.5),
			       Out((H[m_xy] - H[-m_xy]) * 0.5));
}


/**
   Specialization to handle the wired std::vector<bool> implementation 
 */
template <>
template <typename Out>
T3DVector<Out> T3DDatafield<bool>::get_gradient(int hardcode) const
{

	// Lookup all neccessary Values
	return T3DVector<Out> (Out(((*m_data)[hardcode + 1] - (*m_data)[hardcode -1]) * 0.5),
			       Out(((*m_data)[hardcode + m_size.x] - (*m_data)[hardcode -m_size.x]) * 0.5),
			       Out(((*m_data)[hardcode + m_xy] - (*m_data)[hardcode -m_xy]) * 0.5));
}

template <class T>
template <typename Out>
T3DVector<Out> T3DDatafield<T>::get_gradient(const T3DVector<float >& p) const
{
        // This will become really funny
	const int sizex = m_size.x;
        const std::vector<T>& data = *m_data;
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
        if (x-1 < m_size.x-3 &&  y -1 < m_size.y-3 && z - 1 < m_size.z-3 ) {
                // Lookup all neccessary Values
                const T *H000  = &data[x + sizex * y + m_xy * z];

                const T* H_100 = &H000[-m_xy];
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

                const T* H100 = &H000[m_xy];

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

                const T* H200 = &H100[m_xy];
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

NS_MIA_END

#endif
