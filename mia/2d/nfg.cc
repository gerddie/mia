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

#include <stdexcept>
#include <mia/2d/nfg.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using namespace std;

EXPORT_2D float get_jump_level(const C2DImage& image, float noise_level)
{
	double sum = 0.0;
	C2DFVectorfield gradient = get_gradient(image);
	for (C2DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g) {
		sum += g->norm();
	}

	return noise_level * sum / gradient.size();
}

EXPORT_2D float get_jump_level(const C2DImage& image)
{
	return get_jump_level(image, get_noise_level(image));
}

EXPORT_2D C2DFVectorfield get_nfg_n(const C2DImage& image,  float noise_level)
{

	C2DFVectorfield gradient = get_gradient(image);

	double sum = 0.0;
	for (C2DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g)
		sum += g->norm();

	const float jump_level = noise_level * sum / gradient.size();
	const float e2 = jump_level * jump_level;

	if (e2 != 0.0) {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + e2);
	} else {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
			float n2 = g->norm2();
			if (n2 > 0)
				*g /= sqrt(g->norm2());
		}
	}

	return gradient;
}

class CNoiseLevel: public TFilter<float> {
public:
	template <typename  T>
	float operator () (const T2DImage<T>& data) const {

		double sum1 = 0.0;
		double sum2 = 0.0;
		if (data.get_size().x < 3 ||
		    data.get_size().y < 3)
			throw invalid_argument("input too small to support noise level estimation\n");

		const size_t yn =  data.get_size().y - 1;
		const size_t xn =  data.get_size().x - 1;
		const size_t xs =  data.get_size().x;
		typename T2DImage<T>::const_iterator i = data.begin();

		for (size_t y = 0; y < yn; ++y) {
			for (size_t x = 0; x < xn; ++x, ++i) {
				double delta1 = double(*i) - double(i[1]);
				double delta2 = double(*i) - double(i[xs]);
				sum2 += delta2 * delta2 + delta1 * delta1;
				sum1 += fabs(delta1) + fabs(delta2);
			}

			double delta2 = double(*i) - double(i[xs]);
			sum2 += delta2 * delta2;
			sum1 += fabs(delta2);
			++i;
		};
		for (size_t x = 0; x < xn; ++x, ++i) {

			double delta1 = double(*i) - double(i[1]);
			sum2 += delta1 * delta1;
			sum1 += fabs(delta1);

		}


		double n = 2 * xn * yn + xn + yn;


		return sqrt((sum2 - sum1 * sum1 / n) / (n - 1)); // / (range + 1);
	}
};


EXPORT_2D float get_noise_level(const C2DImage& image)
{
	CNoiseLevel f;
	return mia::filter(f, image);
}


EXPORT_2D C2DFVectorfield get_nfg_j(const C2DImage& image,  float jump_level2)
{
	assert(jump_level2 >= 0.0f);

	C2DFVectorfield gradient = get_gradient(image);

	if (jump_level2 != 0.0f) {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + jump_level2);
	}else {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
			float n2 = g->norm2();
			if (n2 > 0.0f)
				*g /= sqrt(g->norm2());
		}
	}

	return gradient;
}


EXPORT_2D C2DFVectorfield get_nfg(const C2DImage& image)
{

	float noise_level = get_noise_level(image);
	C2DFVectorfield gradient = get_gradient(image);

	double sum = 0.0;
	for (C2DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g)
		sum += g->norm();

	const float jump_level = noise_level * sum / gradient.size();
	const float e2 = jump_level * jump_level;

	if (e2 != 0.0f) {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + e2);
	}else {
		for (C2DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
			float n2 = g->norm2();
			if (n2 > 0.0f)
				*g /= sqrt(g->norm2());
		}
	}

	return gradient;
}


NS_MIA_END

