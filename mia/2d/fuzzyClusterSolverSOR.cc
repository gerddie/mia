/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/2d/fuzzyClusterSolverSOR.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN
C2DFuzzyClusterSolver::C2DFuzzyClusterSolver(const C2DFImage& weight, double lambda1, double lambda2, int max_iter):
	m_weight(weight), 
	m_lambda1(lambda1), 
	m_lambda2(lambda2), 
	m_max_iter(max_iter)
{
}

void C2DFuzzyClusterSolver::solve(const C2DFImage& force, C2DFImage& gain)
{
	assert(force.get_size() == gain.get_size()); 
	assert(m_weight.get_size() == gain.get_size()); 

	const float lambda = 4 * m_lambda1 + 20 * m_lambda2; 
	const float omega = 1.75; 
	const float eps = 0.00001; 

	auto size = m_weight.get_size(); 
	const int dx = size.x; 
	float n0 = 0.0; 
	for (int i = 0; i < m_max_iter; ++i) {
		float norm = 0.0; 
		for(size_t y = 2; y < size.y - 2; ++y) {
			auto igain = gain.begin_at(2, y); 
			auto iforce = force.begin_at(2, y); 
			auto iweight = m_weight.begin_at(2, y); 
			
			for(size_t x = 2; x < size.x - 2; ++x, ++igain, ++iforce, ++iweight) {
				float d = *iweight + lambda; 
				if ( fabs(d) < 1e-8) 
					continue; 

				float m1 = igain[-dx] + igain[-1] + igain[1] + igain[dx];  
				float m2 = igain[-dx -1 ] + igain[-dx + 1]
					+ igain[dx - 1] + igain[dx + 1];  
				float m3 = igain[-2 * dx] + igain[-2] + igain[2] + igain[2 * dx];  

				float h = m_lambda1 * m1 + 
					m_lambda2 * (8 * m1 - 2 * m2 - m3); 
				float r = omega * (d * *igain - h - *iforce) / d; 
				norm += fabs(r); 
				*igain -= r; 
				
			}
		}
		cvinfo() << "FuzzyComputeGain: [" << i << "] n=" << norm <<"\n"; 
		if (i == 0) 
			n0 = norm; 
		else 
			if (norm / n0 < eps || norm < 0.0001) 
				break; 
		
	}
}

NS_MIA_END
