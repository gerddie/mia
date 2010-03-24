/*  -*- mia-c++  -*-
 * Copyright (c) 2004 
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: 2DDatafield.hh 732 2005-11-14 18:06:39Z write1 $

/*! \brief A templated 2D datafield class

The class holds all types of data stored in two dimensional fields.

\file 2DDatafield.hh
\author Gert Wollny <wollny@cbs.mpg.de>, 2004

*/
// implementation part is here:

#ifdef WIN32
#  define EXPORT_2DDATAFIELD __declspec(dllexport)
#else 
#  define EXPORT_2DDATAFIELD 
#endif

#include <mia/2d/2DDatafield.cxx>

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

NS_MIA_END

