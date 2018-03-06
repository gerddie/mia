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

#include <limits>
#include <mia/core/histogram.hh>
#include <mia/2d/filter.hh>

NS_BEGIN(aniso_2dimage_filter)

float psi_tuckey(float x, float sigma);
float psi_pm1(float x, float sigma);
float psi_pm2(float x, float sigma);
float psi_test(float x, float sigma);


class C2DAnisoDiff: public mia::C2DFilter
{
public:
       typedef float (*FEdgeStopping)(float x, float sigma);

       C2DAnisoDiff(int maxiter, float epsilon, float k, FEdgeStopping edge_stop, int neighbourhood);

       template <class T>
       result_type operator () (const mia::T2DImage<T>& image) const;
protected:
       virtual mia::P2DImage do_filter(const mia::C2DImage& image) const;

       float diffuse(mia::C2DFImage& dest, const mia::C2DFImage& src)const;

       float estimate_MAD(const mia::C2DFImage& data)const;

       void create_histogramfeeder(const mia::C2DFImage& data) const;
       void update_gamma_sigma(const mia::C2DFImage& src)const;

       int m_maxiter;
       float m_epsilon;
       float m_k;
       FEdgeStopping m_edge_stop;
       int m_neighbourhood;
       mutable mia::THistogramFeeder<float> m_histogramfeeder;
       mutable float m_sigma_e;
       mutable float m_gamma;
       mutable float m_sigma;
};

class CAnisoDiff2DImageFilterFactory: public mia::C2DFilterPlugin
{
public:
       CAnisoDiff2DImageFilterFactory();
       virtual mia::C2DFilter *do_create()const;
       virtual const std::string do_get_descr()const;
private:
       int m_maxiter;
       float m_epsilon;
       float m_k;
       C2DAnisoDiff::FEdgeStopping m_edge_stop;
       int m_neighbourhood;
};

NS_END
