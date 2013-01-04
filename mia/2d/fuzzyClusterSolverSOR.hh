/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_2d_fuzzyclustersolver_sor_hh
#define mia_2d_fuzzyclustersolver_sor_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/2d.hh>
#include <cstdio>
#include <stdexcept>
#include <string>

NS_MIA_BEGIN

/**
   \ingroup filtering
   \brief A solver for the fuzzy segmentation smoothness constraint 
   
   This is a solver for the smoothness constraint of the fuzzy c-means algorithm 
   described in: D.L. Pham and J.L.Prince, 
   "An adaptive fuzzy C-means algorithm for image segmentation in the presence
   of intensity inhomogeneities", Pat. Rec. Let., 20:57-68,1999
*/
class C2DFuzzyClusterSolver {
public: 
	/**
	   Contructor 
	   \param weight matrix 
	   \param lambda1 Penalize magnitude of intensity inhomogeinity correction
	   \param lambda2 Smoothness of intensity inhomogeinity correction
	   \param max_iter maximum number of iterations 
	*/
	C2DFuzzyClusterSolver(const C2DFImage& weight, double lambda1, double lambda2, int max_iter); 

	/**
	   \param[in] force matrix to drive update of gain field 
	   \param[in,out] gain filed to be updated 
	*/
	void solve(const C2DFImage& force, C2DFImage& gain); 
private: 
	
	const C2DFImage& m_weight; 
	double m_lambda1;
	double m_lambda2; 
	int m_max_iter; 
}; 

NS_MIA_END

#endif
