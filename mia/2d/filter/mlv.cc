/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
#include <mia/2d/filter/mlv.hh>

#include <limits>

#ifdef HAVE_BLAS
extern "C" {
#include <cblas.h>
}
#endif

NS_BEGIN(mlv_2dimage_filter);

NS_MIA_USE;
using namespace std;

C2DMLV::C2DMLV(int hw):
	m_l(hw),
	m_kh(2 * hw +1),
	m_mu(C2DBounds(0,0)),
	m_sigma(C2DBounds(0,0)),
	m_n(C2DBounds(0,0))
{
}

template <typename T>
void C2DMLV::run_sub(const T2DImage<T>& image, int cx, int cy, float& mu, float& sigma, float &n)const
{
	n = 0.0;
	const int ymax = min(cy +  m_kh, image.get_size().y);
	const int xmax = min(cx +  m_kh, image.get_size().x);
	mu = 0.0f;
	sigma = 0.0f;

	for (int y = max(cy, 0);  y < ymax ; ++y)
		for (int x = max(cx, 0); x < xmax; ++x) {
			++n;
			const float val = image(x,y);
			mu += val;
			sigma += val * val;
		}
	if (n > 1) {
		mu /= n;
		sigma = (sigma - n * mu * mu) / (n - 1);
	}else
		sigma = 0;
}


template <typename T>
T C2DMLV::get(int x, int y, float ref)const
{

	const int ymax = y +  m_kh;
	const int xmax = x +  m_kh;


	float best_sigma = numeric_limits<float>::max();
	float best_mu = 0;

	int i = 0;

	for (int iy = y; iy < ymax; ++iy)
		for (int ix = x; ix < xmax; ++ix, ++i) {
			float const &s =  m_sigma(ix,iy);
			if (best_sigma > s) {
				best_sigma = s;
				best_mu = m_mu(ix,iy);
			}else if (best_sigma == s) {
				const float mu = m_mu(ix,iy);
				if (::fabs(mu - best_mu) < ::fabs(best_mu - ref)) {
					best_mu = mu;
				}
			}
		}
	return  static_cast<T>(best_mu);
}



template <typename CI>
struct __dispatch_trasform {
	static void apply(CI beginA, CI endA, C2DFImage::iterator out) {
		transform(beginA, endA,  out,  [](typename CI::value_type x){return x  * x;});
	}
	static void apply_add(C2DFImage::iterator beginA, C2DFImage::iterator endA, CI beginB) {
		while (beginA != endA)
			*beginA++ += *beginB++;
	}
};

// boost lambda doesn't support "long long" or "__int64"
#if defined(HAVE_INT64) && defined(LONG_IS_INT64)
template <>
struct __dispatch_trasform<C2DSLImage::const_iterator> {
	static void apply(C2DSLImage::const_iterator beginA, C2DSLImage::const_iterator endA, C2DFImage::iterator out) {
		while (beginA != endA) {
			*out = *beginA * *beginA;
			++out;
			++beginA;
		}
	}
	static void apply_add(C2DFImage::iterator beginA, C2DFImage::iterator endA, C2DSLImage::const_iterator beginB) {
		while (beginA != endA)
			*beginA++ += *beginB++;
	}
};


template <>
struct __dispatch_trasform<C2DULImage::const_iterator> {
	static void apply(C2DULImage::const_iterator beginA, C2DULImage::const_iterator endA, C2DFImage::iterator out) {
		while (beginA != endA) {
			*out = *beginA * *beginA;
			++out;
			++beginA;
		}
	}
	static void apply_add(C2DFImage::iterator beginA, C2DFImage::iterator endA, C2DULImage::const_iterator beginB) {
		while (beginA != endA)
			*beginA++ += *beginB++;
	}
};
#endif

template <>
struct __dispatch_trasform<C2DBitImage::const_iterator> {
	static void apply(C2DBitImage::const_iterator beginA, C2DBitImage::const_iterator endA, C2DFImage::iterator out) {
		while (beginA != endA)
			*out++  = *beginA++ ? 1 : 0;
	}
	static void apply_add(C2DFImage::iterator beginA, C2DFImage::iterator endA, C2DBitImage::const_iterator beginB) {
		while (beginA != endA) {
			if (*beginB)
				*beginA += 1.0f;
			++beginB;
			++beginA;
		}
	}
};

template <typename T>
C2DMLV::result_type C2DMLV::operator () (const T2DImage<T>& data) const
{
	cvdebug() << "C2DMLV::operator () begin\n";

	T2DImage<T> *result = new T2DImage<T>(data.get_size());
	C2DBounds border(2*m_l, 2*m_l);
	C2DBounds extsize = data.get_size() + border;

	if (m_mu.get_size() != extsize) {
		cvdebug() << "run initialise code\n";
		// reinitialize
		m_mu = C2DFImage(extsize);
		m_sigma = C2DFImage(extsize);
		m_n = C2DFImage(extsize);
		m_sqbuf.resize(data.get_size().x);
		m_buf.resize(data.get_size().x);
		m_mu_l1.resize(extsize.x);
		m_sigma_l1.resize(extsize.x);

		C2DFImage::iterator i_mu = m_mu.begin();
		C2DFImage::iterator i_s = m_sigma.begin();
		C2DFImage::iterator i_n = m_n.begin();
		for (size_t y = 0; y < m_n.get_size().y; ++y)
			for (size_t x = 0; x < m_n.get_size().x; ++x, ++i_mu, ++i_s, ++i_n)
				run_sub(data, int(x) - border.x, int(y) - border.y, *i_mu, *i_s, *i_n);
	}else {
		cvdebug() << "run size repeat code\n";
		fill(m_mu.begin(), m_mu.end(), 0.0f);
		fill(m_sigma.begin(), m_sigma.end(), 0.0f);

		cvdebug() << "temporary image have size (" << m_mu.get_size().x << ", " << m_mu.get_size().y << ")\n";

		for (size_t y = 0; y < data.get_size().y; ++y) {
			fill(m_mu_l1.begin(), m_mu_l1.end(), 0.0f);
			fill(m_sigma_l1.begin(), m_sigma_l1.end(), 0.0f);


			__dispatch_trasform<typename T2DImage<T>::const_iterator>::apply(data.begin() + data.get_size().x * y,
											 data.begin() + data.get_size().x * (y + 1) ,
											 m_sqbuf.begin());

#ifdef HAVE_BLAS
			copy(data.begin_at(0,y), data.begin_at(0,y) + data.get_size().x, m_buf.begin());
			for (size_t x = 0; x < m_kh; ++x) {
				cblas_saxpy(data.get_size().x, 1.0f, &m_buf[0],  1, &m_mu_l1[x], 1);
				cblas_saxpy(data.get_size().x, 1.0f, &m_sqbuf[0],  1, &m_sigma_l1[x], 1);
			}

			for (size_t iy= 0; iy < m_kh; ++iy) {
				cblas_saxpy(m_mu_l1.size(), 1.0f, &m_mu_l1[0],  1, &m_mu(0,y + iy), 1);
				cblas_saxpy(m_sigma_l1.size(), 1.0f, &m_sigma_l1[0],  1, &m_sigma(0,y + iy), 1);
			}
#else
			for (size_t x = 0; x < m_kh; ++x) {
				C2DFImage::iterator start = m_mu_l1.begin() + x;

				__dispatch_trasform<typename T2DImage<T>::const_iterator>::apply_add(start,
												     start + data.get_size().x,
												     data.begin() + data.get_size().x * y);

				start = m_sigma_l1.begin() + x;
				transform(start, start + data.get_size().x, m_sqbuf.begin(), start, 
					  [][float a, float b](return a+b;});
			}

			for (size_t iy= 0; iy < m_kh; ++iy) {
				transform(m_mu_l1.begin(), m_mu_l1.end(), m_mu.begin_at(0,y + iy), 
					  m_mu.begin_at(0,y + iy), [](float a, float b)(return a+b;});
				transform(m_sigma_l1.begin(), m_sigma_l1.end(), m_sigma.begin_at(0,y + iy), 
					  m_sigma.begin_at(0,y + iy), [](float a, float b)(return a+b;});
			}
#endif

		}


		C2DFImage::const_iterator i_n = m_n.begin();
		C2DFImage::const_iterator e_n = m_n.end();
		C2DFImage::iterator i_mu = m_mu.begin();
		C2DFImage::iterator i_s = m_sigma.begin();

		while ( i_n != e_n) {
			if (*i_n > 1.0) {
				const float mun = *i_mu;
				*i_mu /= *i_n;
				*i_s = (*i_s - mun * *i_mu) / (*i_n - 1);
			}else
				*i_s = 0.0f;
			++i_n; ++i_s; ++i_mu;
		}
	}

	typename T2DImage<T>::iterator i = result->begin();
	typename T2DImage<T>::const_iterator s = data.begin();

	for (int y = 0; y < (int)data.get_size().y; ++y) {
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i, ++s)
			*i = get<T>(x, y, *s);
	}

	cvdebug() << "C2DExtKuwa::operator () end\n";
	return P2DImage(result);
}

P2DImage C2DMLV::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMLVImageFilterFactory::C2DMLVImageFilterFactory():
	C2DFilterPlugin("mlv"),
	m_hw(1)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));
}

C2DFilter *C2DMLVImageFilterFactory::do_create()const
{
	return new C2DMLV(m_hw);
}

const string C2DMLVImageFilterFactory::do_get_descr()const
{
	return "2D image mean least variance filter";
}

struct FCompare {
	bool operator () (float a, float b){
		return ::fabs(a-b) < 0.0001;
	}
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMLVImageFilterFactory();
}

NS_END
