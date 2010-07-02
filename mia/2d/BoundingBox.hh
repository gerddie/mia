/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef __mia_2d_boundingbox_hh
#define __mia_2d_boundingbox_hh

#include <mia/2d/2DVector.hh>


NS_MIA_BEGIN

typedef T2DVector<int> C2DIVector;

class C2DBoundingBox {
public:
	C2DBoundingBox();
	C2DBoundingBox(const C2DFVector& begin, const C2DFVector& end);

	void unite(const C2DBoundingBox& other);
	void add(const C2DFVector& point);

	C2DIVector get_grid_begin() const;
	C2DIVector get_grid_end() const;
	C2DBounds get_grid_size() const;

	C2DFVector get_begin() const;
	C2DFVector get_end() const;
	C2DFVector get_size() const;

	void enlarge(float boundary);

	bool empty() const;

private:
	bool _M_empty;
	C2DFVector _M_begin;
	C2DFVector _M_end;
};

NS_MIA_END

#endif
