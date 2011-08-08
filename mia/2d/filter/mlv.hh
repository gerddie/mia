/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/2dfilter.hh>

#include <limits>

NS_BEGIN(mlv_2dimage_filter);

class C2DMLV: public mia::C2DFilter {
public:
	C2DMLV(int hw);

	template <typename T>
	C2DMLV::result_type operator () (const mia::T2DImage<T>& data) const;
private:

	template <typename T>
	T get(int x, int y, float ref)const;

	template <typename T>
	void run_sub(const mia::T2DImage<T>& image, int cx, int cy, float& mu, float& sigma, float& n) const;

	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	int m_l;
	unsigned int m_kh;

	mutable mia::C2DFImage m_mu;
	mutable mia::C2DFImage m_sigma;
	mutable mia::C2DFImage m_n;
	mutable std::vector<float> m_sqbuf;
	mutable std::vector<float> m_buf;
	mutable std::vector<float> m_mu_l1;
	mutable std::vector<float> m_sigma_l1;
};


class C2DExtKuwaImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DExtKuwaImageFilterFactory();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr() const;
	int m_hw;
};

NS_END
