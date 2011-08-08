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


#include <mia/core/noisegen.hh>

NS_BEGIN(gauss_noise_generator)

class CGaussNoiseGenerator: public mia::CNoiseGenerator {
public:
	CGaussNoiseGenerator(unsigned int seed, double mu, double sigma); 
private: 
	virtual double get() const; 
	double box_muller() const;
	
	double m_mu; 		
	double m_sigma; 
	mutable bool m_use_last; 
	mutable double m_y2; 
};


class CGaussNoiseGeneratorFactory: public mia::CNoiseGeneratorPlugin {
public: 
	CGaussNoiseGeneratorFactory(); 
private: 
	mia::CNoiseGenerator *do_create() const; 
	virtual const std::string do_get_descr()const; 
	bool do_test()const; 
	unsigned int m_param_seed; 
	float m_param_mu; 
	float m_param_sigma;

}; 

NS_END
