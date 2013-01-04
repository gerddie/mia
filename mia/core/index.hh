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


#ifndef mia_core_index_hh
#define mia_core_index_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup misc
   
   \brief A class to adress a circulat bufer of a fixed size. 

   This class provides the structure for the translation of a linear index to 
   a circular buffer. 
   An example for its use can be found in mia/3d/translate/spline.cc: on_grid (BLAS_VERSION)
 */

class EXPORT_CORE CCircularIndex {
public: 
	/**
	  Construct the buffer
	  \param size size ofthe buffer 
	  \param start start index
	*/
	CCircularIndex(unsigned int size, unsigned int start); 
	
	/** Insert one index */
	void insert_one(); 

	/** get the next index */
	unsigned int next() const; 

	/** reset the start index */ 
	void new_start(unsigned int start);

	/** \returns the number of occupied indices */
	unsigned int fill()const; 

	/** 
	    Translate the linear index i to the index in the circular buffer 
	    \param i 
	    \returns circular index 
	 */
	unsigned int value(unsigned int i) const; 
private: 
	unsigned int m_size; 
	unsigned int m_start; 
	unsigned int m_fill;
	unsigned int m_next_fill; 
	unsigned int m_cur_start; 
}; 

NS_MIA_END

#endif
