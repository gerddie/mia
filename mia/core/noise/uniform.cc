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
	m_step(1.0 / (b-a))
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
	add_parameter("a", new CFloatParameter(m_param_a, -numeric_limits<float>::max(),
								       numeric_limits<float>::max(),
								       false, "lower bound if noise range"));

	add_parameter("b", new CFloatParameter(m_param_b, -numeric_limits<float>::max(),
								       numeric_limits<float>::max(),
								       false, "higher bound if noise range"));
	add_parameter("seed", new CUIntParameter(m_param_seed, 0,   numeric_limits<unsigned int>::max(),
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
