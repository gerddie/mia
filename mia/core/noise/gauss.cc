/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

 Function  CGaussNoiseGenerator::box_muller implements the Polar form
 of the Box-Muller Transformation and is

 (c) Copyright 1994, Everett F. Carter Jr.
 Permission is granted by the author to use
 this software for any application provided this
 copyright notice is preserved.

*/

/* 
   LatexBeginPlugin{Noise Generators}
   
   \subsection{Gaussian noise generator}
   \label{noise:gauss}
   
   \begin{description}
   
   \item [Plugin:] gauss
   \item [Description:] This noise generator creates random values that are distributed according to a 
        Gaussien distribution by using the Box-Muller transformation. 

   \plugtabstart
   mu & float & mean of the Gaussian distribution & 0 \\
   sigma & float & standart derivation of the Gaussian  distribution & 1 \\
   seed & unsigned & Seed value for the initialization of the pseudo-number generator, 0 indicates to 
   use the current system time value returned by the time(NULL) function. & 0 \\
   \plugtabend

   \end{description}

   LatexEnd 
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

CNoiseGeneratorPlugin::ProductPtr
CGaussNoiseGeneratorFactory::do_create() const
{

	return CNoiseGeneratorPlugin::ProductPtr(new CGaussNoiseGenerator(m_param_seed, m_param_mu, m_param_sigma));
}

const string CGaussNoiseGeneratorFactory::do_get_descr()const
{
	return "Gaussian noise generator using the Box-Muller algorithm";
}

bool CGaussNoiseGeneratorFactory::do_test()const
{
	const double mu = 1.0;
	const double sigma = 10.0;

	CGaussNoiseGenerator ng(1, mu, sigma);

	double sum1 = 0.0;
	double sum2 = 0.0;
	const size_t n = 10000000;

	size_t k = n;
	while (k--) {
		double val = ng();
		sum1 += val;
		sum2 += val * val;
	}

	cvdebug() << sum1 << " (" << sum2 << ")\n";

	sum1 /= n;
	sum2 = sqrt(( sum2 - n * sum1 * sum1) / (n-1));

	cvdebug() << sum1 << " (" << sum2 << ")\n";

	if (fabs(mu - sum1) > 0.01 || fabs(sigma  - sum2) > 0.01) {
		cvfail() << "avargaing at " << sum1 << " should be " << mu << " sigma " << sum2 << " should be " << sigma << "\n";
		return -1;
	}

	return 1;
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CGaussNoiseGeneratorFactory();
}


}
