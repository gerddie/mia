/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
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

/*
  This plug-in implements the navier-stokes operator like registration model
  that accounts for linear elastic and fluid dynamic registration.
  Which model is used depends on the selected time step.

  To solve the PDE a full SOR is implemented.
*/
#ifndef mia_2d_model_navier_hh
#define mia_2d_model_navier_hh

#include <limits>

#include <mia/2d/model.hh>

NS_BEGIN(naviera_regmodel);

class C2DNavierRegModel: public mia::C2DRegModel {
public:
	C2DNavierRegModel(float mu, float lambda, size_t maxiter, float omega, float epsilon);

	float  solve_at(unsigned int x, unsigned int y,
			const mia::C2DFVector& b, mia::C2DFVectorfield::iterator v, int dx)const;

private:
	virtual void do_solve(const mia::C2DFVectorfield& b, mia::C2DFVectorfield& x) const;

	float _M_mu;
	float _M_lambda;
	float _M_a1,_M_a, _M_b, _M_c, _M_a_b, _M_b_4;
	float _M_omega;
	float _M_epsilon;
	size_t _M_max_iter;
};

class C2DNavierRegModelPlugin: public mia::C2DRegModelPlugin {
public:
	C2DNavierRegModelPlugin();
	C2DNavierRegModelPlugin::ProductPtr do_create()const;

private:
	bool do_test() const;
	const std::string do_get_descr()const;

	float _M_mu;
	float _M_lambda;
	float _M_omega;
	float _M_epsilon;
	int _M_maxiter;
};

NS_END

#endif

