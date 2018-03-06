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

#include <mia/3d/vfregularizerkernel/fluid-generic.hh>

NS_MIA_BEGIN

C3DFVfFluidStandardRegularizerKernel::C3DFVfFluidStandardRegularizerKernel(float mu, float lambda, float relax):
       C3DFVectorfieldRegularizerKernel(true),
       m_relax(relax),
       m_dx(0),
       m_dxy(0)

{
       float a = mu;
       float b = lambda + mu;
       m_c = 1 / (6.0f * a + 2.0f * b);
       m_b4 = 0.25f * b * m_c;
       m_a_b = ( a + b ) * m_c;
       m_a = a * m_c;
}

void C3DFVfFluidStandardRegularizerKernel::post_set_data_fields()
{
       m_dx = get_input_field().get_size().x;
       m_dxy = m_dx * get_input_field().get_size().y;
}

float C3DFVfFluidStandardRegularizerKernel::do_evaluate_row(unsigned y, unsigned z,
              CBuffers& MIA_PARAM_UNUSED(buf))
{
       float residuum = 0.0f;
       unsigned linear_index = 1 + y * m_dx + z * m_dxy;
       auto& v = get_output_field();
       auto& b = get_input_field();

       for (int x = 1; x < m_dx - 1; ++x, ++linear_index) {
              residuum += solve_at(&v[linear_index], b[linear_index]);
       }

       return residuum;
}

float C3DFVfFluidStandardRegularizerKernel::do_evaluate_row_sparse(unsigned y, unsigned z,
              CBuffers& MIA_PARAM_UNUSED(buf))
{
       float residuum = 0.0f;
       unsigned linear_index = 1 + y * m_dx + z * m_dxy;
       auto& v = get_output_field();
       auto& b = get_input_field();
       auto& update = get_update_flags();
       auto& res = get_residua();

       for (int x = 1; x < m_dx - 1; ++x, ++linear_index) {
              if (update[linear_index]) {
                     res[linear_index] = solve_at(&v[linear_index], b[linear_index]);
              }

              residuum += res[linear_index];

              if (res[linear_index] > get_residual_thresh()) {
                     set_update_flags(linear_index);
              }
       }

       return residuum;
}

void C3DFVfFluidStandardRegularizerKernel::multiply_with_matrix(C3DFVectorfield& out, const C3DFVectorfield& in)
{
       assert(out.get_size() == in.get_size());
       auto size = out.get_size();
       const C3DBounds rbe = size - C3DBounds::_1;
       auto in_useful_range = in.get_range(C3DBounds::_1, rbe);
       auto out_useful_range = out.get_range(C3DBounds::_1, rbe);
       auto iiv = in_useful_range.begin();
       auto eiv = in_useful_range.end();
       auto iov = out_useful_range.begin();

       while ( iiv != eiv ) {
              C3DFVector xx = iiv[-1] + iiv[1];
              C3DFVector yy = iiv[-m_dx] + iiv[m_dx];
              C3DFVector zz = iiv[-m_dxy] + iiv[m_dxy];
              const C3DFVector p(m_a_b * xx.x + m_a * (yy.x + zz.x),  // 6A 6M
                                 m_a_b * yy.y + m_a * (xx.y + zz.y),
                                 m_a_b * zz.z + m_a * (xx.z + yy.z));
              const C3DFVector& Vm1m1p0 = iiv[ -1 - m_dx];
              const C3DFVector& Vp1m1p0 = iiv[  1 - m_dx];
              const C3DFVector& Vm1p1p0 = iiv[ -1 + m_dx];
              const C3DFVector& Vp1p1p0 = iiv[  1 + m_dx];
              const float  vxdxy = Vm1m1p0.x - Vp1m1p0.x + Vp1p1p0.x - Vm1p1p0.x;
              const float  vydxy = Vm1m1p0.y - Vp1m1p0.y + Vp1p1p0.y - Vm1p1p0.y;
              const C3DFVector& Vm1p0m1 = iiv[ -1 - m_dxy];
              const C3DFVector& Vp1p0m1 = iiv[  1 - m_dxy];
              const C3DFVector& Vm1p0p1 = iiv[ -1 + m_dxy];
              const C3DFVector& Vp1p0p1 = iiv[  1 + m_dxy];
              const float  vxdxz = Vm1p0m1.x - Vp1p0m1.x + Vp1p0p1.x  - Vm1p0p1.x;
              const float  vzdxz = Vm1p0m1.z - Vp1p0m1.z + Vp1p0p1.z  - Vm1p0p1.z;
              const C3DFVector& Vp0m1m1 = iiv[ -m_dx - m_dxy];
              const C3DFVector& Vp0p1m1 = iiv[  m_dx - m_dxy];
              const C3DFVector& Vp0m1p1 = iiv[ -m_dx + m_dxy];
              const C3DFVector& Vp0p1p1 = iiv[  m_dx + m_dxy];
              const float  vydyz = Vp0m1m1.y - Vp0p1m1.y + Vp0p1p1.y  - Vp0m1p1.y;
              const float  vzdyz = Vp0m1m1.z - Vp0p1m1.z + Vp0p1p1.z  - Vp0m1p1.z;
              const C3DFVector q(vydxy + vzdxz, vxdxy + vzdyz, vxdxz + vydyz);
              *iov = *iiv + p + m_b4 * q;
              ++iiv;
              ++iov;
       }
}

float C3DFVfFluidStandardRegularizerKernel::do_evaluate_pertuberation_row(unsigned  y, unsigned  z,
              CBuffers& MIA_PARAM_UNUSED(buffers)) const
{
       auto iu = get_input_field().begin_at(1, y, z);
       auto iv = get_output_field().begin_at(1, y, z);
       float max_pert = 0.0f;

       for (int x = 1; x < m_dx - 1; ++x, ++iu, ++iv) {
              auto dux = iu[1] - iu[-1];
              auto duy = iu[m_dx] - iu[-m_dx];
              auto duz = iu[m_dxy] - iu[-m_dxy];
              *iv -= 0.5f * (iv->x * dux + iv->y * duy + iv->z * duz);
              float pert = iv->norm2();

              if (max_pert < pert)
                     max_pert = pert;
       }

       return max_pert;
}

unsigned C3DFVfFluidStandardRegularizerKernel::do_get_boundary_padding() const
{
       return 1;
}

float C3DFVfFluidStandardRegularizerKernel::solve_at(C3DFVector *v, const C3DFVector& b)
{
       C3DFVector *v_loc = &v[ -m_dxy ];
       const C3DFVector Vp0m1m1 = v_loc[ -m_dx ];
       const C3DFVector Vm1p0m1 = v_loc[ -1   ];
       const C3DFVector Vp0p0m1 = v_loc[  0   ];
       const C3DFVector Vp1p0m1 = v_loc[  1   ];
       const C3DFVector Vp0p1m1 = v_loc[  m_dx ];
       v_loc = &v[ -m_dx ];
       const C3DFVector Vm1m1p0 = v_loc[ -1 ];
       const C3DFVector Vp0m1p0 = v_loc[  0 ];
       const C3DFVector Vp1m1p0 = v_loc[  1 ];
       v_loc = &v[ m_dx ];
       const C3DFVector Vm1p1p0 = v_loc[ -1 ];
       const C3DFVector Vp0p1p0 = v_loc[  0 ];
       const C3DFVector Vp1p1p0 = v_loc[  1 ];
       const float  vxdxy = Vm1m1p0.x - Vp1m1p0.x + Vp1p1p0.x - Vm1p1p0.x;
       const float  vydxy = Vm1m1p0.y - Vp1m1p0.y + Vp1p1p0.y - Vm1p1p0.y;
       v_loc = &v[ m_dxy ];
       const C3DFVector Vp0m1p1 = v_loc[ -m_dx ];
       const C3DFVector Vm1p0p1 = v_loc[ -1   ];
       const C3DFVector Vp0p0p1 = v_loc[  0   ];
       const C3DFVector Vp1p0p1 = v_loc[  1   ];
       const C3DFVector Vp0p1p1 = v_loc[  m_dx ];
       const float  vxdxz = Vm1p0m1.x - Vp1p0m1.x + Vp1p0p1.x  - Vm1p0p1.x;
       const float  vzdxz = Vm1p0m1.z - Vp1p0m1.z + Vp1p0p1.z  - Vm1p0p1.z;
       const C3DFVector vdxx = v[-1] + v[1]; // 3A
       const C3DFVector vdyy = Vp0p1p0 + Vp0m1p0;
       const C3DFVector vdzz = Vp0p0p1 + Vp0p0m1;
       const float  vydyz = Vp0m1m1.y - Vp0p1m1.y + Vp0p1p1.y  - Vp0m1p1.y;
       const float  vzdyz = Vp0m1m1.z - Vp0p1m1.z + Vp0p1p1.z  - Vp0m1p1.z;
       const C3DFVector p(m_a_b * vdxx.x + m_a * (vdyy.x + vdzz.x),  // 6A 6M
                          m_a_b * vdyy.y + m_a * (vdxx.y + vdzz.y),
                          m_a_b * vdzz.z + m_a * (vdxx.z + vdyy.z));
       const C3DFVector q(vydxy + vzdxz, vxdxy + vzdyz, vxdxz + vydyz); // 3A
       const C3DFVector R = b + p + m_b4 * q;                // 6A 3M
       const C3DFVector delta = m_relax * ( R - *v );              // 3A 3M
       *v += delta;                                                // 3A
       return delta.norm();
}

void C3DFVfFluidStandardRegularizerKernel::set_update_flags(unsigned idx)
{
       auto& sf = get_set_flags();
       unsigned idx_mz = idx - m_dxy;
       sf[idx_mz - m_dx] = 1;
       sf[idx_mz - 1] = 1;
       sf[idx_mz] = 1;
       sf[idx_mz + 1] = 1;
       sf[idx_mz + m_dx] = 1;
       unsigned idx_my = idx - m_dx;
       sf[idx_my - 1] = 1;
       sf[idx_my] = 1;
       sf[idx_my + 1] = 1;
       sf[idx - 1] = 1;
       sf[idx] = 1;
       sf[idx + 1] = 1;
       unsigned idx_py = idx + m_dx;
       sf[idx_py - 1] = 1;
       sf[idx_py] = 1;
       sf[idx_py + 1] = 1;
       unsigned idx_pz = idx - m_dxy;
       sf[idx_pz - m_dx] = 1;
       sf[idx_pz - 1] = 1;
       sf[idx_pz] = 1;
       sf[idx_pz + 1] = 1;
       sf[idx_pz + m_dx] = 1;
}

C3DFVfFluidStandardRegularizerKernelPlugin::C3DFVfFluidStandardRegularizerKernelPlugin():
       C3DFVectorfieldRegularizerKernelPlugin("fluid"),
       m_mu(1.0f),
       m_lambda(1.0f),
       m_relax(1.0)
{
       this->add_parameter("mu", make_oci_param(m_mu, 0.0, 10000.0, false, "dynamic viscosity (shear)"));
       this->add_parameter("lambda", make_ci_param(m_lambda, 0.0, 10000.0, false, "bulk viscosity (compressibility)"));
       this->add_parameter("relax", make_ci_param(m_relax, 0.1, 2.0, false, "Relaxation parameter for the solver"));
}

C3DFVfFluidStandardRegularizerKernel *C3DFVfFluidStandardRegularizerKernelPlugin::do_create() const
{
       return new C3DFVfFluidStandardRegularizerKernel(m_mu, m_lambda, m_relax);
}

const std::string C3DFVfFluidStandardRegularizerKernelPlugin::do_get_descr() const
{
       return "Evaluation kernel for the fluid-dynamics solver, either using "
              "successive (over-)relaxation, or a Gauss-Southwell relaxation. "
              "This implementation is generic and doesn't implement any "
              "architecture specific optimizations.";
}

NS_MIA_END

extern "C" EXPORT mia::CPluginBase *get_plugin_interface()
{
       return new mia::C3DFVfFluidStandardRegularizerKernelPlugin();
}







