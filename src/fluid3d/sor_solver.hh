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

#ifndef __sor_solver_h
#define __sor_solver_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eqn_solver.hh"


class TSORSolver: public TFluidHomogenSolver {
protected:
	mia::C3DBounds size;
	int d_xy;
public:
	TSORSolver(int _max_steps, float _rel_res, float _abs_res,
		   float mu, float lambda);
	virtual int solve(const mia::C3DFVectorfield& right_side,mia::C3DFVectorfield *solution);
protected:
	float solve_at_very_old(const mia::C3DFVectorfield& B,mia::C3DFVectorfield *V,size_t  x, size_t  y, size_t  z);
	float solve_at_old(mia::C3DFVector *Data,const mia::C3DFVector& bv);
	float solve_at(mia::C3DFVector *Data,const mia::C3DFVector& b);
};



class TSORASolver: public TSORSolver {
public:
	TSORASolver(int _max_steps, float _rel_res, float _abs_res,
		    float mu, float lambda);

	virtual int solve(const mia::C3DFVectorfield& b,mia::C3DFVectorfield *x);
protected:
	class  TUpdateInfo : public mia::C3DUBDatafield {
	public:
		TUpdateInfo(const mia::C3DBounds& size): mia::C3DUBDatafield(size){
		};

		/*
		  This code will show up as race condition, because writes may occure in parallel 
		  However, since it is always the same value that it is written (i.e. 1), this 
		  is of no consequence. 
		  Since the update indicator fields are swapped in serial code, reads do not clash with
		  writes. Note however, that because there is no "official" syncronization, Intel 
		  Thread Inspector will complain. 
		  An option to silence this would be to use a field of atomics or one should add a memory 
		  barrier before the swap. 
		*/
		void set_update(int hardcode) {
			const unsigned char val = 1;
			unsigned char *ptr = &(*this)[hardcode];
			unsigned char *p2 = ptr - get_plane_size_xy();
			const int size_x = get_size().x;

			p2[-size_x] = val;
			p2[-1] = val;
			p2[ 0] = val;
			p2[ 1] = val;
			p2[size_x] = val;

			p2 = ptr - get_size().x;

			p2[-1] = val;
			p2[ 0] = val;
			p2[ 1] = val;

			ptr[-1] = val;
			ptr[+1] = val;

			p2 = ptr + get_size().x;
			p2[-1] = val;
			p2[ 0] = val;
			p2[ 1] = val;

			p2 = ptr + get_plane_size_xy();
			p2[-size_x] = val;
			p2[-1] = val;
			p2[0  ] = val;
			p2[1] = val;
			p2[size_x] = val;
		}
	};
};

#endif

