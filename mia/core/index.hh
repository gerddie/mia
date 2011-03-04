/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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

#ifndef mia_core_index_hh
#define mia_core_index_hh

#include <mia/core/defines.hh>

NS_MIA_BEGIN

class CCircularIndex {
public: 
	CCircularIndex(unsigned int size, unsigned int start); 
	
	void insert_one(); 
	unsigned int next() const; 
	void new_start(unsigned int start);
	unsigned int fill()const; 
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