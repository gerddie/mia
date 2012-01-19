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
#include <cmath>
#include <mia/3d/nfg.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN
using namespace std;

EXPORT_3D float get_jump_level(const C3DImage& image, float noise_level)
{
	double sum = 0.0;
	C3DFVectorfield gradient = get_gradient(image);
	for (C3DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g) {
		sum += g->norm();
	}

	return noise_level * sum / gradient.size();
}

EXPORT_3D float get_jump_level(const C3DImage& image)
{
	return get_jump_level(image, get_noise_level(image));
}

EXPORT_3D C3DFVectorfield get_nfg_n(const C3DImage& image,  float noise_level)
{

	C3DFVectorfield gradient = get_gradient(image);

	double sum = 0.0;
	for (C3DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g)
		sum += g->norm();

	const float jump_level = noise_level * sum / gradient.size();
	const float e2 = jump_level * jump_level;

	if (e2 != 0.0) {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + e2);
	} else {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
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
	float operator () (const T3DImage<T>& data) const {

		double sum1 = 0.0;
		double sum2 = 0.0;
		if (data.get_size().x < 3 ||
		    data.get_size().y < 3 ||
		    data.get_size().z < 3)
			throw invalid_argument("input too small to support noise level estimation\n");

		const size_t zn =  data.get_size().z - 1;
		const size_t yn =  data.get_size().y - 1;
		const size_t xn =  data.get_size().x - 1;
		const size_t xs =  data.get_size().x;
		const size_t xys = xs * data.get_size().y;
		typename T3DImage<T>::const_iterator i = data.begin();

		for (size_t z = 0; z < zn; ++z) {
			for (size_t y = 0; y < yn; ++y) {
				for (size_t x = 0; x < xn; ++x, ++i) {
					const double delta1 = double(*i) - double(i[1]);
					const double delta2 = double(*i) - double(i[xs]);
					const double delta3 = double(*i) - double(i[xys]);
					sum2 += delta1 * delta1 + delta2 * delta2 + delta3 * delta3;
					sum1 += std::fabs(delta1) + std::fabs(delta2) + std::fabs(delta3);
				}
				const double delta3 = double(*i) - double(i[xys]);
				const double delta2 = double(*i) - double(i[xs]);
				sum2 += delta3 * delta3 + delta2 * delta2;
				sum1 += std::fabs(delta3) + std::fabs(delta2);
				++i;
			};
			for (size_t x = 0; x < xn; ++x, ++i) {
				const double delta3 = double(*i) - double(i[xys]);
				const double delta1 = double(*i) - double(i[1]);
				sum2 += delta1 * delta1 + delta3 * delta3;
				sum1 += std::fabs(delta1) + std::fabs(delta3);
			}
			const double delta3 = double(*i) - double(i[xys]);
			sum2 += delta3 * delta3;
			sum1 += std::fabs(delta3);
			++i;
		}
		for (size_t y = 0; y < yn; ++y) {
			for (size_t x = 0; x < xn; ++x, ++i) {
				const double delta1 = double(*i) - double(i[1]);
				const double delta2 = double(*i) - double(i[xs]);
				sum2 += delta1 * delta1 + delta2 * delta2;
				sum1 += std::fabs(delta1) + std::fabs(delta2);
			}
			const double delta2 = double(*i) - double(i[xs]);
			sum2 += delta2 * delta2;
			sum1 += std::fabs(delta2);
			++i;
		};
		for (size_t x = 0; x < xn; ++x, ++i) {
			const double delta1 = double(*i) - double(i[1]);
			sum2 += delta1 * delta1;
			sum1 += std::fabs(delta1);


		}
		//	++i;
		double n = 3 * zn * xn * yn  + 2 * (zn * xn  +  yn * zn + xn * yn) + xn + yn + zn;


		return sqrt((sum2 - sum1 * sum1 / n) / (n - 1)); // / (range + 1);
	}
};


EXPORT_3D float get_noise_level(const C3DImage& image)
{
	CNoiseLevel f;
	return mia::filter(f, image);
}


EXPORT_3D C3DFVectorfield get_nfg_j(const C3DImage& image,  float jump_level2)
{
	assert(jump_level2 >= 0.0f);

	C3DFVectorfield gradient = get_gradient(image);

	if (jump_level2 != 0.0f) {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + jump_level2);
	}else {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
			float n2 = g->norm2();
			if (n2 > 0.0f)
				*g /= sqrt(g->norm2());
		}
	}

	return gradient;
}


EXPORT_3D C3DFVectorfield get_nfg(const C3DImage& image)
{

	float noise_level = get_noise_level(image);
	C3DFVectorfield gradient = get_gradient(image);

	double sum = 0.0;
	for (C3DFVectorfield::const_iterator g = gradient.begin(); g != gradient.end(); ++g)
		sum += g->norm();

	const float jump_level = noise_level * sum / gradient.size();
	const float e2 = jump_level * jump_level;

	if (e2 != 0.0f) {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g)
			*g /= sqrt(g->norm2() + e2);
	}else {
		for (C3DFVectorfield::iterator g = gradient.begin(); g != gradient.end(); ++g) {
			float n2 = g->norm2();
			if (n2 > 0.0f)
				*g /= sqrt(g->norm2());
		}
	}

	return gradient;
}


NS_MIA_END

