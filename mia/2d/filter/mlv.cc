/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/mlv.hh>

#include <limits>

#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

#ifdef HAVE_BLAS
extern "C" {
#include <cblas.h>
}
#endif

NS_BEGIN(mlv_2dimage_filter);

NS_MIA_USE;
using namespace std;

C2DMLV::C2DMLV(int hw):
	_M_l(hw),
	_M_kh(2 * hw +1),
	_M_mu(C2DBounds(0,0)),
	_M_sigma(C2DBounds(0,0)),
	_M_n(C2DBounds(0,0))
{
}

template <typename T>
void C2DMLV::run_sub(const T2DImage<T>& image, int cx, int cy, float& mu, float& sigma, float &n)const
{
	n = 0.0;
	const int ymax = min(cy +  _M_kh, image.get_size().y);
	const int xmax = min(cx +  _M_kh, image.get_size().x);
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

	const int ymax = y +  _M_kh;
	const int xmax = x +  _M_kh;


	float best_sigma = numeric_limits<float>::max();
	float best_mu = 0;

	int i = 0;

	for (int iy = y; iy < ymax; ++iy)
		for (int ix = x; ix < xmax; ++ix, ++i) {
			float const &s =  _M_sigma(ix,iy);
			if (best_sigma > s) {
				best_sigma = s;
				best_mu = _M_mu(ix,iy);
			}else if (best_sigma == s) {
				const float mu = _M_mu(ix,iy);
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
		transform(beginA, endA,  out,  _1 * _1);
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
	C2DBounds border(2*_M_l, 2*_M_l);
	C2DBounds extsize = data.get_size() + border;

	if (_M_mu.get_size() != extsize) {
		cvdebug() << "run initialise code\n";
		// reinitialize
		_M_mu = C2DFImage(extsize);
		_M_sigma = C2DFImage(extsize);
		_M_n = C2DFImage(extsize);
		_M_sqbuf.resize(data.get_size().x);
		_M_buf.resize(data.get_size().x);
		_M_mu_l1.resize(extsize.x);
		_M_sigma_l1.resize(extsize.x);

		C2DFImage::iterator i_mu = _M_mu.begin();
		C2DFImage::iterator i_s = _M_sigma.begin();
		C2DFImage::iterator i_n = _M_n.begin();
		for (size_t y = 0; y < _M_n.get_size().y; ++y)
			for (size_t x = 0; x < _M_n.get_size().x; ++x, ++i_mu, ++i_s, ++i_n)
				run_sub(data, int(x) - border.x, int(y) - border.y, *i_mu, *i_s, *i_n);
	}else {
		cvdebug() << "run size repeat code\n";
		fill(_M_mu.begin(), _M_mu.end(), 0.0f);
		fill(_M_sigma.begin(), _M_sigma.end(), 0.0f);

		cvdebug() << "temporary image have size (" << _M_mu.get_size().x << ", " << _M_mu.get_size().y << ")\n";

		for (size_t y = 0; y < data.get_size().y; ++y) {
			fill(_M_mu_l1.begin(), _M_mu_l1.end(), 0.0f);
			fill(_M_sigma_l1.begin(), _M_sigma_l1.end(), 0.0f);


			__dispatch_trasform<typename T2DImage<T>::const_iterator>::apply(data.begin() + data.get_size().x * y,
											 data.begin() + data.get_size().x * (y + 1) ,
											 _M_sqbuf.begin());

#ifdef HAVE_BLAS
			copy(&data(0,y), &data(0,y) + data.get_size().x, _M_buf.begin());
			for (size_t x = 0; x < _M_kh; ++x) {
				cblas_saxpy(data.get_size().x, 1.0f, &_M_buf[0],  1, &_M_mu_l1[x], 1);
				cblas_saxpy(data.get_size().x, 1.0f, &_M_sqbuf[0],  1, &_M_sigma_l1[x], 1);
			}

			for (size_t iy= 0; iy < _M_kh; ++iy) {
				cblas_saxpy(_M_mu_l1.size(), 1.0f, &_M_mu_l1[0],  1, &_M_mu(0,y + iy), 1);
				cblas_saxpy(_M_sigma_l1.size(), 1.0f, &_M_sigma_l1[0],  1, &_M_sigma(0,y + iy), 1);
			}
#else
			for (size_t x = 0; x < _M_kh; ++x) {
				C2DFImage::iterator start = _M_mu_l1.begin() + x;

				__dispatch_trasform<typename T2DImage<T>::const_iterator>::apply_add(start,
												     start + data.get_size().x,
												     data.begin() + data.get_size().x * y);

				start = _M_sigma_l1.begin() + x;
				transform(start, start + data.get_size().x, _M_sqbuf.begin(), start, _1 + _2);
			}

			for (size_t iy= 0; iy < _M_kh; ++iy) {
				transform(_M_mu_l1.begin(), _M_mu_l1.end(), _M_mu.begin_at(0,y + iy), _M_mu.begin_at(0,y + iy), _1 + _2);
				transform(_M_sigma_l1.begin(), _M_sigma_l1.end(), _M_sigma.begin_at(0,y + iy), _M_sigma.begin_at(0,y + iy), _1 + _2);
			}
#endif

		}


		C2DFImage::const_iterator i_n = _M_n.begin();
		C2DFImage::const_iterator e_n = _M_n.end();
		C2DFImage::iterator i_mu = _M_mu.begin();
		C2DFImage::iterator i_s = _M_sigma.begin();

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

C2DExtKuwaImageFilterFactory::C2DExtKuwaImageFilterFactory():
	C2DFilterPlugin("mlv"),
	_M_hw(1)
{
	add_parameter("w", new CIntParameter(_M_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));
}

C2DExtKuwaImageFilterFactory::ProductPtr C2DExtKuwaImageFilterFactory::do_create()const
{
	return C2DExtKuwaImageFilterFactory::ProductPtr(new C2DMLV(_M_hw));
}

const string C2DExtKuwaImageFilterFactory::do_get_descr()const
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
	return new C2DExtKuwaImageFilterFactory();
}

NS_END
