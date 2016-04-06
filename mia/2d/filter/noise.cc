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

#include <limits>
#include <stdexcept>
#include <mia/core/noisegen.hh>
#include <mia/2d/filter/noise.hh>

NS_BEGIN(noise_2dimage_filter)
NS_MIA_USE;
using namespace std;

static char const *plugin_name = "noise";

template <typename T>
struct 	FNoiseModulator {
	FNoiseModulator(const CNoiseGenerator& ng):
		m_ng(ng) {
	}

	T operator ()(T x) const {
		double y = x * this->m_ng();
		if (y < numeric_limits<T>::min())
			return numeric_limits<T>::min();
		if (y > numeric_limits<T>::max())
			return numeric_limits<T>::max();
		return T(y);
	}
private:
	const CNoiseGenerator& m_ng;
};

template <typename T>
struct 	FNoiseAdder {
	FNoiseAdder(const CNoiseGenerator& ng):
		m_ng(ng) {
	}

	T operator ()(T x) const {
		double y = x + this->m_ng();
		if (y < numeric_limits<T>::min())
			return numeric_limits<T>::min();
		if (y > numeric_limits<T>::max())
			return numeric_limits<T>::max();
		return T(y);
	}
private:
	const CNoiseGenerator& m_ng;
};

template <class T>
typename C2DNoise::result_type C2DNoise::operator () (const T2DImage<T>& data) const
{
	T2DImage<T> *result = new T2DImage<T>(data.get_size(), data);
	cvdebug() << "C2DNoise\n";
	if (m_modulated)
		transform(data.begin(), data.end(), result->begin(), FNoiseModulator<T>(*m_generator));
	else
		transform(data.begin(), data.end(), result->begin(), FNoiseAdder<T>(*m_generator));



	return P2DImage(result);
}

P2DImage C2DNoise::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DNoiseImageFilterFactory::C2DNoiseImageFilterFactory():
	C2DFilterPlugin(plugin_name),
	m_modulate(false)

{
	add_parameter("g", make_param(m_noise_gen, "gauss:mu=0,sigma=10", false, "noise generator"));
	add_parameter("mod", new CBoolParameter(m_modulate, false, "additive or modulated noise"));
}

C2DFilter *C2DNoiseImageFilterFactory::do_create()const
{
	return new C2DNoise(m_noise_gen, m_modulate);
}

const string C2DNoiseImageFilterFactory::do_get_descr()const
{
	return "2D image noise filter: add additive or modulated noise to an image";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNoiseImageFilterFactory();
}
NS_END

