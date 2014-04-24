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



CLNCC3DImageCost::CLNCC3DImageCost(int hw):
m_hwidth(hw)
{
}

double CLNCC3DImageCost::do_value(const Data& a, const Data& b, const Mask& m) const
{
        
}

inline pair<C3DBounds, C3DBounds> prepare_range(const C3DBounds& size, int cx, int cy, int cz, int hw) 
{

	int zb = cz - hw;
	if (zb < 0) zb = 0; 
	unsigned ze = cz + hw + 1; 
	if (ze > size.z) ze = size.z; 
	
	int yb = cy - hw;
	if (yb < 0) yb = 0; 
	unsigned ye = cy + hw + 1; 
	if (ye > size.y) ye = size.y; 
	
	int xb = cx - hw;
	if (xb < 0) xb = 0; 
	unsigned xe = cx + hw + 1; 
	if (xe > size.x) xe = size.x; 
	
	return make_pair(C3DBounds(xb,yb,zb), C3DBounds(xe,ye,ze)); 
}


double CLNCC3DImageCost::do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const
{
        const int hw = m_hwidth; 
        
        auto ag = get_gradient(a); 

        auto evaluate_local_cost = [hw, &a, &b, &m, &force, &ag](const tbb::blocked_range<size_t>& range, float result) {
                float lresult = 0.0; 
                const int count = 0; 
                const int max_length = 2 * hw +1; 
                vector<float> a_buffer( max_length * max_length * max_length); 
                vector<float> b_buffer( max_length * max_length * max_length); 

                for (auto z = range.begin(); z != range.end(); ++z) {
                        
                        auto iforce = force.begin_at(0,0,z);
                        auto imask = force.begin_at(0,0,z);
                        auto ig = ag.begin_at(0,0,z);
                        auto ifixed = b.begin_at(0,0,z);
                        
                        for (size_t y = 0; y < data.get_size().y; ++y)
                                for (size_t x = 0; x < data.get_size().x; ++x, ++iforce, ++imask, ++ig, ++ifixed) {
                                        if (!*imask) 
                                                continue; 
                                        
                                        auto c_block = prepare_range(a.get_size(), x, y, z, hw); 
                                        auto ia = a.begin_range(c_block.first,c_block.second); 
                                        auto ea = a.end_range(c_block.first,c_block.second); 
                                        auto ib = b.begin_range(c_block.first,c_block.second); 
                                        auto im = m.begin_range(c_block.first,c_block.second); 
                                        

                                        float suma = 0.0; 
                                        float sumb = 0.0; 
                                        float suma2 = 0.0; 
                                        float sumb2 = 0.0; 
                                        float sumab = 0.0; 
                                        long n = 0; 
                                        
                                        // make a local copy 
                                        while (ia != ea) {
                                                if (*im) {
                                                        a_buffer[n] = *ia; 
                                                        b_buffer[n] = *ib; 
                                                        suma += *ia;
                                                        sumb += *ib;
                                                        ++n;
                                                }
                                                ++ia; ++ib; ++im; 
                                        }
                                        if (n > 1) {
                                                const float mean_a = suma/n; 
                                                const float mean_b = sumb/n;
                                                
                                                // strip mean and evaluate cross correlation 
                                                for (int i = 0; i < n; ++i) {
                                                        const float a_ = a_buffer[i] - mean_a; 
                                                        suma2 += a_ * a_; 
                                                        const float b_ = b_buffer[i] - mean_b; 
                                                        sumb2 += b_ * b_; 
                                                        sumab += a_ * b_; 
                                                }
                                        
                                                float suma2_sumb2 = suma2 * sumb2;

                                                if (suma2_sumb2 > 1e-5) {
                                                        
                                                        lresult += sumab * sumab / suma2_sumb2; 
                                                        ++count;
                                                        
                                                        *iforce = static_cast<float>(2.0 * sumab / suma2_sumb2 * ( *ib -  sumab / suma2 * *ia)) * *ig; 
                                                }
                                        }
                                }
                }
                if (count > 0) 
                        return result + lresult / count; 
                else 
                        return result; 
        };
        
        return parallel_reduce(tbb::blocked_range<size_t>(0, a.get_size().z, 1), run_slice, 0.0);
}


void CLNCC3DImageCost::post_set_reference(const Data& ref)
{
}
