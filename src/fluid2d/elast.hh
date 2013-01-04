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

#ifndef __elast_hh
#define __elast_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/2d/image.hh>
#include <mia/2d/interpolator.hh>

#include "Pixel.hh"

#define MIN_FORCE_MULT 0.0000001
#define MAX_STEP 4.0

#define OMEGA    1.8
#define PDE_NIT  200
#define TEMP_NIT 200
#define THRESH 4

NS_MIA_BEGIN

class TElastReg {
	C2DFVectorfield ut;
	C2DFVectorfield B;			// force field
	C2DFVectorfield u;			// displacement field
	C2DFVectorfield r;			// perturbation field

	const C2DFImage& Ref;
	const C2DFImage& Model;
	C2DFImage Template;
	C2DFImage Target;
	std::shared_ptr<T2DInterpolator<float> >  target_interp;
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
	TElastReg(const C2DFImage& _Ref, const C2DFImage& _Model,float __regrid_thresh,
		  int level, float __epsilon,float __lambda, float __mu);
	~TElastReg();

	float  solveAt(int x, int y);
	void	solvePDE(unsigned int nit);

	C2DFVector VdeformAt(const C2DFVector &p);
	bool	Vdeform(C2DFVectorfield *im = NULL);

	float  deformAt(const C2DFVector &p);
	bool	deformTemplate(C2DFImage *im = NULL);
	C2DFVector  forceAt(const C2DFVector &p, float s);
	void	calculateForces();
	void	calculateForcesHist();

	float  calculateStep();
	float  calculateMismatch(bool apply);
	float  calculateMismatchHist(bool apply);
	bool	decreaseStep();
	void	increaseStep();
	void work(C2DFVectorfield *Shift, const C2DInterpolatorFactory& ipfac);

};


inline bool TElastReg::decreaseStep()
{
	stepSize *= 0.5;
	if (stepSize >= min_stepsize) {
		return true;
	}else {
		stepSize = min_stepsize;
		return false;
	}
}

inline void TElastReg::increaseStep()
{
	stepSize *= 1.5;
	if (stepSize > MAX_STEP)
		stepSize = MAX_STEP;
}

#endif

/* CVS LOG

   $Log: elast.hh,v $
   Revision 1.2  2004/03/18 14:29:29  gerddie
   make -Wall silent

   Revision 1.1  2003/12/12 11:45:38  gerddie
   add elastic reg to 2d case

   Revision 1.5  2003/10/08 16:38:14  gerddie
   removed errors in fluid2d

   Revision 1.4  2002/07/25 12:40:57  gerddie
   some testing on regridding

   Revision 1.3  2002/06/20 09:59:47  gerddie
   added cvs-log entry


*/

NS_MIA_END
