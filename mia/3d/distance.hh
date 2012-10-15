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

#ifndef mia_3d_distance_hh
#define mia_3d_distance_hh

#include <mia/core/iodata.hh>
#include <mia/3d/3DImage.hh>


NS_MIA_BEGIN

C3DFImage EXPORT_3D distance_transform(const C3DImage& f); 


class C3DDistance : public CIOData {
	
	C3DDistance(); 

	~C3DDistance(); 

	C3DDistance(const C3DDistance& other) = delete;
	C3DDistance& operator =(const C3DDistance& other) = delete;
		
	void push_slice(int z, const C2DImage& slice); 

	float get_distance_at(const C3DFVector& p) const;

	C2DFImage get_distance_slice(int z) const; 
private: 
	struct C3DDistanceImpl *impl; 
}; 

NS_MIA_END

#endif 
