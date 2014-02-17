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


#include <limits>

#include <mia/2d/model/navier.hh>

NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(navier_regmodel)


C2DNavierRegModel::C2DNavierRegModel(float mu, float lambda, size_t maxiter,
				     float omega, float epsilon):
	m_mu(mu),
	m_lambda(lambda),
	m_omega(omega),
	m_epsilon(epsilon),
	m_max_iter(maxiter)
{
	float a = mu;
	float b = lambda + mu;
	m_c = 1 / (4*a+2*b);
	m_b_4 = 0.25 * b * m_c;
	m_a_b = ( a + b ) * m_c;
	m_a = a * m_c;
}


float C2DNavierRegModel::do_force_scale() const
{
	return m_c;
}

void C2DNavierRegModel::do_solve(const C2DFVectorfield& b, C2DFVectorfield& v) const
{
	// init velocity fields
	float start_residuum = 0.0;
	float residuum;

	size_t i = 0;

	do {
		residuum = 0;

		C2DFVectorfield::const_iterator ib = b.begin() + b.get_size().x;

		for (size_t y = 1; y < b.get_size().y-1; y++) {
			++ib;
			for (size_t x = 1; x < b.get_size().x-1; x++, ++ib){
				residuum += solve_at(x, y, *ib, v);
			}
			++ib;
		}
		if (i ==0)
			start_residuum = residuum;

		if (residuum < 0.01)
			break;
		++i;

	} while (i < m_max_iter && residuum / start_residuum > m_epsilon);
}

float  C2DNavierRegModel::solve_at(unsigned int x, unsigned int y,
				   const C2DFVector& b, C2DFVectorfield& v)const
{
	const int sizex = v.get_size().x;
	C2DFVector *vp = &v(x,y);

	C2DFVector *vpp = &vp[  sizex];
	C2DFVector *vpm = &vp[ -sizex];

	C2DFVector p(b.x + m_a_b * ( vp[-1].x + vp[+1].x ) +
		     m_a * ( vpp->x   + vpm->x  ),
		     b.y + m_a_b * ( vpm->y   + vpp->y   ) +
		     m_a * ( vp[-1].y + vp[1].y ));

	C2DFVector q;
	q.y = ( ( vpm[-1].x + vpp[1].x ) - ( vpm[1].x + vpp[-1].x ) ) * m_b_4;
	q.x = ( ( vpm[-1].y + vpp[1].y ) - ( vpm[1].y + vpp[-1].y ) ) * m_b_4;

	C2DFVector hmm((( p + q ) - *vp) * m_omega);

	*vp += hmm;
	return hmm.norm();
}

bool C2DNavierRegModel::test_kernel()
{
	return true;
}

class C2DNavierRegModelPlugin: public C2DRegModelPlugin {
public:
	C2DNavierRegModelPlugin();
	C2DRegModel *do_create()const;

private:
	const string do_get_descr()const;

	float m_mu;
	float m_lambda;
	float m_omega;
	float m_epsilon;
	int m_maxiter;
};

C2DNavierRegModelPlugin::C2DNavierRegModelPlugin():
	C2DRegModelPlugin("navier"),
	m_mu(1.0),
	m_lambda(1.0),
	m_omega(1.0),
	m_epsilon(0.0001),
	m_maxiter(100)
{
	add_parameter("mu", new CFloatParameter(m_mu, 0.0, numeric_limits<float>::max(),
						false, "isotropic compliance"));
	add_parameter("lambda", new CFloatParameter(m_lambda, 0.0,
						    numeric_limits<float>::max(),
						    false, "isotropic compression"));
	add_parameter("omega", new CFloatParameter(m_omega, 0.1, 10,
						   false, "relexation parameter"));
	add_parameter("epsilon", new CFloatParameter(m_epsilon, 0.000001, 0.1,
						     false, "stopping parameter"));
	add_parameter("iter", new CIntParameter(m_maxiter, 10, 10000,
						false, "maximum number of iterations"));
}

C2DRegModel *C2DNavierRegModelPlugin::do_create()const
{
	return new C2DNavierRegModel(m_mu, m_lambda,
				     m_maxiter, m_omega, m_epsilon);
}

const string C2DNavierRegModelPlugin::do_get_descr()const
{
	return "navier-stokes based registration model";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNavierRegModelPlugin();
}

NS_END

