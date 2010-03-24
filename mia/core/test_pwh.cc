/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2009 -.2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/internal/autotest.hh>
#include <boost/lambda/lambda.hpp>
#include <mia/core/noisegen.hh>
#include <mia/core/histogram.hh>

#include <stdexcept>
#include <cmath>

#include <mia/core/pwh.hh>


NS_MIA_USE
using namespace std; 
using boost::lambda::_1; 
namespace bfs=::boost::filesystem;

struct FNormalDistribution {
	FNormalDistribution(float mean, float sigma) :
		_M_mean(mean), 
		_M_sigma(sigma), 
		_M_w1(1.0 / (2.0 * _M_sigma * _M_sigma)), 
		_M_w2(sqrt( _M_w1 / M_PI))

	{

	}
	
	float operator()(float x) const {
		const float h = x - _M_mean; 
		return _M_w2 * exp( - _M_w1 * h * h); 
	}

	float dndx(float x) const {
		const float h = x - _M_mean; 
		return - 2.0 * _M_w2 * _M_w1 * h * exp( - _M_w1 * h * h); 
	}

	float _M_mean; 
	float _M_sigma; 
	float _M_w1; 
	float _M_w2; 
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
