/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

NS_MIA_USE
using std::vector;

BOOST_AUTO_TEST_CASE( test_instance )
{
       C2DPerfusionAnalysis pa(0, true, true);
}

extern const vector<float> LV_init;
extern const vector<float> RV_init;
extern const vector<float>  MOV_init;
extern const vector<float> PERF_init;
extern const vector<float> BG_init;

extern const vector<float> LV_mix;
extern const vector<float> RV_mix;
extern const vector<float> MOV_mix;
extern const vector<float> PERF_mix;
extern const vector<float> BG_mix;

const unsigned nframes = 64;
const unsigned ncomponents = 5;

const float LV_scale = 5;
const float RV_scale = 5;
const float MOV_scale = 7;
const float PERF_scale = 3;
const float BG_scale = 1;

extern const vector<float> mix[];
extern const vector<float> init[];

const float scale[5] = {
       BG_scale, LV_scale, RV_scale, MOV_scale, PERF_scale
};


/**
   This checks the scalar prduct to see whether the stwo series are
   (mostly) parallel.
*/
template <typename T>
void check_scalar_product(const vector<float>& prototype,
                          const T& value, float tol)
{
       float sump2 = 0.0f;
       float sumv2 = 0.0f;
       float sumpv = 0.0f;
       BOOST_CHECK_EQUAL(prototype.size(), value.size());

       for (unsigned i = 0; i < prototype.size(); ++i) {
              const float p =  prototype[i];
              const float v =  value[i];
              sump2 += p * p;
              sumv2 += v * v;
              sumpv += p * v;
       }

       BOOST_REQUIRE(sump2 > 0.1);
       BOOST_REQUIRE(sumv2 > 0.1);
       const float dot_normal = fabsf(sumpv) / (sump2 * sumv2);
       cvdebug() << "dot_normal = " << dot_normal << "\n";
       BOOST_CHECK_SMALL(dot_normal, tol);
}

void check_scalar_product_with_image(const vector<float>& prototype,
                                     const C2DImage& img, float tol)
{
       const C2DFImage& fimage = dynamic_cast<const C2DFImage&>(img);
       check_scalar_product(prototype, fimage, tol);
}

void check_pa(const C2DPerfusionAnalysis& pa)
{
       BOOST_CHECK(pa.has_movement());
       BOOST_CHECK_EQUAL(pa.get_RV_peak_time(), 7);
       BOOST_CHECK_EQUAL(pa.get_LV_peak_time(), 15);
       BOOST_CHECK(pa.get_RV_idx() >= 0);
       BOOST_CHECK(pa.get_LV_idx() >= 0);
       BOOST_CHECK(pa.get_perfusion_idx() >= 0);
       BOOST_CHECK(pa.get_movement_idx() >= 0);
       // this test checks how much the obtained mixing curve deviates from the
       // original 0.1 means ~5 degree off
       check_scalar_product(LV_mix, pa.get_mixing_curve(pa.get_LV_idx()), 0.01);
       check_scalar_product(RV_mix, pa.get_mixing_curve(pa.get_RV_idx()), 0.01);
       check_scalar_product(PERF_mix, pa.get_mixing_curve(pa.get_perfusion_idx()), 0.01);
       check_scalar_product(MOV_mix, pa.get_mixing_curve(pa.get_movement_idx()), 0.01);
       check_scalar_product_with_image(LV_init, *pa.get_feature_image(pa.get_LV_idx()), 0.01);
       check_scalar_product_with_image(RV_init, *pa.get_feature_image(pa.get_RV_idx()), 0.01);
       check_scalar_product_with_image(PERF_init, *pa.get_feature_image(pa.get_perfusion_idx()), 0.01);
       check_scalar_product_with_image(MOV_init, *pa.get_feature_image(pa.get_movement_idx()), 0.01);
}

BOOST_AUTO_TEST_CASE( test_series_with_movement_fixed_componenets_defl )
{
       C2DBounds size(16, 16);
       C2DFImage prototype(size);
       prototype.set_pixel_size(C2DFVector(4, 4));
       std::vector<C2DFImage> series(nframes, prototype);

       for (unsigned i = 0; i < nframes; ++i) {
              for (unsigned k = 0; k < ncomponents; ++k) {
                     const vector<float>& m = mix[k];
                     float mix_factor = m[i] * scale[k];
                     transform(init[k].begin(), init[k].end(), series[i].begin(), series[i].begin(),
                     [mix_factor](float comp, float pixel) {
                            return comp * mix_factor + pixel;
                     });
              }
       }

       auto ica_factory = produce_ica_factory("internal");
       ica_factory->set_deterministic_seed(1000);
       C2DPerfusionAnalysis pa(5, true, true);
       pa.set_approach(CIndepCompAnalysis::appr_defl);
       BOOST_CHECK(pa.run(series, *ica_factory));
       check_pa(pa);
       C2DBounds crop_start;
       P2DFilter crop_filter = pa.get_crop_filter(1.0, crop_start, C2DPerfusionAnalysis::bs_delta_feature);
       BOOST_REQUIRE(crop_filter);
       C2DPerfusionAnalysis pa0(0, true, true);
       BOOST_CHECK(pa0.run(series, *ica_factory));
       check_pa(pa0);
}


BOOST_AUTO_TEST_CASE( test_series_with_movement_fixed_componenets_symm )
{
       C2DBounds size(16, 16);
       C2DFImage prototype(size);
       prototype.set_pixel_size(C2DFVector(4, 4));
       std::vector<C2DFImage> series(nframes, prototype);

       for (unsigned i = 0; i < nframes; ++i) {
              for (unsigned k = 0; k < ncomponents; ++k) {
                     const vector<float>& m = mix[k];
                     float mix_factor = m[i] * scale[k];
                     transform(init[k].begin(), init[k].end(), series[i].begin(), series[i].begin(),
                     [mix_factor](float comp, float pixel) {
                            return comp * mix_factor + pixel;
                     });
              }
       }

       auto ica_factory = produce_ica_factory("internal");
       ica_factory->set_deterministic_seed(1);
       C2DPerfusionAnalysis pa(5, true, true);
       pa.set_approach(CIndepCompAnalysis::appr_symm);
       BOOST_CHECK(pa.run(series, *ica_factory));
       check_pa(pa);
       C2DBounds crop_start;
       P2DFilter crop_filter = pa.get_crop_filter(1.0, crop_start, C2DPerfusionAnalysis::bs_delta_feature);
       BOOST_REQUIRE(crop_filter);
       C2DPerfusionAnalysis pa0(0, true, true);
       BOOST_CHECK(pa0.run(series, *ica_factory));
       check_pa(pa0);
}

const vector<float>  LV_mix = {
       -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -1,  4, 10, 20,
       18, 16, 14, 12, 10,  9,  8,  7,
       6,  6,  5,  5,  4,  4,  4,  3,
       3,  3,  3,  2,  2,  2,  2,  1,
       1,  1,  1,  0,  0,  0,  0,  0,
       -1, -1, -1, -1, -2, -2, -2, -2,
       -3, -3, -3, -3, -4, -4, -4, -4
};

const vector<float>  RV_mix = {
       -4, -4,  -4, -4,  2,  8,  10,  25,
       18, 16,  14, 13, 13, 12,  12,  11,
       11, 10, 10,   9,  9,  8,   8,   7,
       7,  7,  6,   6,  6,  5,   5,   5,
       4,  4,  4,   4,  3,  3,   2,   2,
       2,  1,  1,   1,  0,  0,   0,   0,
       -1, -1, -1,  -1, -2, -2,  -2,  -2,
       -3, -3,	-3,  -3, -4, -4,  -4,  -4
};


const vector<float>  MOV_mix = {
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3,
       -4, -3,  0,  3,  4,  3,   0,  -3
};

const vector<float> PERF_mix = {
       -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -4,
       -4, -4, -4, -4, -4, -4, -4, -3,
       -3, -3, -3, -2, -2, -2, -1, -1,
       -1, -1,  0,  0,  0,  1,  1,  1,
       2,  2,  2,  2,  3,  3,  3,  3,
       3,  4,  4,  4,  4,  4,  4,  4
};

const vector<float>  BG_mix = {
       -1, -1, -1,  0,  1, -1,  0,  0,
       1,  0,  1,  1,  0, -1,  1, -1,
       0,  0,  1, -1,  1,  0,  1, -1,
       -1,  1,  1, -1,  1,  0,  0, -1,
       -1,  0,  1,  1,  0, -1,  1,  1,
       -1, -1,  0,  0,  0,  1,  1,  1,
       -1,  1, -1,  0,  1,  0,  1,  0,
       0,  0,  1,  1, -1, -1,  1,  1
};

const vector<float>  LV_init = {

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

const vector<float>  RV_init = {
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


const vector<float>  MOV_init = {
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

const vector<float>  PERF_init = {

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

const vector<float>  BG_init = {

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

const vector<float> mix[5] = {
       BG_mix, LV_mix, RV_mix, MOV_mix, PERF_mix
};

const vector<float> init[5] = {
       BG_init, LV_init, RV_init, MOV_init, PERF_init
};
