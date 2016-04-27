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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>


#include <mia/2d/perfusion.hh>
#include <mia/core/ica.hh>

NS_MIA_USE

BOOST_AUTO_TEST_CASE( test_instance ) 
{
	C2DPerfusionAnalysis pa(0, true, true); 
}

extern const float LV_init[];
extern const float RV_init[];
extern const float MOV_init[];
extern const float PERF_init[];
extern const float BG_init[];

extern const float LV_mix[];
extern const float RV_mix[];
extern const float MOV_mix[];
extern const float PERF_mix[];
extern const float BG_mix[];

const unsigned nframes = 64;
const unsigned ncomponents = 5; 

const float LV_scale = 5;
const float RV_scale = 5;
const float MOV_scale = 7;
const float PERF_scale = 3;
const float BG_scale = 1;

const float scale[5] = {
	BG_scale, LV_scale, RV_scale, MOV_scale, PERF_scale
}; 

const float *mix[5] = {
	BG_mix, LV_mix, RV_mix, MOV_mix, PERF_mix
}; 

const float *init[5] = {
	BG_init, LV_init, RV_init, MOV_init, PERF_init
}; 


BOOST_AUTO_TEST_CASE( test_series_with_movement )
{
	C2DBounds size(16,16);
	std::vector<C2DFImage> series(nframes, C2DFImage(size));

	
	for (unsigned i = 0; i < nframes; ++i) {
		for (unsigned k = 0; k < ncomponents; ++k) {
			float mix_factor = mix[k][i] * scale[k];
			transform(init[k], init[k] + 256, series[i].begin(), series[i].begin(),
				  [mix_factor](float comp, float pixel) {
					  return comp * mix_factor + pixel; 
				  }); 
		}
	}
	
	CICAAnalysisITPPFactory ica_factory;
	
	C2DPerfusionAnalysis pa(5, true, true);

	BOOST_CHECK(pa.run(series, ica_factory));
	BOOST_CHECK(pa.has_movement()); 
	BOOST_CHECK_EQUAL(pa.get_RV_peak_time(), 7);
	BOOST_CHECK_EQUAL(pa.get_LV_peak_time(), 15);

	
	
	
}

const float LV_mix[64] = {
	-4, -4, -4, -4, -4, -4, -4, -4,
	-4, -4, -4, -4, -1,  4, 10, 20,
	18, 16, 14, 12, 10,  9,  8,  7,
	 6,  6,  5,  5,  4,  4,  4,  3,
	 3,  3,  3,  2,  2,  2,  2,  1,
	 1,  1,  1,  0,  0,  0,  0,  0,
	-1, -1, -1, -1, -2, -2, -2, -2,
	-3, -3, -3, -3, -4, -4, -4, -4
}; 

const float RV_mix[64] = {
	-4, -4,  -4, -4,  2,  8,  10,  25,
	18, 16,  14, 13, 13, 12,  12,  11,
	11, 10, 10,   9,  9,  8,   8,   7, 
	 7,  7,  6,   6,  6,  5,   5,   5,
	 4,  4,  4,   4,  3,  3,   2,   2,
  	 2,  1,  1,   1,  0,  0,   0,   0,
	-1, -1, -1,  -1, -2, -2,  -2,  -2,
	-3, -3,	-3,  -3, -4, -4,  -4,  -4 
}; 


const float MOV_mix[64] = {
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3,
	-4, -3,  0,  3,  4,  3,   0,  -3
}; 

extern const float PERF_mix[] = {
	-4, -4, -4, -4, -4, -4, -4, -4,
	-4, -4, -4, -4, -4, -4, -4, -4,
	-4, -4, -4, -4, -4, -4, -4, -4,
	-4, -4, -4, -4, -4, -4, -4, -3,
	-3, -3, -3, -2, -2, -2, -1, -1,
	-1, -1,  0,  0,  0,  1,  1,  1,
	 2,  2,  2,  2,  3,  3,  3,  3, 
	 3,  4,  4,  4,  4,  4,  4,  4
}; 

extern const float BG_mix[] = {
	-1, -1, -1,  0,  1, -1,  0,  0,
	 1,  0,  1,  1,  0, -1,  1, -1,
	 0,  0,  1, -1,  1,  0,  1, -1,
	-1,  1,  1, -1,  1,  0,  0, -1,
	-1,  0,  1,  1,  0, -1,  1,  1,
	-1, -1,  0,  0,  0,  1,  1,  1,
	-1,  1, -1,  0,  1,  0,  1,  0,
	 0,  0,  1,  1, -1, -1,  1,  1
}; 

const float LV_init[256] = {

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const float RV_init[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


const float MOV_init[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const float PERF_init[256] = {

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const float BG_init[256] = {

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
	0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0,
	0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
