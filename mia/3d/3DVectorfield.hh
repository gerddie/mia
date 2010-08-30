/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifndef mia_3d_3dvectorfield_hh
#define mia_3d_3dvectorfield_hh

#include <mia/core/attributes.hh>
#include <mia/3d/3DDatafield.hh>

NS_MIA_BEGIN

template <typename T>
class T3DVectorfield: public T3DDatafield<T>, public CAttributedData {
public:
	T3DVectorfield(){};
	T3DVectorfield(const C3DBounds& size):
		T3DDatafield<T>(size) {};

	T3DVectorfield(const CAttributedData& data, const C3DBounds& size):
		T3DDatafield<T>(size),
		CAttributedData(data)
	{
	}
};

typedef T3DVectorfield<C3DFVector>  C3DFVectorfield;
typedef T3DVectorfield<C3DDVector>  C3DDVectorfield;
typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;

EXPORT_3D C3DFVectorfield& operator += (C3DFVectorfield& a, const C3DFVectorfield& b);

NS_MIA_END

#endif
