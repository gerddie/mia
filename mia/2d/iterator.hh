/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#ifndef mia_2d_iterator_hh
#define mia_2d_iterator_hh

#include <mia/2d/2DVector.hh>

NS_MIA_BEGIN

/**
   \brief a 2D iterator that knows its position in the 2D grid ans supports iterating over 
   sub-ranges 
   

   Iterator to iterate over a sub-range of 2D data that is given on a grid. 
   Two iterators are considered to be equal, if their positions are equal.  
   \tparam the internal iterator that is used to iterate of the original 
   grid without skipping. 
   
 */

template <typename I> 
class range2d_iterator: public std::forward_iterator_tag {
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
	}; 
	

	
	/** standard constructor */
	range2d_iterator(); 

	/**
	   Full constructor of the range iterator 
	   @param pos iterator position to initialize the iterator with 
	   @param size size of the original data field 
	   @param start start of the iterator range 
	   @param end end of the iterator range 
	   @param iterator the iterator of the underlying 2D data structure 
	*/
	range2d_iterator(const C2DBounds& pos, const C2DBounds& size, 
			 const C2DBounds& start, const C2DBounds& end, I iterator);
	
	/**
	   End iterator, can't be dereferenced  
	   This iterator is only there to define the end position of the range_iterator. 
	   \param pos end position to set this iterator to. 
	 */
	range2d_iterator(const C2DBounds& pos);

	/// assignment operator 
	range2d_iterator<I>& operator = (const range2d_iterator<I>& other); 
	
	/// copy constructore 
	range2d_iterator(const range2d_iterator<I>& other); 

	/// friend iterator type because we may want to copy a iterator to a const_iterator. 
	template <typename AI> 
	friend class range2d_iterator; 
	
	/**
	   Constructor to construct the iterator  from one that is based on another 
	   iterator type. The usual idea is that a iterator may be converted  into it's const variant. 
	   \tparam AI the other iterator type. Iterator type I must be copy-constructable from 
	   type AI 
	   \param other 
	 */
	template <typename AI>
	range2d_iterator(const range2d_iterator<AI>& other); 


	/**
	   Assignment operator from another type of iterator 
	   \tparam AI other iterator type. The assignment I b = a; with a of type AI must be defined.
	   \param other 
	 */
	template <typename AI>
	range2d_iterator<I>& operator = (const range2d_iterator<AI>& other); 

	
	/// prefix increment 
	range2d_iterator<I>& operator ++(); 
	/// postfix increment 
	range2d_iterator<I> operator ++(int); 
	
	/// @returns current value the iterator points to 
	reference  operator *() const;
	
	/// @returns pointer to the current value the iterator points to 
	pointer    operator ->() const;

	/** \returns the current position within the 2D grid with respect to the 
	    full size of the grid. 
	 */
	const C2DBounds& pos() const; 

	/// @cond NOFRIENDDOC
	template <typename T> friend
	bool operator == (const range2d_iterator<T>& left, const range2d_iterator<T>& right); 
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

	C2DBounds m_pos; 
	C2DBounds m_size; 
	C2DBounds m_begin; 
	C2DBounds m_end; 
	int m_xstride; 
	I m_iterator; 
	int m_boundary; 
}; 



template <typename I> 
template <typename AI>
range2d_iterator<I>& range2d_iterator<I>::operator = (const range2d_iterator<AI>& other)
{
	m_pos = other.m_pos; 
	m_size = other.m_size;  
	m_begin = other.m_begin; 
	m_end = other.m_end; 
	m_iterator = other.m_iterator; 
	m_xstride = other.m_xstride; 
	m_boundary = other.m_boundary; 
	return *this; 
}

template <typename I> 
template <typename AI>
range2d_iterator<I>::range2d_iterator(const range2d_iterator<AI>& other):
	m_pos(other.m_pos), 
	m_size(other.m_size), 
	m_begin(other.m_begin), 
	m_end(other.m_end), 
	m_xstride(other.m_xstride),
	m_iterator(other.m_iterator), 
	m_boundary(other.m_boundary)
{
}	


/**
   Compare two range iterators. There equivalence is only decided based on the grid position. 
 */

template <typename I> 
bool operator == (const range2d_iterator<I>& left, const range2d_iterator<I>& right)
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
bool operator != (const range2d_iterator<I>& a, const range2d_iterator<I>& b)
{
	return !(a == b); 
}

NS_MIA_END

#endif
