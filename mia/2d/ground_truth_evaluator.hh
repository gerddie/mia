/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                    Gert Wollny <gw.fossdev @ gmail.com>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef mia_2d_ground_truth_evaluator_hh
#define mia_2d_ground_truth_evaluator_hh

#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

/**
   This class implements the Ground Truth for perfusion series 
   as given in 
   Chao Li and Ying Sun, "Nonrigid Registration of Myocardial Perfusion
   MRI Using Pseudo Ground Truth", MICCAI 2009, pp 165-172
*/

class C2DGroundTruthEvaluator {
public: 
	C2DGroundTruthEvaluator(double alpha, double beta, double rho); 
	~C2DGroundTruthEvaluator(); 
	std::vector<P2DImage> operator () (const std::vector<P2DImage>& originals) const; 
private: 
	struct C2DGroundTruthEvaluatorImpl *impl; 
}; 

NS_MIA_END

#endif
