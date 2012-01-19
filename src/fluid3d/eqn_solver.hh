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

#ifndef __eqn_solver_h
#define __eqn_solver_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/3d/3DImage.hh>

class TLinEqnSolver {
public:
	virtual int solve(const mia::C3DFVectorfield& right_side, mia::C3DFVectorfield *solution)=0;
	virtual ~TLinEqnSolver(){};
};

class TIterLinEqnSolver: public TLinEqnSolver {
protected:
	int max_steps;
	float rel_res;
	float abs_res;
public:
	TIterLinEqnSolver(int _max_steps, float _rel_res, float _abs_res);
};


class TFluidHomogenSolver : public TIterLinEqnSolver {
protected:
	float b_4,a_b,a,c,b;
public:
	TFluidHomogenSolver(int _max_steps, float _rel_res, float _abs_res, float mu, float lambda);
};


#endif
