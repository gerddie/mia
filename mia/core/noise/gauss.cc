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

/*
 Function  CGaussNoiseGenerator::box_muller implements the Polar form
 of the Box-Muller Transformation and is

 (c) Copyright 1994, Everett F. Carter Jr.
 Permission is granted by the author to use
 this software for any application provided this
 copyright notice is preserved.

*/

#include <cmath>

#include <limits>
#include <vector>


#include <mia/core/noise/gauss.hh>

NS_BEGIN(gauss_noise_generator)
using namespace std;
using namespace mia;

static const size_t center = 1024;

double CGaussNoiseGenerator::box_muller()const	/* normal random variate generator */
{
	double x1, x2, w, y1;

	if (m_use_last)	{
		y1 = m_y2;
		m_use_last = false;
	} else {
		do {
			x1 = 2.0 * ranf() - 1.0;
			x2 = 2.0 * ranf() - 1.0;
			w = x1 * x1 + x2 * x2;
		} while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		m_y2 = x2 * w;
		m_use_last = true;
	}

	return( m_mu + y1 * m_sigma );
}

CGaussNoiseGenerator::CGaussNoiseGenerator(unsigned int seed, double mu, double sigma):
	CNoiseGenerator(seed),
	m_mu(mu),
	m_sigma( sigma ),
	m_use_last(false)
{
}

double CGaussNoiseGenerator::get() const
{
	return box_muller();
}

CGaussNoiseGeneratorFactory::CGaussNoiseGeneratorFactory():
	CNoiseGeneratorPlugin("gauss"),
	m_param_seed(0),
	m_param_mu(0.0f),
	m_param_sigma(1.0f)
{
	add_parameter("mu", new CFloatParameter(m_param_mu, -numeric_limits<float>::max(),
								       numeric_limits<float>::max(),
								       false, "mean of distribution"));

	add_parameter("sigma", new CFloatParameter(m_param_sigma, 0.0f,
								       numeric_limits<float>::max(),
								       false, "standart derivation of distribution"));

	add_parameter("seed", new CUIntParameter(m_param_seed, 0,   numeric_limits<unsigned int>::max(),
						  false, "set random seed (0=init based on system time)"));

}

CNoiseGenerator *
CGaussNoiseGeneratorFactory::do_create() const
{

	return new CGaussNoiseGenerator(m_param_seed, m_param_mu, m_param_sigma);
}

const string CGaussNoiseGeneratorFactory::do_get_descr()const
{
	return "This noise generator creates random values that are distributed according to a "
		"Gaussien distribution by using the Box-Muller transformation.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CGaussNoiseGeneratorFactory();
}


}
