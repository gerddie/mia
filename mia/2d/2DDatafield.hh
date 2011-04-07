/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// $Id: 2DDatafield.hh 921 2006-06-22 10:43:30Z write1 $

/*! \brief A templated 2D datafield class

The class holds all types of data stored in two dimensional fields.

\file 2DDatafield.hh
\author Gert Wollny <wollny@cbs.mpg.de>, 2004

*/

#ifndef __MIA_2DDATAFIELD_HH
#define __MIA_2DDATAFIELD_HH 1


#include <vector>
#include <mia/core/shared_ptr.hh>

// MIA specific
#include <mia/2d/defines2d.hh>
#include <mia/2d/2DVector.hh>
#include <mia/core/parameter.hh>

#ifndef EXPORT_2DDATAFIELD
#  define EXPORT_2DDATAFIELD EXPORT_2D
#endif

NS_MIA_BEGIN



template <class T>
class EXPORT_2DDATAFIELD T2DDatafield  {

public:
	typedef  ::std::vector<T> data_array;
	typedef  std::shared_ptr<data_array > data_pointer;

	/// a shortcut data type
	typedef typename data_array::iterator iterator;
	typedef typename data_array::const_iterator const_iterator;
	typedef typename data_array::const_reference const_reference;
	typedef typename data_array::reference reference;
	typedef typename data_array::const_pointer const_pointer;
	typedef typename data_array::pointer pointer;
	typedef typename data_array::value_type value_type;
	typedef typename data_array::difference_type difference_type;
	typedef typename data_array::size_type size_type;
	typedef C2DBounds dimsize_type;
	typedef C2DFVector coord_type;

	T2DDatafield();

	T2DDatafield(const C2DBounds& _m_size);

	T2DDatafield(const C2DBounds& _m_size, const T *_data);

	T2DDatafield(const C2DBounds& _m_size, const data_array& data);

	T2DDatafield(const T2DDatafield<T>& org);

	virtual ~T2DDatafield();

	T get_interpol_val_at(const C2DFVector& p) const;

	void make_single_ref();

	T2DDatafield& operator = (const T2DDatafield& org);

	const C2DBounds&  get_size() const;

	void clear();

	const_reference operator()(size_t  x, size_t  y) const;

	reference operator()(size_t  x, size_t  y);

	const_reference operator[](size_t  idx) const{
			return (*m_data)[idx];
	}

	reference operator[](size_t  idx){
			return (*m_data)[idx];
	}

	const_reference operator()(const C2DBounds& l) const;

	reference operator()(const C2DBounds& l);

	void get_data_line_x(size_t y, std::vector<T>& buffer) const;

	void get_data_line_y(size_t x, std::vector<T>& buffer) const;

	void put_data_line_x(size_t y, const std::vector<T>& buffer);

	void put_data_line_y(size_t x, const std::vector<T>& buffer);

	size_type size() const;

	const_iterator begin()const {
		const data_array& data = *m_data;
		return data.begin();
	}

	const_iterator end()const {
		const data_array& data = *m_data;
		return data.end();
	}

	iterator begin() {
		make_single_ref();
		return m_data->begin();
	}

	iterator end() {
		make_single_ref();
		return m_data->end();
	}

	const_iterator begin_at(size_t x, size_t y)const {
		const_iterator b = begin();
		advance(b, x + y * m_size.x);
		return b;
	}

	iterator begin_at(size_t x, size_t y) {
		iterator b = begin();
		advance(b, x + y * m_size.x);
		return b;
	}
private:
	C2DBounds  m_size;
	data_pointer m_data;
	const static T Zero;
};

/// 2D scalar field that holds double values 
typedef T2DDatafield<double> C2DDDatafield;

/// 2D scalar field that holds float values 
typedef T2DDatafield<float>  C2DFDatafield;

/// 2D scalar field that holds unsigned int values 
typedef T2DDatafield<unsigned int> C2DUIDatafield;

/// 2D scalar field that holds signed int values 
typedef T2DDatafield<signed int>  C2DSIDatafield;

#ifdef HAVE_INT64
/// 
typedef T2DDatafield<mia_uint64> C2DULDatafield;

/// long  instanziation of a 2D data field
typedef T2DDatafield<mia_int64>  C2DSLDatafield;
#endif

/// 2D scalar field that holds unsigned short values 
typedef T2DDatafield<unsigned short> C2DUSDatafield;

/// 2D scalar field that holds signed short values 
typedef T2DDatafield<signed short>  C2DSSDatafield;

/// 2D scalar field that holds unsigned char (=byte) values 
typedef T2DDatafield<unsigned char> C2DUBDatafield;

/// 2D scalar field that holds signed char values 
typedef T2DDatafield<signed char>  C2DSBDatafield;

/// 2D scalar field that holds bool values
typedef T2DDatafield<bool>  C2DBitDatafield;


extern const char type_str_2dbounds[]; 
typedef  CTParameter<C2DBounds, type_str_2dbounds> C2DBoundsParameter;

NS_MIA_END

#endif

