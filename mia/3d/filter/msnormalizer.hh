/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_3d_filter_msnormaliter_hh
#define mia_3d_filter_msnormaliter_hh

#include <mia/3d/filter.hh>

NS_BEGIN(msnormalizer_3dimage_filter)
 
int n_elements( int i, int n, int w); 

class C3DMSNormalizerFilter: public mia::C3DFilter {
public:
	C3DMSNormalizerFilter(int hwidth);
        
	template <class T>
	mia::P3DImage operator () (const mia::T3DImage<T>& data) const;
        
private:

        template <class T>
        void  add(mia::C3DFImage& mean, mia::C3DFImage& variance, const mia::T3DImage<T>& data, 
                  const mia::C3DBounds& bi, const mia::C3DBounds& bo, const mia::C3DBounds& ei) const; 
	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;
	int m_hwidth;
};

class C3DMSNormalizerFilterPlugin: public mia::C3DFilterPlugin {
public:
	C3DMSNormalizerFilterPlugin();
private:
	virtual mia::C3DFilter *do_create()const;
	virtual const std::string  do_get_descr() const;
	int m_hw;
};

NS_END

#endif 
