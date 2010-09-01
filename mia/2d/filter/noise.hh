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
#include <stdexcept>
#include <mia/2d/2dfilter.hh>
#include <mia/core/noisegen.hh>

NS_BEGIN(noise_2dimage_filter)

class C2DNoise: public mia::C2DFilter {
	mia::CNoiseGeneratorPlugin::ProductPtr _M_generator;
	bool _M_modulated;
public:
	C2DNoise(mia::CNoiseGeneratorPlugin::ProductPtr generator, bool modulate):
		_M_generator(generator),
		_M_modulated(modulate)
	{
	}

	template <class T>
	typename C2DNoise::result_type operator () (const mia::T2DImage<T>& data) const ;
private:
	virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;
};


class C2DNoiseImageFilterFactory: public mia::C2DFilterPlugin {
public:
	C2DNoiseImageFilterFactory();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const;
private:
	std::string _M_noise_gen;
	bool _M_modulate;
};

NS_END

