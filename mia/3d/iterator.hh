/* -*- mia-c++  -*-
 *   
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#ifndef mia_3d_iterator_hh
#define mia_3d_iterator_hh

#include <mia/3d/3DVector.hh>

NS_MIA_BEGIN

template <typename I> 
class range3d_iterator: public std::forward_iterator_tag {
public: 
	typedef typename I::value_type value_type; 
	
	range3d_iterator(); 
	range3d_iterator(const C3DBounds& pos, const C3DBounds& size, 
		       const C3DBounds& start, const C3DBounds& end, I iterator);

	range3d_iterator<I>& operator = (const range3d_iterator<I>& other); 
	range3d_iterator(const range3d_iterator<I>& other); 
	
	range3d_iterator<I>& operator ++(); 
	range3d_iterator<I> operator ++(int); 
	
	const value_type&  operator *() const;
	const value_type  *operator ->() const;
	
	const C3DBounds& pos() const; 

	bool operator == (const range3d_iterator<I>& other) const; 

private: 
	void increment_y(); 
	void increment_z(); 

	C3DBounds m_pos; 
	C3DBounds m_size; 
	C3DBounds m_start; 
	C3DBounds m_end; 
	int m_xstride; 
	int m_ystride; 
	I m_iterator; 
}; 

NS_MIA_END

#endif
