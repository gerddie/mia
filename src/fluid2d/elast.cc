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

#include <iomanip>

#include "vfluid.hh"
#include "elast.hh"
#include <memory>
#include <mia/core/msgstream.hh>
#include <mia/2d/deformer.hh>


NS_MIA_BEGIN
using namespace std;

TElastReg::TElastReg(const C2DFImage& _Ref, const C2DFImage& _Model, float __regrid_tresh,
		     int level, float __epsilon, float __lambda, float __mu):
	ut(_Ref.get_size()),
	B(_Ref.get_size()),
	u(_Ref.get_size()),
	r(_Ref.get_size()),
	Ref(_Ref),
	Model(_Model),
	min_stepsize(MIN_FORCE_MULT/level),
	final_level(level == 1),
	epsilon(__epsilon),
	lambda(__lambda),
	mu(__mu),
	regrid_thresh(__regrid_tresh)
{
	stepSize = .00001;

	Start = C2DBounds(1,1);
	End = Ref.get_size() - Start;


	// note: grid is assumed to be unity in all directions
	a = mu;
	b = lambda + mu;
	c = 1 / (4*a+2*b);
	b_4 = 0.25*b * c;

	omega = 1.0;

	a_b = (a+b) * c;
	a_ = a * c;

	delta = 0;

}
TElastReg::~TElastReg()
{
}

float  TElastReg::solveAt(int x, int y)
{
	const C2DFVector& B_ = B(x,y);

	C2DFVector *vp = &u(x,y);

	C2DFVector *vpp = &vp[ u.get_size().x];
	C2DFVector *vpm = &vp[-u.get_size().x];

	C2DFVector p(B_.x + (a_b) * ( vp[-1].x + vp[+1].x ) + a_ * ( vpp->x + vpm->x ),
		     B_.y + (a_b) * ( vpm->y + vpp->y ) + a_ * ( vp[-1].y + vp[1].y ));

	C2DFVector q;
	q.y = ( ( vpm[-1].x + vpp[1].x ) - ( vpm[1].x + vpp[-1].x ) ) * b_4;
	q.x = ( ( vpm[-1].y + vpp[1].y ) - ( vpm[1].y + vpp[-1].y ) ) * b_4;

	C2DFVector hmm((( p + q ) - *vp) * omega);

	*vp += hmm;
	return hmm.norm();
}

void	TElastReg::solvePDE(unsigned int nit)
{

	unsigned int i, x, y;
	double start_residuum = 0;
	double residuum;


	// init velocity fields
	//v.clear();

	for (y = Start.y+1; y < End.y-1; y++)
		for (x = Start.x+1; x < End.x-1; x++){
			start_residuum += solveAt(x, y);
		}

	residuum = start_residuum;

	for (i = 0; i < nit && residuum > epsilon; i++)  {
		residuum = 0;


		for (y = Start.y+1; y < End.y-1; y++)
			for (x = Start.x+1; x < End.x-1; x++) {
				residuum += solveAt(x, y);
			}


	}

	cvmsg() << " res = " <<  residuum;
}


C2DFVector  TElastReg::forceAt(const C2DFVector &p, float s)
{
	float d = (*target_interp)(p) - s;
	C2DFVector gradt =  target_interp->derivative_at(p);
	return gradt * d;
}

void	TElastReg::calculateForces()
{
	float sum_force = 0.0;
	for (unsigned int y = Start.y; y < End.y; y++)  {
		for (unsigned int x = Start.x; x < End.x; x++)  {
			// compute force at (xi, yi):
			C2DFVector f  = stepSize * forceAt(C2DFVector(x,y) - u(x,y), Ref(x,y)) * c;
			sum_force += f.norm();
			B(x,y) = f;
		}
	}
	cvdebug() << "sum_force = " << sqrt(sum_force) << "\n";
}


float  TElastReg::calculateStep()
{
	float gamma = 0;
	for (unsigned int y = Start.y; y < End.y; y++)  {
		for (unsigned int x = Start.x; x < End.x; x++)  {
			float g = ut(x,y).norm();
			if (g > gamma) gamma = g;
		}
	}

	if (gamma > 0){
		return stepSize / gamma;
	}else
		return 0;
}


float  TElastReg::calculateMismatch(bool apply)
{
	//	int n = (End.y  - Start.y) * (End.x - Start.x);
	float diff = 0;
	if (apply){
		for (unsigned int y = Start.y; y < End.y; y++)  {
			for (unsigned int x = Start.x; x < End.x; x++)  {
				C2DFVector xi = C2DFVector(x,y) - u(x,y);
				float p = (*target_interp)(xi);
				diff += fabs(p - Ref(x,y));
			}
		}
	}else{
		C2DFImage::const_iterator ra = Ref.begin();
		C2DFImage::const_iterator re = Ref.end();
		C2DFImage::const_iterator t = Template.begin();
		do {
			diff += fabs(*ra - *t);
			++t;
			++ra;
		}while (ra != re);

	}
	return diff ;

}

void TElastReg::work(C2DFVectorfield *Shift, const C2DInterpolatorFactory& ipfac)
{
	int trys = 0;
	C2DFVectorfield usave(*Shift);

	// init displacement field
	u = *Shift;
	Shift->make_single_ref();
	Template = C2DFImage(Model.get_size());
	{
		FDeformer2D deformer(*Shift, ipfac);
		deformer(Model, Template);
	}
	target_interp.reset(ipfac.create(Template.data()));

	double dmin = calculateMismatch(false);

	double diff = dmin;
	int maxiter = (final_level) ? 500 : 20;
	for (int i = 0; i < maxiter && trys < 5 && dmin > 0; i++)  {

		cvmsg() << "[" << setw(3) << "]:" << setw(10) << dmin << ", " << setw(10) << stepSize  << "\n";

 		// Algorithm 2, step 2+3:
		calculateForces();
		//calculateForces();


		// Algorithm 2, step 4:
		solvePDE(1000);

		// Algorithm 2, step 5+6:

		// Algorithm 2, step 7:
		// apply weighted perturbation to displacement field
		// see algorithm 2, step 7, second part


		// calculate difference between source and template
		diff = calculateMismatch(true);



		if (diff < dmin) {

#if 0

			if (diff < dmin * 0.9)
				increaseStep();
#endif
			trys = 0;
			float delta = dmin - diff;
			cvdebug() << " delta cost " <<  delta << "\n";
			usave = u;
			u.make_single_ref();

			if (delta < 100)
				break;

			dmin = diff;
		}else{

#if 0
			if (!decreaseStep()) {
				trys = 5;
				fprintf(stderr,"try %d",trys);
			}
#endif
			trys++;
			cvdebug() << " delta cost " <<  delta << "\n";
		}

		if (dmin < 1000) break;

		cvmsg() << "\n";
	}
	cvmsg() << "final diff = " << dmin << "\n";
	*Shift = usave;
}


NS_MIA_END
