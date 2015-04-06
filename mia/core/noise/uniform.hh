/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <cmath>

#include <limits>
#include <vector>

#include <mia/core/noisegen.hh>


NS_BEGIN(uniform_noise_generator)
class CUniformNoiseGenerator: public mia::CNoiseGenerator {
public:
	CUniformNoiseGenerator(unsigned int seed, double a, double b); 
private: 
	virtual double get() const; 
		double m_a; 
	double m_step; 
};

class CUniformNoiseGeneratorFactory: public mia::CNoiseGeneratorPlugin {
public: 
	CUniformNoiseGeneratorFactory(); 
private: 
	mia::CNoiseGenerator *do_create() const; 
	virtual const std::string do_get_descr()const; 
	unsigned int m_param_seed; 
	float m_param_a; 
	float m_param_b; 
}; 

NS_END
