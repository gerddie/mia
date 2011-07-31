/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * Biomedical Image Technologies, Universidad Politecnica de Madrid
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

#ifndef mia_3d_filter_mlv_hh
#define mia_3d_filter_mlv_hh

#include <mia/3d/3dfilter.hh>

NS_BEGIN(mlv_3dimage_filter)

class C3DMLVImageFilter: public mia::C3DFilter {
	int m_l;
	int m_kh;
public:
	C3DMLVImageFilter(int hw);

	template <typename T>
	typename mia::C3DFilter::result_type operator () (const mia::T3DImage<T>& data) const ;

private:
	template <typename T>
	T get(const mia::C3DFImage& mu, const mia::C3DFImage& sigma,
					int x, int y, int z, T ref) const;

	template <typename InputIterator>
	void run_sub(InputIterator begin, InputIterator end,  int cy, int cz,
		     mia::C3DFImage& mu, mia::C3DFImage& sigma,
		     mia::C3DFImage& n, std::vector<float>& buffer, const std::vector<float>& eins)const;

	virtual mia::P3DImage do_filter(const mia::C3DImage& image) const;

};

class C3DMLVImageFilterFactory: public mia::C3DFilterPlugin {
public:
	C3DMLVImageFilterFactory();
private:

	virtual mia::C3DFilter *do_create()const;
	virtual const std::string do_get_descr() const;
	virtual bool  do_test() const;
	int m_hw;
};


NS_END

#endif
