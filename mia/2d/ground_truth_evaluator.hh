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

#ifndef mia_2d_ground_truth_evaluator_hh
#define mia_2d_ground_truth_evaluator_hh

#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   @ingroup perf 
   \brief Main class for peudo ground truth estimation for perfusion series 
   
   This class implements the Ground Truth Estimation for perfusion series
   as given in
   Chao Li and Ying Sun, "Nonrigid Registration of Myocardial Perfusion
   MRI Using Pseudo Ground Truth", MICCAI 2009, pp 165-172

   The class make use of GroundTruthProblem and CCorrelationEvaluator in order 
   to evaluate the pseudo ground truth. 

   There is a new paper about this methos that requires a segmentation 
   for better pseudo graound truth estimation. 

*/
class  EXPORT_2D C2DGroundTruthEvaluator {
public:

	/**
	   Initialize the gound truth estimator with the given parameters. 
	   unfortunately the paper is silent about sensible values.
	 */
	C2DGroundTruthEvaluator(double alpha, double beta, double rho);
	~C2DGroundTruthEvaluator();

	/**
	   Run the estimation algorithm.
	   @param originals iriginal image series 
	   @param[in,out]  estimate the estimated pseudo ground truth. 
	 */
	void operator () (const std::vector<P2DImage>& originals,
			  std::vector<P2DImage>& estimate) const;
private:
	struct C2DGroundTruthEvaluatorImpl *impl;
};

NS_MIA_END

#endif
