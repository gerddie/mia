/*
** Copyright (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollny@cns.mpg.de>
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


#ifndef __vfluid_h
#define __vfluid_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <memory>

#include <mia/2d/2DImage.hh>
#include <mia/2d/2DVectorfield.hh>
#include <mia/2d/interpolator.hh>

#include "Pixel.hh"

#define MIN_STEP 0.1
#define MAX_STEP 4.0

#define OMEGA    1.8
#define LAMBDA   1
#define MU       1
#define PDE_NIT  200
#define TEMP_NIT 200
#define THRESH 4

NS_MIA_BEGIN


class TFluidReg {
	C2DFVectorfield v;			// velocity field
	C2DFVectorfield B;			// force field
	C2DFVectorfield u;			// displacement field
	C2DFVectorfield r;			// perturbation field

	C2DFImage Ref;
	C2DFImage Model;
	C2DFImage Template;
	SHARED_PTR(T2DInterpolator<float>)  target_interp; 


	float min_stepsize; 
	C2DBounds Start,End;

	bool final_level; 
	float epsilon; 
	float  	delta;  		// step size
	float  	stepSize;		// step size
	float  	lambda, mu;		// elasticity constants
  	float   omega;			// overrelaxation factor
	float  	a_,a, b, c, a_b,b_4;		// integration constants
	float  regrid_thresh;
public:
	TFluidReg(const C2DFImage& _Ref, const C2DFImage& _Model, float __regrid_thresh, int level, 
		  float __epsilon, float __lambda, float __mu);
	~TFluidReg();
	

	float  solveAt(unsigned int x, unsigned int y); 
	void	solvePDE(unsigned int nit); 
	//	void	solvePDE(C2DFVectorfield& v, const C2DFVectorfield& B, unsigned int nit);


	C2DFVector  forceAt(const C2DFVector &p, float s);
	void	calculateForces();
	float  calculateMismatch(bool apply); 
	bool	decreaseStep();
	void	increaseStep();
	void work(C2DFVectorfield *Shift, const C2DInterpolatorFactory& ipfac);

private:

float  perturbationAt(unsigned int x, unsigned int y); 
float  calculatePerturbation(); 
float  jacobianAt(unsigned int x, unsigned int y); 
float  calculateJacobian(); 





};


inline bool TFluidReg::decreaseStep()
{ 
	stepSize *= 0.5; 
	if (stepSize >= min_stepsize) {
		return true; 
	}else {
		stepSize = min_stepsize;
		return false;
	}
}

inline void TFluidReg::increaseStep()
{
	stepSize *= 1.5; 
	if (stepSize > MAX_STEP) 
		stepSize = MAX_STEP; 
}

#endif

NS_MIA_END
