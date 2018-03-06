/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef __MIA_2DDATAFIELD_HH
#define __MIA_2DDATAFIELD_HH 1


#include <vector>
#include <memory>

// MIA specific
#include <mia/2d/defines2d.hh>
#include <mia/2d/vector.hh>
#include <mia/2d/iterator.hh>
#include <mia/core/parameter.hh>
#include <mia/core/attributes.hh>
#include <mia/core/typedescr.hh>
#include <miaconfig.h>

#ifndef EXPORT_2DDATAFIELD
/// define used export 2D symbols
#  define EXPORT_2DDATAFIELD EXPORT_2D
#endif

NS_MIA_BEGIN

#define DECLARE_EXTERN_ITERATORS(TYPE)						\
	extern template class  EXPORT_2D range2d_iterator<std::vector<TYPE>::iterator>; \
	extern template class  EXPORT_2D range2d_iterator<std::vector<TYPE>::const_iterator>; \
	extern template class  EXPORT_2D range2d_iterator_with_boundary_flag<std::vector<TYPE>::iterator>; \
	extern template class  EXPORT_2D range2d_iterator_with_boundary_flag<std::vector<TYPE>::const_iterator>;

#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif

DECLARE_EXTERN_ITERATORS(double);
DECLARE_EXTERN_ITERATORS(float);
DECLARE_EXTERN_ITERATORS(uint32_t);
DECLARE_EXTERN_ITERATORS(int32_t);
DECLARE_EXTERN_ITERATORS(int16_t);
DECLARE_EXTERN_ITERATORS(uint16_t);
DECLARE_EXTERN_ITERATORS(int8_t);
DECLARE_EXTERN_ITERATORS(uint8_t);
DECLARE_EXTERN_ITERATORS(bool);
DECLARE_EXTERN_ITERATORS(int64_t);
DECLARE_EXTERN_ITERATORS(uint64_t);

DECLARE_EXTERN_ITERATORS(C2DBounds)
DECLARE_EXTERN_ITERATORS(C2DFVector)
DECLARE_EXTERN_ITERATORS(C2DDVector)

#undef DECLARE_EXTERN_ITERATORS

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


/**
   \ingroup basic
   \brief A class to hold data on a regular 2D grid

   This class is the base for all kind of 2D data that is stored over a regular grid.
   The data it hols is stored in a shared pointer.
   \tparam T the data type of the values hold at the grid points.
*/
template <class T>
class EXPORT_2DDATAFIELD T2DDatafield
{

public:

       /// type for the flat reprentation of the 2D data field
       typedef  ::std::vector<typename __holder_type_dispatch<T>::type> data_array;



       /// \cond SELFEXPLAINING
       typedef typename data_array::iterator iterator;
       typedef typename data_array::const_iterator const_iterator;
       typedef typename data_array::const_reference const_reference;
       typedef typename data_array::reference reference;
       typedef typename data_array::const_pointer const_pointer;
       typedef typename data_array::pointer pointer;
       typedef typename data_array::value_type value_type;
       typedef typename data_array::difference_type difference_type;
       typedef typename data_array::size_type size_type;
       typedef range2d_iterator<iterator> range_iterator;
       typedef range2d_iterator<const_iterator> const_range_iterator;
       typedef range2d_iterator_with_boundary_flag<iterator> range_iterator_with_boundary_flag;
       typedef range2d_iterator_with_boundary_flag<const_iterator> const_range_iterator_with_boundary_flag;



       typedef C2DBounds dimsize_type;
       typedef C2DFVector coord_type;
       /// \endcond

       class EXPORT_2DDATAFIELD Range
       {
              friend class T2DDatafield<T>;
              friend class ConstRange;
       public:

              typedef T2DDatafield<T>::range_iterator iterator;

              iterator begin();

              iterator end();

       private:
              Range(const C2DBounds& start, const C2DBounds& end, T2DDatafield<T>& field);

              iterator m_begin;
              iterator m_end;
       };

       class EXPORT_2D ConstRange
       {
       public:
              friend class T2DDatafield<T>;

              typedef T2DDatafield<T>::const_range_iterator iterator;

              iterator begin() const;

              iterator end() const;

       private:
              ConstRange(const C2DBounds& start, const C2DBounds& end, const T2DDatafield<T>& field);

              ConstRange(const Range& range);

              iterator m_begin;
              iterator m_end;
       };


       T2DDatafield();

       /**
          Create a 2D data field with the given size
          \param size
       */
       explicit T2DDatafield(const C2DBounds& size);

       /**
          Create a 2D data field with the given size and initialize it with the given data
          \param size
          \param _data must at least be of size (size.x*size.y)
       */




       T2DDatafield(const C2DBounds& size, const T *_data);

       /**
          Create a 2D data field with the given size and initialize it with the given data
          \param size
          \param data must at least be of size (size.x*size.y)
       */
       T2DDatafield(const C2DBounds& size, const std::vector<T>& data);

       /** copy constructor, it does a deep copy.

        */
       T2DDatafield(const T2DDatafield<T>& org);

       /**
          Assignment operator, does a deep copy.
       */
       T2DDatafield<T>& operator = (const T2DDatafield<T>& org);

       /** move constructor */
       T2DDatafield(T2DDatafield<T>&& org);

       /**   Assignment move operator */
       T2DDatafield<T>& operator = (T2DDatafield<T>&& org);


       virtual ~T2DDatafield();


       /**
          Since the data is internally stored by a shared pointer, this
          function ensures that the data is not shared with any other object
        */
       void make_single_ref() __attribute__((deprecated));

       /// \returns the size of the data field
       const C2DBounds&  get_size() const;

       /**
          This function sets the elements of data to T().
        */
       void clear();

       /**
          returns a read-only reference to the data element at (x,y).
          If the index is outside the image domain, the function returns the Zero element.
          \param x
          \param y
          \returns the reference
       */
       const_reference operator()(size_t  x, size_t y) const;

       /**
          returns a writable reference to the data element at (x,y).
          If the index is outside the image domain, the function throws a
          std::invalid_argument exception.
       */
       reference operator()(size_t  x, size_t  y);

       /**
          Direct index into the underlying data array  - this access is usually faster then
          calling operator()(size_t  x, size_t  y) and corresponds to
          \f$idx= x + y * m_size.x\f$ but is doesn't test whether
          the index is inside the image domain.
          \param idx
          \returns read-only reference to the data
        */
       const_reference operator[](size_t  idx) const
       {
              return m_data[idx];
       }

       /**
          Direct index into the underlying data array  - this access is usually faster then
          calling operator()(size_t  x, size_t  y) and corresponds to
          \f$idx= x + y * m_size.x\f$ but is doesn't test whether
          the index is inside the image domain.
          \param idx
          \returns read-write reference to the data
        */
       reference operator[](size_t  idx)
       {
              return m_data[idx];
       }

       /// \overload const_reference  operator()(size_t  x, size_t  y) const;
       const_reference operator()(const C2DBounds& l) const;

       /// \overload reference operator()(size_t  x, size_t  y);
       reference operator()(const C2DBounds& l);


       /**
          Read a data row from the data field to the provided buffer
          \param[in]  y row to be copied from
          \param[out] buffer to copy the data to. It will be resized to fit the data
        */
       void get_data_line_x(size_t y, std::vector<T>& buffer) const;

       /**
          Read a data column from the data field to the provided buffer
          \param[in]  x column to be copied from
          \param[out] buffer to copy the data to. It will be resized to fit the data
        */
       void get_data_line_y(size_t x, std::vector<T>& buffer) const;

       /**
          Write a data row to the data field. The function ensures that tha data is not shared
          with some other object.
          \param[in]  y row to be copied to
          \param[in]  buffer data buffer, must be of the same size as the row size of the data field
        */
       void put_data_line_x(size_t y, const std::vector<T>& buffer);

       /**
          Write a data column to the data field. The function ensures that tha data is not shared
          with some other object.
          \param[in]  x column to be copied to
          \param[in]  buffer data buffer, must be of the same size as the column size of the data field
        */
       void put_data_line_y(size_t x, const std::vector<T>& buffer);

       /// \returns the number of elements in the data field
       size_type size() const;

       /// \returns a read-only iterator to the begin of the data field with x being the fastest changing index
       const_iterator begin()const
       {
              return m_data.begin();
       }

       /// \returns a read-only iterator to the end of the data field with x being the fastest changing index
       const_iterator end()const
       {
              return m_data.end();
       }

       /** Get a read-write iterator to iterate over the whole field.
           The functions ensures that the data will only be referenced by this object.
           \returns a read-write iterator to the begin of the data field
        */
       iterator begin()
       {
              return m_data.begin();
       }

       /** Get a read-write iterator to iterate over the whole field.
           The functions ensures that the data will only be referenced by this object.
           \returns a read-write iterator to the end of the data field
        */

       iterator end()
       {
              return m_data.end();
       }

       /** Get a read-write iterator to iterate over the field staring from the given position.
           The functions ensures that the data will only be referenced by this object.
           \param x
           \param y
           \returns a read-write iterator to the given position of the data field
        */
       const_iterator begin_at(size_t x, size_t y)const
       {
              const_iterator b = begin();
              advance(b, x + y * m_size.x);
              return b;
       }

       /** Get a read-only iterator to iterate over the field staring from the given position.
           \param x
           \param y
           \returns a read-write iterator to the given position of the data field
        */
       iterator begin_at(size_t x, size_t y)
       {
              iterator b = begin();
              advance(b, x + y * m_size.x);
              return b;
       }

       Range get_range(const C2DBounds& start, const C2DBounds& end);

       ConstRange get_range(const C2DBounds& start, const C2DBounds& end) const;


       /** \returns an read/write forward iterator over a subset of the data.
           The functions ensures, that the field uses a single referenced datafield */
       range_iterator begin_range(const C2DBounds& begin, const C2DBounds& end);

       /** \returns the end of a read/write forward iterator over a subset of the data. */
       range_iterator end_range(const C2DBounds& begin, const C2DBounds& end);


       /** \returns an read/write forward iterator over a subset of the data.
           The functions ensures, that the field uses a single referenced datafield */
       const_range_iterator begin_range(const C2DBounds& begin, const C2DBounds& end)const;

       /** \returns the end of a read/write forward iterator over a subset of the data. */
       const_range_iterator end_range(const C2DBounds& begin, const C2DBounds& end)const;


private:
       C2DBounds  m_size;
       data_array m_data;
       const static value_type Zero;
};

/// 2D scalar field that holds double values
typedef T2DDatafield<double> C2DDDatafield;

/// 2D scalar field that holds float values
typedef T2DDatafield<float>  C2DFDatafield;

/// 2D scalar field that holds unsigned int values
typedef T2DDatafield<uint32_t> C2DUIDatafield;

/// 2D scalar field that holds signed int values
typedef T2DDatafield<int32_t>  C2DSIDatafield;

typedef T2DDatafield<uint64_t> C2DULDatafield;

/// long  instanziation of a 2D data field
typedef T2DDatafield<int64_t>  C2DSLDatafield;

/// 2D scalar field that holds unsigned short values
typedef T2DDatafield<uint16_t> C2DUSDatafield;

/// 2D scalar field that holds signed short values
typedef T2DDatafield<int16_t>  C2DSSDatafield;

/// 2D scalar field that holds unsigned char (=byte) values
typedef T2DDatafield<uint8_t> C2DUBDatafield;

/// 2D scalar field that holds signed char values
typedef T2DDatafield<int8_t>  C2DSBDatafield;

/// 2D scalar field that holds bool values
typedef T2DDatafield<bool>  C2DBitDatafield;

/// Parameter type for 2D size definitions
typedef  CTParameter<C2DBounds> C2DBoundsParameter;

/// Parameter type for 2D vector
typedef CTParameter<C2DFVector> C2DFVectorParameter;

/// typedef for the C2DFVector to std::string translator
typedef TTranslator<C2DFVector> C2DFVectorTranslator;

/// @cond NEVER

#define DEFINE_2DFIELD_TEMPLATE(TYPE) \
	extern template class EXPORT_2D T2DDatafield<TYPE>;			\
	extern template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	extern template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::const_iterator>; \
	extern template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::iterator>; \
	extern template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::const_iterator>;


#ifdef __GNUC__
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wattributes"
#endif
#endif


DEFINE_2DFIELD_TEMPLATE(double);
DEFINE_2DFIELD_TEMPLATE(float);
DEFINE_2DFIELD_TEMPLATE(int64_t);
DEFINE_2DFIELD_TEMPLATE(uint64_t);
DEFINE_2DFIELD_TEMPLATE(uint32_t);
DEFINE_2DFIELD_TEMPLATE(int32_t);
DEFINE_2DFIELD_TEMPLATE(uint16_t);
DEFINE_2DFIELD_TEMPLATE(int16_t);
DEFINE_2DFIELD_TEMPLATE(uint8_t);
DEFINE_2DFIELD_TEMPLATE(int8_t);

DEFINE_2DFIELD_TEMPLATE(C2DBounds);
DEFINE_2DFIELD_TEMPLATE(C2DFVector)
DEFINE_2DFIELD_TEMPLATE(C2DDVector)

DECLARE_TYPE_DESCR(C2DBounds);
DECLARE_TYPE_DESCR(C2DFVector);
DECLARE_TYPE_DESCR(C2DDVector);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


extern template class EXPORT_2D CTParameter<C2DFVector>;
extern template class EXPORT_2D CTParameter<C2DBounds>;
extern template class EXPORT_2D TTranslator<C2DFVector>;
extern template class EXPORT_2D TAttribute<C2DFVector>;

/// @endcond

NS_MIA_END

#endif

