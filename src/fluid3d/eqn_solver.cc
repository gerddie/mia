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

  As an exception to this license, "NEC C&C Research Labs" may use
  this software under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation.
   

*/

#include "eqn_solver.hh"

TIterLinEqnSolver::TIterLinEqnSolver(int _max_steps, float _rel_res, float _abs_res):
	max_steps(_max_steps), 
	rel_res(_rel_res),
	abs_res(_abs_res) 
{
	
}


TFluidHomogenSolver::TFluidHomogenSolver(int _max_steps, float _rel_res, float _abs_res, 
					 float mu, float lambda):
	TIterLinEqnSolver(_max_steps,_rel_res,_abs_res)
{
	assert(mu != 0.0 && lambda != 0.0);
	a = mu;
	b = lambda + mu;
	c = 1/(6*a+2*b);
	a_b = a + b; 
	b_4 = b * 0.25; 
}
