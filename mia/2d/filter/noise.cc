/* -*- mia-c++  -*-
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
		_M_ng(ng) {
	}

	T operator ()(T x) const {
		double y = x * this->_M_ng();
		if (y < numeric_limits<T>::min())
			return numeric_limits<T>::min();
		if (y > numeric_limits<T>::max())
			return numeric_limits<T>::max();
		return T(y);
	}
private:
	const CNoiseGenerator& _M_ng;
};

template <typename T>
struct 	FNoiseAdder {
	FNoiseAdder(const CNoiseGenerator& ng):
		_M_ng(ng) {
	}

	T operator ()(T x) const {
		double y = x + this->_M_ng();
		if (y < numeric_limits<T>::min())
			return numeric_limits<T>::min();
		if (y > numeric_limits<T>::max())
			return numeric_limits<T>::max();
		return T(y);
	}
private:
	const CNoiseGenerator& _M_ng;
};

template <class T>
typename C2DNoise::result_type C2DNoise::operator () (const T2DImage<T>& data) const
{
	T2DImage<T> *result = new T2DImage<T>(data.get_size());
	cvdebug() << "C2DNoise\n";
	if (_M_modulated)
		transform(data.begin(), data.end(), result->begin(), FNoiseModulator<T>(*_M_generator));
	else
		transform(data.begin(), data.end(), result->begin(), FNoiseAdder<T>(*_M_generator));



	return P2DImage(result);
}

P2DImage C2DNoise::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DNoiseImageFilterFactory::C2DNoiseImageFilterFactory():
	C2DFilterPlugin(plugin_name),
	_M_noise_gen("gauss:mu=0,sigma=10"),
	_M_modulate(false)

{
	add_parameter("g", new CStringParameter(_M_noise_gen, false, "noise generator"));
	add_parameter("mod", new TParameter<bool>(_M_modulate,false, "additive or modulated noise"));
}

C2DFilterPlugin::ProductPtr C2DNoiseImageFilterFactory::do_create()const
{

	const CNoiseGeneratorPluginHandler::Instance& ng = CNoiseGeneratorPluginHandler::instance();

	CNoiseGeneratorPlugin::ProductPtr generator(ng.produce(_M_noise_gen.c_str()));
	if (!generator)
		throw invalid_argument(_M_noise_gen + " does not describe a noise generator");

	return C2DFilterPlugin::ProductPtr(new C2DNoise(generator, _M_modulate));
}

const string C2DNoiseImageFilterFactory::do_get_descr()const
{
	return "2D image noise filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNoiseImageFilterFactory();
}
NS_END

