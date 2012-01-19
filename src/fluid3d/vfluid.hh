/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef __vfluid_h
#define __vfluid_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <time.h>

#include <mia/core/watch.hh>
#include <mia/3d/interpolator.hh>
#include "eqn_solver.hh"

#define TEMP_NIT 20	// # of time steps to adapt template
#define MAX_STEP 0.8	// maximum step size
#define MIN_STEP 0.2	// minimum step size
extern int STARTSIZE;

#define PDE_NIT  20	// # of iterations to solve PDE
#define OMEGA	1.0	// overrelaxation factor

typedef struct {
	mia::P3DImage source;
	mia::P3DImage reference;
	float InitialStepsize;
	float Lambda;
	float My;
	int HalfFilterDim;
	float Overrelaxation;
	int  maxiter;
	float factor;
	bool useMutual;
	bool checkerboard;
	float matter_threshold;
} TFluidRegParams;

typedef struct {
	mia::C3DBounds Size;
	double PDETime;
	int    PDEEval;
	int    Regrids;
	int    niter;
	double allovertime;
}TMeasurement;
typedef std::list<TMeasurement> TMeasureList;
extern double g_start;


mia::P3DFVectorfield fluid_transform(const TFluidRegParams& params,TLinEqnSolver *solver,
				     bool use_multigrid, bool use_fullres,TMeasureList *measure_list,
				     const mia::C3DInterpolatorFactory& ipf
				);

extern mia::CWatch Clock;


class TFluidReg  {

protected:
	// Hmm, yeah, I know, bad style'n'stuff

        mia::C3DFImage src;	// source image
	mia::C3DFImage tmp;
        mia::C3DFImage ref;	// reference image
	float  a, b, c;
	float  lambda, my;     // elasticity constants
	mia::C3DBounds Start,End;
	mia::C3DBounds ROI;            // Size of Regin of Interest
	mia::C3DFVectorfield *u;        // Shiftfield
	float  delta;          // step size
	float mismatch;
	float matter_threshold;
private:
	mia::C3DFVectorfield *B;
	mia::C3DFVectorfield *V;
	TLinEqnSolver *solver;

	float  initialStepsize;       // step size
	const mia::C3DInterpolatorFactory& ipf;

	void  InitTemps();
	void  DeleteTemps();
	void  solvePDE();
	float perturbationAt(int x, int y, int z);
	float  calculateForces();
	float jacobianAt(int x, int y, int z)const;
	void  ApplyShift();
	mia::C3DFVector forceAt(int hardcode,float *misma)const;
	mia::C3DFVector forceAt(int x, int y ,int z,float *misma)const;
        float  calculatePerturbation();
        float  calculateJacobian()const;
	unsigned int GetLinCoord(unsigned int x,unsigned int y,unsigned int z) const;
public:
        TFluidReg(const TFluidRegParams& params, TLinEqnSolver *solver, const mia::C3DInterpolatorFactory& _ipf);
	~TFluidReg();

	float work(mia::P3DImage NewSource, mia::C3DFVectorfield& Shift);

	TMeasurement Measurement;
};

#endif

/* Changes to this file

  $Log$
  Revision 1.4  2005/06/29 13:43:35  wollny
  cg removed and libmona-0.7

  Revision 1.1.1.1  2005/06/17 10:31:09  gerddie
  initial import at sourceforge

  Revision 1.3  2005/02/22 09:49:16  wollny
  removed vistaio dependecy

  Revision 1.1.1.1  2005/02/21 15:00:37  wollny
  initial import

  Revision 1.13  2004/04/05 15:24:33  gerddie
  change filter allocation

  Revision 1.12  2004/03/24 16:17:35  gerddie
  landmarks seem to work now ...

  Revision 1.11  2004/03/21 20:45:17  gerddie
  deep landmark usage

  Revision 1.10  2004/03/18 14:28:44  gerddie
  Inserted hooks for landmark support

  Revision 1.9  2002/06/20 09:59:49  gerddie
  added cvs-log entry


*/
