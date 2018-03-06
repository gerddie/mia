/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

/*
  This plug-in implements the navier-stokes operator like registration model
  that accounts for linear elastic and fluid dynamic registration.
  Which model is used depends on the selected time step.

  To solve the PDE a full SOR is implemented.
*/

#include <limits>
#include <mia/3d/model.hh>
NS_MIA_USE
using namespace boost;
using namespace std;

NS_BEGIN(navier_regmodel)

class C3DNavierRegModel: public C3DRegModel
{
public:
       C3DNavierRegModel(float mu, float lambda, size_t maxiter, float omega, float epsilon);

       bool test_kernel();
private:
       virtual void do_solve(const C3DFVectorfield& b, C3DFVectorfield& x) const;
       virtual float do_force_scale() const;

       C3DFVector get_p(const C3DFVector& b, const C3DFVectorfield::iterator& v) const;
       C3DFVector get_q(const C3DFVectorfield::iterator& v)const;
       float  solve_at(const C3DFVector& b, C3DFVectorfield::iterator& x)const;
       float m_mu;
       float m_lambda;
       float m_a,  m_c, m_a_b, m_b_4;
       float m_omega;
       float m_epsilon;
       size_t m_max_iter;
       mutable int m_dx;
       mutable int m_dxy;

};

C3DNavierRegModel::C3DNavierRegModel(float mu, float lambda, size_t maxiter, float omega, float epsilon):
       m_mu(mu),
       m_lambda(lambda),
       m_omega(omega),
       m_epsilon(epsilon),
       m_max_iter(maxiter),
       m_dx(0),
       m_dxy(0)
{
       float a = mu;
       float b = lambda + mu;
       m_c = 1 / (6 * a + 2 * b);
       m_b_4 = 0.25 * b * m_c;
       m_a_b = ( a + b ) * m_c;
       m_a = a * m_c;
       cvdebug() << "initialise model with mu=" << mu << " lambda=" << lambda << " omega=" << omega << "\n";
}


float C3DNavierRegModel::do_force_scale() const
{
       cvdebug() << "Force scale = " << m_c << "\n";
       return m_c;
}

void C3DNavierRegModel::do_solve(const C3DFVectorfield& b, C3DFVectorfield& v) const
{
       // init velocity fields
       float start_residuum = 0.0;
       float residuum;
       size_t i = 0;
       assert(b.get_size() == v.get_size());
       m_dx = b.get_size().x;
       m_dxy = b.get_plane_size_xy();

       do {
              ++i;
              residuum = 0;

              for (size_t z = 1; z < b.get_size().z - 1; z++) {
                     C3DFVectorfield::const_iterator ib = b.begin() + m_dx + m_dxy * z;
                     C3DFVectorfield::iterator iv = v.begin() + m_dx + m_dxy * z;

                     for (size_t y = 1; y < b.get_size().y - 1; y++) {
                            ++ib;
                            ++iv;

                            for (size_t x = 1; x < b.get_size().x - 1; x++, ++ib, ++iv) {
                                   residuum += solve_at(*ib, iv);
                            }

                            ++ib;
                            ++iv;
                     }

                     if (i == 1)
                            start_residuum = residuum;
              }

              cvdebug() << "SOLVE: " << residuum << "\n";

              if (residuum < 1)
                     break;
       } while (i < m_max_iter && residuum / start_residuum > m_epsilon);
}

inline C3DFVector C3DNavierRegModel::get_p(const C3DFVector& b, const C3DFVectorfield::iterator& v)const
{
       return  b + m_a_b * ( v[-1] + v[+1] )  + m_a * (v[-m_dx] + v[+m_dx] + v[-m_dxy] + v[+m_dxy]);
}

inline C3DFVector C3DNavierRegModel::get_q(const C3DFVectorfield::iterator& v)const
{
       return m_b_4 * C3DFVector( v[ -1 - m_dx].y + v[ 1 + m_dx].y - v[ -1 + m_dx].y - v[ 1 - m_dx].y +
                                  v[ -1 - m_dxy].z + v[ 1 + m_dxy].z - v[ -1 + m_dxy].z - v[ 1 - m_dxy].z,
                                  v[ -1 - m_dx].x + v[ 1 + m_dx].x - v[ -1 + m_dx].x - v[ 1 - m_dx].x +
                                  v[ -m_dx - m_dxy].z + v[ m_dx + m_dxy].z - v[ -m_dx + m_dxy].z - v[ m_dx - m_dxy].z,
                                  v[ -1 - m_dxy].x + v[ 1 + m_dxy].x - v[ -1 + m_dxy].x - v[ 1 - m_dxy].x +
                                  v[ -m_dx - m_dxy].y + v[ m_dx + m_dxy].y - v[ -m_dx + m_dxy].y - v[ m_dx - m_dxy].y);
}

inline float  C3DNavierRegModel::solve_at(const C3DFVector& b, C3DFVectorfield::iterator& v)const
{
       C3DFVector p = get_p(b, v);
       C3DFVector q = get_q(v);
       C3DFVector hmm((( p + q ) - *v) * m_omega);
       *v += hmm;
       return hmm.norm();
}

class C3DNavierRegModelPlugin: public C3DRegModelPlugin
{
public:
       C3DNavierRegModelPlugin();
       C3DRegModel *do_create()const;

private:
       const string do_get_descr()const;

       float m_mu;
       float m_lambda;
       float m_omega;
       float m_epsilon;
       int m_maxiter;
};

C3DNavierRegModelPlugin::C3DNavierRegModelPlugin():
       C3DRegModelPlugin("navier"),
       m_mu(1.0),
       m_lambda(1.0),
       m_omega(1.0),
       m_epsilon(0.0001),
       m_maxiter(100)
{
       add_parameter("mu", make_nonnegative_param(m_mu, false, "isotropic compliance"));
       add_parameter("lambda", make_nonnegative_param(m_lambda, false, "isotropic compression"));
       add_parameter("omega", make_ci_param(m_omega, 0.1, 10, false, "relexation parameter"));
       add_parameter("epsilon", make_oci_param(m_epsilon, 0.0, 0.1, false, "stopping parameter"));
       add_parameter("iter", make_lc_param(m_maxiter, 1, false, "maximum number of iterations"));
}

C3DRegModel *C3DNavierRegModelPlugin::do_create()const
{
       return new C3DNavierRegModel(m_mu, m_lambda, m_maxiter, m_omega, m_epsilon);
}

const string C3DNavierRegModelPlugin::do_get_descr()const
{
       return "navier-stokes based registration model";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C3DNavierRegModelPlugin();
}

NS_END

