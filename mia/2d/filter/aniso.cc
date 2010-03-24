/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

#include <limits>

#include <algorithm>
#include <cmath>
#include <boost/algorithm/minmax_element.hpp>
#include <mia/core/dictmap.hh>
#include <mia/2d/filter/aniso.hh>


NS_BEGIN(aniso_2dimage_filter)
NS_MIA_USE;
using namespace std;

// scale factor for MAD to zero-mean normal distribution
static const float zmn_weight = 1.4826;
static const float cinv_sqrt2 = 1.0f / sqrt(2.0f);
static const char *plugin_name = "aniso";

float psi_guess(float x, float sigma)
{
	const float val = x / sigma;
	return x * expf(- val * val * 0.25f);
}

float psi_test(float /*x*/, float /*sigma*/)
{
	return 1.0;
}


float psi_tuckey(float x, float sigma)
{
	if (x > sigma || -x > sigma)
		return 0.0;
	const float val = x / sigma;
	const float val2 = 1.0 - val * val;
	return x * val2 * val2;
}

float psi_pm1(float x, float sigma)
{
	const float val = x / sigma;
	return 2.0f * x / (2.0f + val * val);
}

float psi_pm2(float x, float sigma)
{
	const float val = x / sigma;
	return x * expf(- val * val * 0.25f);
}

static const TDictMap<C2DAnisoDiff::FEdgeStopping>::Table edge_stop_table[] = {
	{ "tuckey", psi_tuckey},
	{ "pm1", psi_pm1},
	{ "pm2", psi_pm2},
	{ "guess", psi_guess},
	{ NULL, NULL}
};

static const TDictMap<C2DAnisoDiff::FEdgeStopping> edge_stop_dict(edge_stop_table);


C2DAnisoDiff::C2DAnisoDiff(int maxiter, float epsilon, float k, FEdgeStopping edge_stop, int neighbourhood):
	_M_maxiter(maxiter),
	_M_epsilon(epsilon),
	_M_k(k),
	_M_edge_stop(edge_stop),
	_M_neighbourhood(neighbourhood),
	_M_histogramfeeder(0,256,256)
{
	if (_M_neighbourhood != 4 && _M_neighbourhood != 8) {
		stringstream errmsg;
		errmsg << "neighbourhood " << _M_neighbourhood << "not supported. Select 4 or 8";
		throw invalid_argument(errmsg.str());
	}
}

typedef pair<float, float> HScale;

inline bool operator < (const HScale& a, const HScale& b)
{
	return a.first < b.first;
}

void C2DAnisoDiff::create_histogramfeeder(const C2DFImage& data) const
{
	pair<C2DFImage::const_iterator, C2DFImage::const_iterator>
		range = ::boost::minmax_element(data.begin(), data.end());

	// this is to avoid some warnings
	float dist = *range.second - *range.first;

	cvdebug() << "Histogram spread = " << dist
		  << " in ["<< *range.first
		  <<", "<< *range.second  <<"]\n";

	size_t bins;
	if (dist <= 4096.0f)
		bins = 4096;
	else if (dist >= 16384)
		bins = 16384;
	else
		bins = (size_t)(dist+1);

	_M_histogramfeeder = CHistogramFeeder<float>(*range.first, *range.second, bins);
}

/* estimate the MAD */
float C2DAnisoDiff::estimate_MAD(const C2DFImage& data)const
{
	CHistogram<CHistogramFeeder<float> >  gradient_histogram(_M_histogramfeeder);

	C2DFImage::const_iterator id = data.begin();

	for (size_t y = 0; y < data.get_size().y - 1; ++y) {
		for (size_t x = 0; x < data.get_size().x - 1; ++x, ++id) {
			const float idd = *id;
			const float d3 = id[+1] - idd;
			const float d4 = id[data.get_size().x]  - idd;

			gradient_histogram.push ( d3 > 0 ? d3 : -d3 );
			gradient_histogram.push ( d4 > 0 ? d4 : -d4 );
		}
		const float d4 = id[data.get_size().x]  - *id;
		gradient_histogram.push ( d4 > 0 ? d4 : -d4 );
		++id;
	}
	for (size_t x = 0; x < data.get_size().x - 1; ++x, ++id) {
		const float d3 = id[+1] - *id;
		gradient_histogram.push ( d3 > 0 ? d3 : -d3 );
	}

	return gradient_histogram.MAD();
}

float C2DAnisoDiff::diffuse(C2DFImage& dest, const C2DFImage& src)const
{
	float sum = 0.0;

	C2DFImage::iterator idest = dest.begin() + src.get_size().x;
	C2DFImage::const_iterator id = src.begin() + src.get_size().x;

	int sxm1 = src.get_size().x - 1;
	int sxp1 = src.get_size().x + 1;
	int sx   = src.get_size().x;

	// lazy way out to get the boundaries copied
	// just let the test pass for now
	copy(src.begin(), src.end(), dest.begin());

	for (size_t y = 1; y < src.get_size().y - 1; ++y) {
		++id;
		++idest;
		for (size_t x = 1; x < src.get_size().x - 1; ++x, ++id, ++idest) {

			float val  = 0.0;
			float idd = *id;

			if (_M_neighbourhood == 8) {
				val += _M_edge_stop(cinv_sqrt2 * (id[-sxp1]  - idd), _M_sigma);
				val += _M_edge_stop(cinv_sqrt2 * (id[-sxm1]  - idd), _M_sigma);
				val += _M_edge_stop(cinv_sqrt2 * (id[ sxm1]  - idd), _M_sigma);
				val += _M_edge_stop(cinv_sqrt2 * (id[ sxp1]  - idd), _M_sigma);
			}

			val += _M_edge_stop(id[-sx] - idd, _M_sigma);
			val += _M_edge_stop(id[ -1] - idd, _M_sigma);
			val += _M_edge_stop(id[ +1] - idd, _M_sigma);
			val += _M_edge_stop(id[ sx] - idd, _M_sigma);

			val *= _M_gamma;
			sum += val * val;

			*idest = idd + val;

		}
		++id;
		++idest;
	}

	return sum;
}

void C2DAnisoDiff::update_gamma_sigma(const mia::C2DFImage& src)const
{
	if (_M_k <= 0) {
		_M_sigma_e = zmn_weight *  estimate_MAD(src);
		_M_sigma = sqrt(5.0f) *  _M_sigma_e;
	}else{
		_M_sigma_e = _M_k / sqrt(5.0f);
		_M_sigma = _M_k;
	}
	_M_gamma = 1.0 / (_M_edge_stop(_M_sigma_e, _M_sigma) * _M_neighbourhood);

}

template <class T>
typename C2DAnisoDiff::result_type C2DAnisoDiff::operator () (const T2DImage<T>& image) const
{
	cvdebug() << "begin C2DAnisoDiff::operator (), k=" << _M_k << "\n";

	int iter = 0;

	C2DFImage *dest = new C2DFImage(image.get_size());;
	C2DFImage *src = new C2DFImage(image.get_size());

 	std::copy(image.begin(), image.end(), src->begin());
	create_histogramfeeder(*src);

	char endline = cverb.show_debug() ? '\n' : '\r';

	float delta;
	do {
		++iter;

		update_gamma_sigma(*src);

		if (_M_sigma_e == 0.0)  // image contains only one colour
			break;

		delta = diffuse(*dest, *src);

		cvmsg() <<iter <<": " << " _M_sigma_e = " << _M_sigma_e <<" _M_gamma = "<<_M_gamma <<" ";
		cvmsg() <<" delta " << delta << "       " << endline;

		swap(src, dest);

	} while (delta > _M_epsilon && iter < _M_maxiter);

	cvmsg() << '\n';

	const T max_val = numeric_limits<T>::max();
	const T min_val = numeric_limits<T>::min();

	T2DImage<T> *result = new T2DImage<T>(image.get_size());

	typename T2DImage<T>::iterator di = result->begin();
	for (C2DFImage::const_iterator wi = dest->begin();
	     wi != dest->end(); ++wi, ++di) {
		*di = (*wi > min_val) ?
			( (*wi < max_val) ? static_cast<T>(*wi) : max_val )
			: min_val;
	}

	delete src;
	delete dest;

	return P2DImage(result);
}

P2DImage C2DAnisoDiff::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

CAnisoDiff2DImageFilterFactory::CAnisoDiff2DImageFilterFactory():
	C2DFilterPlugin(plugin_name),
	_M_maxiter(100),
	_M_epsilon(1.0),
	_M_k ( -1.0),
	_M_edge_stop(),
	_M_neighbourhood(8)
{

	TRACE("CAnisoDiff2DImageFilterFactory::CAnisoDiff2DImageFilterFactory()");
	cvdebug() << "CAnisoDiff2DImageFilterFactory::CAnisoDiff2DImageFilterFactory()\n";

	add_parameter("iter", new CIntParameter(_M_maxiter, 1, 10000, false,  "number of iterations"));

	add_parameter("epsilon", new CFloatParameter(_M_epsilon,
						     0.001, 100, false,  "iteration change threshold"));
	add_parameter("k", new CFloatParameter(_M_k, 0, 100, false,
					       "k the noise threshold (<=0 -> adaptive)"));

	add_parameter("psi", new CDictParameter<C2DAnisoDiff::FEdgeStopping>(_M_edge_stop, edge_stop_table,
							       "edge stopping function"));

	set<int> nset;
	nset.insert(4);
	nset.insert(8);

	add_parameter("n", new CSetParameter<int>(_M_neighbourhood, nset, "neighbourhood"));
}

C2DFilterPlugin::ProductPtr CAnisoDiff2DImageFilterFactory::do_create() const
{
	return C2DFilterPlugin::ProductPtr(new C2DAnisoDiff(_M_maxiter, _M_epsilon, _M_k, _M_edge_stop, _M_neighbourhood));
}

const string CAnisoDiff2DImageFilterFactory::do_get_descr()const
{
	return string("2D Anisotropic image filter");
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CAnisoDiff2DImageFilterFactory();
}

NS_END
