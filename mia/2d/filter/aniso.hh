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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <limits>
#include <mia/core/histogram.hh>
#include <mia/2d/2dfilter.hh>

NS_BEGIN(aniso_2dimage_filter)

float psi_tuckey(float x, float sigma);
float psi_pm1(float x, float sigma);
float psi_pm2(float x, float sigma);
float psi_test(float x, float sigma);


class C2DAnisoDiff: public mia::C2DFilter {
public:
	typedef float (*FEdgeStopping)(float x, float sigma);

	C2DAnisoDiff(int maxiter, float epsilon, float k, FEdgeStopping edge_stop, int neighbourhood);

	template <class T>
	result_type operator () (const mia::T2DImage<T>& image) const;
protected:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

	float diffuse(mia::C2DFImage& dest, const mia::C2DFImage& src)const;

	float estimate_MAD(const mia::C2DFImage& data)const;

	void create_histogramfeeder(const mia::C2DFImage& data) const;
	void update_gamma_sigma(const mia::C2DFImage& src)const;

	int _M_maxiter;
	float _M_epsilon;
	float _M_k;
	FEdgeStopping _M_edge_stop;
	int _M_neighbourhood;
	mutable mia::CHistogramFeeder<float> _M_histogramfeeder;
	mutable float _M_sigma_e;
	mutable float _M_gamma;
	mutable float _M_sigma;
};

class CAnisoDiff2DImageFilterFactory: public mia::C2DFilterPlugin {
public:
	CAnisoDiff2DImageFilterFactory();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:
	int _M_maxiter;
	float _M_epsilon;
	float _M_k;
	C2DAnisoDiff::FEdgeStopping _M_edge_stop;
	int _M_neighbourhood;
};

NS_END
