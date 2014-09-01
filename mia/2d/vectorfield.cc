/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
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

#include <mia/2d/defines2d.hh>
#include <mia/2d/vectorfield.hh>
#include <mia/2d/datafield.cxx>
#include <mia/2d/iterator.cxx>

NS_MIA_BEGIN


template class EXPORT_2D T2DDatafield<C2DFVector>;
template class EXPORT_2D T2DDatafield<C2DDVector>;

template <typename T> 
T2DVectorfield<T>::T2DVectorfield()
{
};

template <typename T> 
T2DVectorfield<T>::T2DVectorfield(const C2DBounds& size):
	T2DDatafield<T>(size) 
{
};


template <typename T> 
T2DVectorfield<T>::T2DVectorfield(const CAttributedData& data, const C2DBounds& size):
	T2DDatafield<T>(size),
	CAttributedData(data)
{
}
	
template <typename T> 
C2DFVector T2DVectorfield<T>::get_pixel_size() const 
{
	const PAttribute attr = get_attribute("pixel");
	if (!attr) {
		cvinfo() << "C2DImage::get_pixel_size(): pixel size not defined\n";
		return C2DFVector(1,1);
	}
	
	const TAttribute<C2DFVector> * vs = dynamic_cast<TAttribute<C2DFVector> *>(attr.get());
	if (!vs){
		cvinfo() << "C2DImage::get_pixel_size(): pixel size wrong type\n";
		return C2DFVector(1,1);
	}
	return *vs;
}


template <typename T> 
void T2DVectorfield<T>::set_pixel_size(const C2DFVector& pixel)
{
	set_attribute("pixel", PAttribute(new TAttribute<C2DFVector>(pixel)));
}

template <typename T>
const char *T2DVectorfield<T>::data_descr = "2dvf"; 

#define INSTANCIATE(TYPE) \
	template class EXPORT_2D T2DVectorfield<TYPE>;			\
	template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::iterator>; \
	template class EXPORT_2D range2d_iterator<T2DDatafield<TYPE>::const_iterator>; \
	template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::iterator>; \
	template class EXPORT_2D range2d_iterator_with_boundary_flag<T2DDatafield<TYPE>::const_iterator>;


INSTANCIATE(C2DFVector);
INSTANCIATE(C2DDVector);

/**
   
 */
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







NS_MIA_END
