/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/core/noise/uniform.hh>


NS_BEGIN(uniform_noise_generator)
using namespace std;
using namespace mia;

static const size_t center = 1024;

CUniformNoiseGenerator::CUniformNoiseGenerator(unsigned int seed, double a, double b):
	CNoiseGenerator(seed),
	m_a(a),
	m_step(b-a)
{
}

double CUniformNoiseGenerator::get() const
{
	return m_step * ranf() + m_a;
}


CUniformNoiseGeneratorFactory::CUniformNoiseGeneratorFactory():
	CNoiseGeneratorPlugin("uniform"),
	m_param_seed(0),
	m_param_a(0),
	m_param_b(1)
{
	add_parameter("a", new CTParameter<float>(m_param_a, false, "lower bound if noise range"));

	add_parameter("b", new CTParameter<float>(m_param_b, false, "higher bound if noise range"));
	add_parameter("seed", new CUIBoundedParameter(m_param_seed, EParameterBounds::bf_min_closed, {0}, 
						     false, "set random seed (0=init based on system time)"));

}

CNoiseGenerator *CUniformNoiseGeneratorFactory::do_create()const
{
	return new CUniformNoiseGenerator(m_param_seed, m_param_a, m_param_b);
}

const string CUniformNoiseGeneratorFactory::do_get_descr()const
{
	return "Uniform noise generator using C stdlib rand()";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CUniformNoiseGeneratorFactory();
}

NS_END
