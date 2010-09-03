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

#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else
#  ifdef __GNUC__
#    define EXPORT_2DDATAFIELD  __attribute__((visibility("default")))
#  else 
#    define EXPORT_2DDATAFIELD 
#  endif 
#endif

#include <mia/2d/2DVectorfield.hh>
#include <mia/2d/2DDatafield.cxx>

NS_MIA_BEGIN

EXPORT_2D C2DFVectorfield& operator += (C2DFVectorfield& a, const C2DFVectorfield& b)
{
	assert(a.get_size() == b.get_size());

	C2DFVectorfield help(a.get_size());
	std::copy(a.begin(), a.end(), help.begin());
	C2DFVectorfield::iterator i = a.begin();
	C2DFVectorfield::const_iterator u = b.begin();


	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x, ++i, ++u)  {
			C2DFVector xi = C2DFVector(x, y) - *u;
			*i = help.get_interpol_val_at(xi) +  *u;
		}
	}
	return a;
}
template <typename T>
const char *T2DVectorfield<T>::type_descr = "2dvf"; 

template class T2DDatafield<C2DFVector>;
template class EXPORT_2DDATAFIELD T2DDatafield<C2DDVector>;

template class T2DVectorfield<C2DFVector>;
template class EXPORT_2DDATAFIELD T2DVectorfield<C2DDVector>;



NS_MIA_END
