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

#include <mia/internal/autotest.hh>
#include <mia/core/noisegen.hh>
#include <mia/core/histogram.hh>

#include <stdexcept>
#include <cmath>

#include <mia/core/pwh.hh>


NS_MIA_USE
using namespace std; 
namespace bfs=::boost::filesystem;

struct FNormalDistribution {
	FNormalDistribution(float mean, float sigma) :
		m_mean(mean), 
		m_sigma(sigma), 
		m_w1(1.0 / (2.0 * m_sigma * m_sigma)), 
		m_w2(sqrt( m_w1 / M_PI))

	{

	}
	
	float operator()(float x) const {
		const float h = x - m_mean; 
		return m_w2 * exp( - m_w1 * h * h); 
	}

	float dndx(float x) const {
		const float h = x - m_mean; 
		return - 2.0 * m_w2 * m_w1 * h * exp( - m_w1 * h * h); 
	}

	float m_mean; 
	float m_sigma; 
	float m_w1; 
	float m_w2; 
}; 

BOOST_AUTO_TEST_CASE ( test_parzen_window_histogram ) 
{
	const size_t size = 16000; 

	vector<double> data; 

	list< bfs::path> noisesearchpath;
	noisesearchpath.push_back(bfs::path("noise"));
	CNoiseGeneratorPluginHandler::set_search_path(noisesearchpath);
	
	const CNoiseGeneratorPluginHandler::Instance&  ngp = CNoiseGeneratorPluginHandler::instance(); 

	CNoiseGeneratorPlugin::ProductPtr ng[3]; 
	ng[1] = ngp.produce("gauss:mu=127,sigma=16,seed=1"); 
	ng[2] = ngp.produce("gauss:mu= 63,sigma=16,seed=1"); 
	ng[0] = ngp.produce("gauss:mu=195,sigma=16,seed=1"); 
	

	FNormalDistribution n1(127, 16); 
	FNormalDistribution n2( 63, 16); 
	FNormalDistribution n3(195, 16); 
	
	for (size_t k = 0; k < 3; ++k) {
		const CNoiseGenerator& g = *ng[k]; 
		size_t i = 0;
		while ( i < size ) {
			double h = g(); 
			if ( h >= 0.0 && h < 256.0) {
				data.push_back(h); 
				++i; 
			}
		}
	}

	CParzenWindowHistogram h(0, 256, 64, data);
	for (size_t x = 0; x < 256; ++x)  
		BOOST_CHECK_CLOSE(1.0 + h[x], 1.0 + (n1(x) + n2(x) + n3(x))/3.0,1); 


	for (size_t x = 0; x < 256; ++x)  
		BOOST_CHECK_CLOSE( 1.0 + h.derivative(x),  1.0 + (n1.dndx(x) + n2.dndx(x) + n3.dndx(x))/3.0,1); 
	
}
