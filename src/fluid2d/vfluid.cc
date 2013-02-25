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

#include <iomanip>
#include "vfluid.hh"
#include "NR2DMatrix.hh"

#include <mia/core/msgstream.hh>
#include <mia/2d/deformer.hh>
#include <memory>

NS_MIA_BEGIN

using namespace std;
bool verbose = true;

void cat (C2DFVectorfield& a, const C2DFVectorfield& b)
{
	assert(a.get_size() == b.get_size());

	C2DFVectorfield::iterator i = a.begin();
	for (size_t y = 0; y < a.get_size().y; ++y)  {
		for (size_t x = 0; x < a.get_size().x; ++x,++i)  {
			C2DFVector xi = C2DFVector(x, y) - *i;
			*i += b.get_interpol_val_at(xi);
		}
	}
}


TFluidReg::TFluidReg(const C2DFImage& _Ref, const C2DFImage& _Model, float __regrid_tresh,
		     int level, float __epsilon, float __lambda, float __mu):
	v(_Ref.get_size()),
	B(_Ref.get_size()),
	u(_Ref.get_size()),
	r(_Ref.get_size()),
	Ref(_Ref),
	Model(_Model),
	min_stepsize(MIN_STEP/level),
	final_level(level == 1),
	epsilon(__epsilon),
	lambda(__lambda),
	mu(__mu),
	regrid_thresh(__regrid_tresh)
{
	stepSize = 1.;

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
TFluidReg::~TFluidReg()
{
}

float  TFluidReg::solveAt(unsigned int x, unsigned int y)
{
	const C2DFVector& B_ = B(x,y);


	C2DFVector *vp = &v(x,y);

	C2DFVector *vpp = &v(x,y + 1);
	C2DFVector *vpm = &v(x,y - 1);

	C2DFVector p(B_.x + (a_b) * ( vp[-1].x + vp[+1].x ) + a_ * ( vpp->x + vpm->x ),
		     B_.y + (a_b) * ( vpm->y + vpp->y ) + a_ * ( vp[-1].y + vp[1].y ));

	C2DFVector q;
	q.y = ( ( vpm[-1].x + vpp[1].x ) - ( vpm[1].x + vpp[-1].x ) ) * b_4;
	q.x = ( ( vpm[-1].y + vpp[1].y ) - ( vpm[1].y + vpp[-1].y ) ) * b_4;

	C2DFVector hmm((( p + q ) - *vp) * omega);

	*vp += hmm;
	return hmm.norm();
}

void	TFluidReg::solvePDE(unsigned int nit)
{

	unsigned int i, x, y;
	double start_residuum = 0;
	double residuum;

	// init velocity fields
	v.clear();

	for (y = Start.y+1; y < End.y-1; y++)
		for (x = Start.x+1; x < End.x-1; x++){
			start_residuum += solveAt(x, y);
		}

	residuum = start_residuum;

	for (i = 0; i < nit && residuum/start_residuum > epsilon; i++)  {
		residuum = 0;

		for (y = Start.y+1; y < End.y-1; y++)
			for (x = Start.x+1; x < End.x-1; x++)
				residuum += solveAt(x, y);

		cvdebug() << "Residuum = " << residuum << "\n";

	}

}


C2DFVector  TFluidReg::forceAt(const C2DFVector &p, float s)
{
	float d = (*target_interp)(p) - s;
	C2DFVector gradt =  target_interp->derivative_at(p);
	return gradt * d;
}

void	TFluidReg::calculateForces()
{
	for (unsigned int y = Start.y; y < End.y; y++)  {
		for (unsigned int x = Start.x; x < End.x; x++)  {
			// compute force at (xi, yi):
			B(x,y) = forceAt(C2DFVector(x,y) - u(x,y), Ref(x,y)) * c;

		}
	}
}

float  TFluidReg::perturbationAt(unsigned int x, unsigned int y)
{
	if (x < Start.x+1 || x >= End.x-1 || y < Start.x+1 || y >= End.y-1) {
		r(x,y) = C2DFVector();
		return 0;
	} else {
		C2DFVector vi = v(x,y);
		C2DFVector ux = ( u(x+1,y) - u(x-1,y)) * vi.x;
		C2DFVector uy = ( u(x,y+1) - u(x,y-1)) * vi.y;
		C2DFVector ue = (ux + uy) * 0.5;
		C2DFVector ri = vi - ue;
		r(x,y) = ri;
		return ri.norm();
	}
}

float  TFluidReg::calculatePerturbation()
{
	float gamma = 0;
	for (unsigned int y = Start.y; y < End.y; y++)  {
		for (unsigned int x = Start.x; x < End.x; x++)  {
			float g = perturbationAt(x, y);
			if (g > gamma) gamma = g;
		}
	}

	if (gamma > 0){
		return stepSize / gamma;
	}else
		return 0;
}

float  TFluidReg::jacobianAt(unsigned int x, unsigned int y)
{
	C2DFVector ux = ( u(x+1,y) - u(x-1,y));
	C2DFVector uy = ( u(x,y+1) - u(x,y-1));

	C2DFVector rx = delta * (r(x+1,y)-r(x-1,y));
	C2DFVector ry = delta * (r(x,y+1)-r(x,y-1));

	float _a = 2 - ux.x - rx.x;
	float _b =   - uy.x - ry.x;
	float _c =   - ux.y - rx.y;
	float _d = 2 - uy.y - ry.y;

	return (_a * _d - _b * _c) * 0.25;

}

float  TFluidReg::calculateJacobian()
{
	float jmin = HUGE;
	for (unsigned int y = Start.y+1; y < End.y-1; y++)  {
		for (unsigned int x = Start.x+1; x < End.x-1; x++)  {
			float j = jacobianAt(x, y);
			if (j < jmin) jmin = j;
		};
	};
	return jmin;
}

float  TFluidReg::calculateMismatch(bool apply)
{
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
		C2DFImage::const_iterator t  = Template.begin();
		do {
			diff += fabs(*ra - *t);
			++t;
			++ra;
		}while (ra != re);

	}
	return diff ;

}

void TFluidReg::work(C2DFVectorfield *Shift, const C2DInterpolatorFactory& ipfac)
{
	int trys = 0;

	// init displacement field
	u.clear();
	Template = C2DFImage(Model.get_size());
	{
		FDeformer2D deformer(*Shift, ipfac);
		deformer(Model, Template);
	}


	double dmin = calculateMismatch(false);
	bool regrid = false;


	double diff = dmin;
	int maxiter = (final_level) ? 200 : 1000;

	for (int i = 0; i < maxiter && trys < 5 && dmin > 0; i++)  {

		cvmsg() <<  "[" << setw(3) << i << "]:" << dmin << ", " << stepSize << "\r";

		target_interp.reset(ipfac.create(Template.data()));

		// Algorithm 2, step 2+3:
		calculateForces();
		//calculateForces();


		// Algorithm 2, step 4:
		solvePDE(maxiter);

		// Algorithm 2, step 5+6:
	step5:
		delta = calculatePerturbation();

		// Algorithm 2, step 7:
		double j = calculateJacobian();
		if (j < regrid_thresh)  {
			if (regrid == true)  {
				// regrid failed again,
				// so try with a lower real time step size
				if (decreaseStep())
					goto step5;
				else {
					cvdebug() << "break because of regrid\n";
					break;
				}
			};

			// deform template
			cat(u, *Shift);
			*Shift = u;

			FDeformer2D deformer(*Shift, ipfac);
			deformer(Model, Template);

			u.clear();
			regrid = true;
			cvdebug() << "regrid\n";
		} else {
			// apply weighted perturbation to displacement field
			// see algorithm 2, step 7, second part

			if (delta > 0) {
				for (unsigned int y = Start.y; y < End.y; y++)
					for (unsigned int x = Start.x; x < End.x; x++)
						u(x,y) += delta * r(x,y);
			}

			// calculate difference between source and template
			diff = calculateMismatch(true);

			if (diff < dmin) {
				if (diff < dmin * 0.9)
					increaseStep();

				trys = 0;
				cvdebug() << "Decrease: " << dmin - diff << "\n";

				dmin = diff;
			}else{

				if (!decreaseStep()) {
					trys = 5;
					cvdebug() << "try: " << trys << "\n";
				}
				cvdebug() << "Decrease: " << dmin - diff << "\n";
			}
			regrid = false;
		}
	}
	cvmsg() <<  "\nfinal diff = " <<  dmin << "\n";
	cat(u, *Shift);
	*Shift = u;

}

NS_MIA_END
