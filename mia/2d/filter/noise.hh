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
#include <mia/2d/filter.hh>
#include <mia/core/noisegen.hh>

NS_BEGIN(noise_2dimage_filter)

class C2DNoise: public mia::C2DFilter {
	mia::PNoiseGenerator m_generator;
	bool m_modulated;
public:
	C2DNoise(mia::PNoiseGenerator generator, bool modulate):
		m_generator(generator),
		m_modulated(modulate)
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
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;
private:
	mia::PNoiseGenerator m_noise_gen;
	bool m_modulate;
};

NS_END

