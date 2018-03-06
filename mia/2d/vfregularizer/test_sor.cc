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

#include <mia/internal/plugintester.hh>
#include <mia/2d/vfregularizer/sor.hh>
#include <mia/2d/vfregularizerkernel.hh>
#include <mia/core/parallel.hh>

using namespace std;
using namespace mia;


class Mock2DVectorfieldRegularizerKernel: public C2DFVectorfieldRegularizerKernel
{
public:
       Mock2DVectorfieldRegularizerKernel();
private:
       virtual float do_evaluate_row(unsigned y, CBuffers& buffers);
       virtual float do_evaluate_row_sparse(unsigned y, CBuffers& buffers);
       virtual unsigned do_get_boundary_padding() const;
       float solve_at(C2DFVector *v, const C2DFVector& b);

       void post_set_data_fields();
       int m_dx;
       vector<CMutex> m_row_mutex;



};

BOOST_AUTO_TEST_CASE(test_sor_solver)
{
       C2DBounds size(5, 6);
       C2DBounds rbe = size - 2 * C2DBounds::_1;
       C2DFVectorfield v(size);
       C2DFVectorfield b(size);
       auto useful_range = v.get_range(2 * C2DBounds::_1, rbe);
       auto irv = useful_range.begin();
       auto erv = useful_range.end();

       while ( irv != erv ) {
              C2DFVector x(irv.pos());
              float fy = cos((x.y - 3.0) / 6.0 * M_PI);
              float fx = x.x - 2.5f;
              *irv = C2DFVector( fy *  1.0 / (1 + fx * fx),
                                 fy * fy *  1.0 / (1 + fx * fx));
              cvdebug() << "input[" << irv.pos() << "]=" << *irv << "\n";
              ++irv;
       }

       C2DBounds rbe1 = size - C2DBounds::_1;
       auto b_range = b.get_range(C2DBounds::_1, rbe1);
       auto irb = b_range.begin();
       auto erb = b_range.end();

       while (irb != erb) {
              C2DBounds p = irb.pos();
              *irb = (6.0f * v(p) +
                      v(C2DBounds(p.x + 1, p.y)) +
                      v(C2DBounds(p.x - 1, p.y)) +
                      v(C2DBounds(p.x, p.y + 1)) +
                      v(C2DBounds(p.x, p.y - 1)))
                     * 1.0f / 10.0f;
              ++irb;
       }

       C2DSORVectorfieldRegularizer sor_solver(1e-7, 1e-9, 1000,
                                               make_shared<Mock2DVectorfieldRegularizerKernel>());
       C2DFVectorfield vs(size);
       C2DFVectorfield d(size);
       sor_solver.set_size(size);
       sor_solver.run(vs, b, d);
       auto vs_full_range = vs.get_range(C2DBounds::_0, vs.get_size());
       auto v_full_range = v.get_range(C2DBounds::_0, vs.get_size());

       for (auto ivs = vs_full_range.begin(), iv = v_full_range.begin(); ivs != vs_full_range.end(); ++ivs, ++iv) {
              cvdebug() << "[" << ivs.pos() << "]=" << *ivs << " expect " << *iv << "\n";

              if (fabs(iv->x) > 1e-5)
                     BOOST_CHECK_CLOSE(ivs->x, iv->x, 1e-4);
              else
                     BOOST_CHECK_SMALL(ivs->x, 1e-5f);

              if (fabs(iv->y) > 1e-5)
                     BOOST_CHECK_CLOSE(ivs->y, iv->y, 1e-4);
              else
                     BOOST_CHECK_SMALL(ivs->y, 1e-5f);
       }
}

Mock2DVectorfieldRegularizerKernel::Mock2DVectorfieldRegularizerKernel():
       C2DFVectorfieldRegularizerKernel(false),
       m_dx(0)
{
}


float Mock2DVectorfieldRegularizerKernel::do_evaluate_row(unsigned y,
              CBuffers& MIA_PARAM_UNUSED(buffers))
{
       float residuum = 0.0f;
       auto& v = get_output_field();
       auto& b = get_input_field();
       unsigned linear_index = 1 + y * m_dx;
       // There should be a more fine-grained method, but for the test it's okay
       CScopedLock lock_zm1(m_row_mutex[y - 1]);
       CScopedLock lock_z(m_row_mutex[y]);
       CScopedLock lock_zp1(m_row_mutex[y + 1]);

       for (int x = 1; x < m_dx - 1; ++x, ++linear_index) {
              residuum += solve_at(&v[linear_index], b[linear_index]);
       }

       return residuum;
}

float Mock2DVectorfieldRegularizerKernel::do_evaluate_row_sparse(unsigned MIA_PARAM_UNUSED(y),
              CBuffers& MIA_PARAM_UNUSED(buffers))
{
       return numeric_limits<float>::max();
}

float Mock2DVectorfieldRegularizerKernel::solve_at(C2DFVector *v, const C2DFVector& b)
{
       C2DFVector s = 6.0f * v[0];
       s += v[1];
       s += v[-1];
       s += v[m_dx];
       s += v[-m_dx];
       s /= 10.0f;
       C2DFVector r = 1.2 * ( b - s);
       v[0] += r;
       return r.norm();
}

unsigned Mock2DVectorfieldRegularizerKernel::do_get_boundary_padding() const
{
       return 1;
}

void Mock2DVectorfieldRegularizerKernel::post_set_data_fields()
{
       auto s = get_input_field().get_size();
       m_dx = s.x;
       m_row_mutex = vector<CMutex>(s.y);
}
