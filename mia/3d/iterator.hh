/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_iterator_hh
#define mia_3d_iterator_hh

#include <mia/3d/vector.hh>

NS_MIA_BEGIN

/**
   @ingroup basic 
   \brief a 3D iterator that knows its position in the 3D grid ans supports iterating over 
   sub-ranges 
   

   Iterator to iterate over a sub-range of 3D data that is given on a grid. 
   Two iterators are considered to be equal, if their positions are equal.  
   \tparam the internal iterator that is used to iterate of the original 
   grid without skipping. 
   
 */

template <typename I> 
class range3d_iterator: public std::forward_iterator_tag {
public: 
	/// data type reference 
	typedef typename I::reference reference; 
	/// data type pointer  
	typedef typename I::pointer pointer; 
	/// data type for the real iterator in the background 
	typedef I internal_iterator; 
	
	/**
	   Enumerate to describe the various positions on the domain boundarys. 
	   These boundaries correspond to the full domain of the data, not to 
           the sub-range this iteratior works on. 
	   I.e. if the sub-range is a subset of the \a open domain (i.e. without its boundary) 
	   then the iterator will never touch the domain boundary. 
	 */
	enum EBoundary {
		eb_none  = 0, /**< no boundary */
		eb_xlow  = 1, /**< at low x-boundary */ 
		eb_xhigh = 2, /**< at high x-boundary */  
		eb_x     = 3, /**< at one of the x-boundaries */  
		eb_ylow = 4,  /**< at low y-boundary */ 
		eb_yhigh = 8, /**< at high y-boundary */
		eb_y     = 0xC, /**< at one of the y-boundaries */  
		eb_zlow = 0x10, /**< at low x-boundary */ 
		eb_zhigh = 0x20,/**< at high z-boundary */
		eb_z     = 0x30 /**< at one of the z-boundaries */  
	}; 
	

	
	/** standard constructor */
	range3d_iterator(); 

	/**
	   Full constructor of the range iterator 
	   @param pos iterator position to initialize the iterator with 
	   @param size size of the original data field 
	   @param start start of the iterator range 
	   @param end end of the iterator range 
	   @param iterator the iterator of the underlying 3D data structure 
	*/
	range3d_iterator(const C3DBounds& pos, const C3DBounds& size, 
			 const C3DBounds& start, const C3DBounds& end, I iterator);
	
	/**
	   End iterator, can't be dereferenced  
	   This iterator is only there to define the end position of the range_iterator. 
	   \param pos end position to set this iterator to. 
	 */
	range3d_iterator(const C3DBounds& pos);

	/// assignment operator 
	range3d_iterator<I>& operator = (const range3d_iterator<I>& other); 
	
	/// copy constructore 
	range3d_iterator(const range3d_iterator<I>& other); 

	/// friend iterator type because we may want to copy a iterator to a const_iterator. 
	template <typename AI> 
	friend class range3d_iterator; 
	
	/**
	   Constructor to construct the iterator  from one that is based on another 
	   iterator type. The usual idea is that a iterator may be converted  into it's const variant. 
	   \tparam AI the other iterator type. Iterator type I must be copy-constructable from 
	   type AI 
	   \param other 
	 */
	template <typename AI>
	range3d_iterator(const range3d_iterator<AI>& other); 


	/**
	   Assignment operator from another type of iterator 
	   \tparam AI other iterator type. The assignment I b = a; with a of type AI must be defined.
	   \param other 
	 */
	template <typename AI>
	range3d_iterator<I>& operator = (const range3d_iterator<AI>& other); 

	
	/// prefix increment 
	range3d_iterator<I>& operator ++(); 
	/// postfix increment 
	range3d_iterator<I> operator ++(int); 
	
	/// @returns current value the iterator points to 
	reference  operator *() const;
	
	/// @returns pointer to the current value the iterator points to 
	pointer    operator ->() const;

	/** \returns the current position within the 3D grid with respect to the 
	    full size of the grid. 
	 */
	const C3DBounds& pos() const; 

	/// @cond NOFRIENDDOC
	template <typename T> friend
	bool operator == (const range3d_iterator<T>& left, const range3d_iterator<T>& right); 
	/// @endcond 

	/**
	   Return the internal iterator
	 */
	internal_iterator get_point(); 

	/// \returns the flags describing whether the iterator is on a domain boundary. 
	int get_boundary_flags() const; 

private: 

	void increment_y(); 
	void increment_z(); 

	C3DBounds m_pos; 
	C3DBounds m_size; 
	C3DBounds m_begin; 
	C3DBounds m_end; 
	int m_xstride; 
	int m_ystride; 
	I m_iterator; 
	int m_boundary; 
}; 



template <typename I> 
template <typename AI>
range3d_iterator<I>& range3d_iterator<I>::operator = (const range3d_iterator<AI>& other)
{
	m_pos = other.m_pos; 
	m_size = other.m_size;  
	m_begin = other.m_begin; 
	m_end = other.m_end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_ystride = other.m_ystride; 
	m_boundary = other.m_boundary; 
	return *this; 
}

template <typename I> 
template <typename AI>
range3d_iterator<I>::range3d_iterator(const range3d_iterator<AI>& other):
	m_pos(other.m_pos), 
	m_size(other.m_size), 
	m_begin(other.m_begin), 
	m_end(other.m_end), 
	m_xstride(other.m_xstride),
	m_ystride(other.m_ystride),
	m_iterator(other.m_iterator), 
	m_boundary(other.m_boundary)
{
}	


/**
   Compare two range iterators. There equivalence is only decided based on the grid position. 
 */

template <typename I> 
bool operator == (const range3d_iterator<I>& left, const range3d_iterator<I>& right)
{
	// we really want these two to the same range 
//	assert(left.m_size == right.m_size);
//	assert(left.m_begin == right.m_begin);
//	assert(left.m_end == right.m_end);

	return left.m_pos == right.m_pos; 

}

/**
   Compare two range iterators. There equivalence is only decided based on the grid position. 
 */
template <typename I> 
bool operator != (const range3d_iterator<I>& a, const range3d_iterator<I>& b)
{
	return !(a == b); 
}

NS_MIA_END

#endif
