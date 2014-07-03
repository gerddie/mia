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

#include <mia/core/msgstream.hh> 
#include <mia/core/nccsum.hh> 

#if defined(__SSE2__)
#include <emmintrin.h>
#endif


NS_MIA_BEGIN

using std::make_pair; 

#ifdef __SSE2__

double NCCSums::value() const {
        double  result = 1.0;
        if (m_n > 0) {
                v2df sum2 = m_sum2; 
                v2df nn = {m_n, m_n };
                v2df mean = m_sum / nn; 
                v2df delta = m_sum * mean; 
                sum2 -= delta; 
		
                v2df ms2_a = _mm_unpacklo_pd(mean, sum2); 
                v2df ms2_b = _mm_unpackhi_pd(mean, sum2); 
                
                v2df prod = ms2_a * ms2_b; 
                double sumab = m_sumab - prod[0] * m_n; 

                if (prod[1] > 1e-10) {
                        result = 1.0 - sumab * sumab / prod[1]; 
                }else if (sum2[0] < 1e-5 && sum2[1] < 1e-5) {
			result = 0.0; 
		}
        }
        return result; 
}
        
std::pair<double, NCCGradHelper> NCCSums::get_grad_helper() const {
        
        std::pair<double, NCCGradHelper>  result = std::make_pair(1.0, NCCGradHelper()); 
        if (m_n > 0) {
                v2df sum2 = m_sum2; 
                v2df nn = {m_n, m_n };
                v2df mean = m_sum / nn; 
                v2df delta = m_sum * mean; 
                sum2 -= delta; 
		
                v2df ms2_a = _mm_unpacklo_pd(mean, sum2); 
                v2df ms2_b = _mm_unpackhi_pd(mean, sum2); 
                
                v2df prod = ms2_a * ms2_b; 
                
                double sumab = m_sumab - prod[0] * m_n; 

                if (prod[1] > 1e-5) {
                        result = make_pair(1.0 - sumab * sumab / prod[1], 
                                           NCCGradHelper(sumab / prod[1], sumab / sum2[0], mean[0], mean[1])); 
		} else {
			if (sum2[0] < 1e-5 && sum2[1] < 1e-5) {
				result = make_pair(0.0, NCCGradHelper()); 
			}
                }
	}
        return result; 
}
#else 
double NCCSums::value() const
{
        double  result = 1.0;
        if (m_n > 0) {
                
                double mean_a = m_suma / nn; 
                double mean_b = m_suma / nn; 
                
                double delta_a = m_suma * mean_a; 
                double delta_b = m_sumb * mean_b; 

                double suma2 = m_suma2 - mean_a; 
                double sumb2 = m_sumb2 - mean_b; 

                double help_0 = mean_a * mean_b; 
                double help_1 = suma2 * sumb2; 
                        
                double sumab = m_sumab - help_0 * n; 
                
                if (help_1 > 1e-5) {
			result = 1.0 - sumab * sumab / help_1; 
		} else if (suma2 > 1e-5 || sumb2 > 1e-5) {
			result = 1.0; 
		}
        }
        return result; 
        
}

std::pair<double, NCCGradHelper> NCCSums::get_grad_helper() const
{
        std::pair<double, NCCGradHelper>  result = std::make_pair(1.0, NCCGradHelper()); 
        if (m_n > 0) {
                
                double mean_a = m_suma / nn; 
                double mean_b = m_suma / nn; 
                
                double delta_a = m_suma * mean_a; 
                double delta_b = m_sumb * mean_b; 

                double suma2 = m_suma2 - mean_a; 
                double sumb2 = m_sumb2 - mean_b; 

                double help_0 = mean_a * mean_b; 
                double help_1 = suma2 * sumb2; 
                        
                double sumab = m_sumab - help_0 * n; 
                
                if (help_1 > 1e-10) {
                        result = make_pair(1.0 - sumab * sumab / help_1, 
                                           NCCGradHelper(2.0 * m_sumab / help_1, sumab / suma2, mean_a, mean_b)); 
                }else{
			// no correlation but at a maximum (= zero gradient) 
			if (suma2 > 1e-5 || sumb2 > 1e-5) {
				result = make_pair(1.0, NCCGradHelper(0.0, 0.0, mean_a, mean_b)); 
			}else {
				result = make_pair(0.0, NCCGradHelper(0.0, 0.0, mean_a, mean_b)); 
			}
		}
        }
        return result; 
}

#endif 

NS_MIA_END
