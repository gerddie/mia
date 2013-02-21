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


#ifndef mia_3d_imagecollect_hh
#define mia_3d_imagecollect_hh

#include <mia/3d/image.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN

class  EXPORT_3D C3DImageCollector : public TFilter<void> {
public: 
	C3DImageCollector(size_t slices); 
	
	template <typename T>
	void operator ()(const T2DImage<T>& image); 

	void add(const C2DImage &image); 
	
	P3DImage get_result()const; 
private:
	size_t m_slices;
	size_t m_cur_slice;
	P3DImage m_image;
};

NS_MIA_END

#endif
