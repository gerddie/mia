/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
   LatexBeginPluginDescription{Noise Generators}
   
   \subsection{Uniform noise generator}
   \label{noise:uniform}
   
   \begin{description}
   
   \item [Plugin:] uniform
   \item [Description:] This noise generator creates (pseudo) random values that are uniformly distributed over 
        a range [a,b]. 

   \plugtabstart
   a & float & Begin of noise value output range & 0 \\
   b & float & End of noise value output range & 1 \\
   seed & unsigned & Seed value for the initialization of the pseudo-number generator, 0 indicates to 
   use the current system time value returned by the time(NULL) function. & 0 \\
   \plugtabend
      

   \end{description}

   LatexEnd 
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

bool CUniformNoiseGeneratorFactory::do_test()const
{
	const double a = 1.0;
	const double b = 2.0;

	CUniformNoiseGenerator ng(1, a, b);

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
	double mu = (b+a) * 0.5;
	double sigma = sqrt((b-a) * (b-a) / 12.0);

	if (fabs(mu - sum1) > 0.01 || fabs(sigma  - sum2) > 0.01) {
		cvfail() << "averaging at " << sum1 << " should be " << mu << " sigma " << sum2 << " should be " << sigma << "\n";
		return -1;
	}

	return 1;
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CUniformNoiseGeneratorFactory();
}

NS_END
