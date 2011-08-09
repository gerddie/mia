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

#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else
#  ifdef __GNUC__
#    define EXPORT_2DDATAFIELD  __attribute__((visibility("default")))
#  else 
#    define EXPORT_2DDATAFIELD 
#  endif 
#endif

#include <mia/2d/2DDatafield.cxx>
#include <mia/core/parameter.cxx>

NS_MIA_BEGIN

template class EXPORT_2DDATAFIELD T2DDatafield<float>;
#ifdef HAVE_INT64
template class EXPORT_2DDATAFIELD T2DDatafield<mia_int64>;
template class EXPORT_2DDATAFIELD T2DDatafield<mia_uint64>;
#endif
template class EXPORT_2DDATAFIELD T2DDatafield<double>;
template class EXPORT_2DDATAFIELD T2DDatafield<unsigned int>;
template class EXPORT_2DDATAFIELD T2DDatafield<signed int>;
template class EXPORT_2DDATAFIELD T2DDatafield<unsigned short>;
template class EXPORT_2DDATAFIELD T2DDatafield<signed short>;
template class EXPORT_2DDATAFIELD T2DDatafield<bool>;
template class EXPORT_2DDATAFIELD T2DDatafield<unsigned char>;
template class EXPORT_2DDATAFIELD T2DDatafield<signed char>;

extern const char type_str_2dbounds[] = "2DBounds";
template class EXPORT_2DDATAFIELD  CTParameter<C2DBounds, type_str_2dbounds>;

NS_MIA_END

