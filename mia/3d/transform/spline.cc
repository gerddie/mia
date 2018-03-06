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

#include <cstdlib>
#include <cassert>
#include <limits>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <sstream>
#include <mia/3d/transform/spline.hh>
#include <mia/3d/transform/vectorfield.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/imageio.hh>
#include <mia/core/index.hh>

#include <gsl/gsl_cblas.h>

NS_MIA_BEGIN
using namespace std;

C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PSplineKernel kernel,
              const C3DInterpolatorFactory& ipf,
              P3DSplineTransformPenalty penalty):
       C3DTransformation(ipf),
       m_range(range),
       m_target_c_rate(1, 1, 1),
       m_kernel(kernel),
       m_scale(1.0, 1.0, 1.0),
       m_scales_valid(false),
       m_x_weights(m_range.x),
       m_x_indices(m_range.x),
       m_y_weights(m_range.y),
       m_y_indices(m_range.y),
       m_z_weights(m_range.z),
       m_z_indices(m_range.z),
       m_grid_valid(false),
       m_x_boundary(produce_spline_boundary_condition("mirror")),
       m_y_boundary(produce_spline_boundary_condition("mirror")),
       m_z_boundary(produce_spline_boundary_condition("mirror"))
{
       TRACE_FUNCTION;
       unsigned int s = m_kernel->get_active_halfrange() - 1;
       m_shift = C3DBounds(s, s, s);
       // add one for convenience: on the right side a zero padding is needed to
       // avoid memory access failures, because interpolatimng at the right boundary
       // us isually done with a zero weight on the last component, but the
       // code doesn't check if we want to access this
       // Quation is, what is more expensive
       m_enlarge = 2 * m_shift;
       assert(m_range.x > 0);
       assert(m_range.y > 0);
       assert(m_range.z > 0);

       if (penalty)
              m_penalty.reset(penalty->clone());
}

C3DSplineTransformation::C3DSplineTransformation(const C3DSplineTransformation& org):
       C3DTransformation(org),
       m_range(org.m_range),
       m_target_c_rate(org.m_target_c_rate),
       m_coefficients( org.m_coefficients),
       m_kernel(org.m_kernel),
       m_shift(org.m_shift),
       m_enlarge(org.m_enlarge),
       m_scales_valid(false),
       m_x_weights(m_range.x),
       m_x_indices(m_range.x),
       m_y_weights(m_range.y),
       m_y_indices(m_range.y),
       m_z_weights(m_range.z),
       m_z_indices(m_range.z),
       m_grid_valid(false),
       m_x_boundary(produce_spline_boundary_condition("mirror")),
       m_y_boundary(produce_spline_boundary_condition("mirror")),
       m_z_boundary(produce_spline_boundary_condition("mirror"))
{
       TRACE_FUNCTION;

       if (org.m_penalty)
              m_penalty.reset(org.m_penalty->clone());
}

C3DSplineTransformation::C3DSplineTransformation(const C3DBounds& range, PSplineKernel kernel,
              const C3DFVector& c_rate, const C3DInterpolatorFactory& ipf,
              P3DSplineTransformPenalty penalty):
       C3DTransformation(ipf),
       m_range(range),
       m_target_c_rate(c_rate),
       m_kernel(kernel),
       m_scales_valid(false),
       m_x_weights(m_range.x),
       m_x_indices(m_range.x),
       m_y_weights(m_range.y),
       m_y_indices(m_range.y),
       m_z_weights(m_range.z),
       m_z_indices(m_range.z),
       m_grid_valid(false),
       m_x_boundary(produce_spline_boundary_condition("mirror")),
       m_y_boundary(produce_spline_boundary_condition("mirror")),
       m_z_boundary(produce_spline_boundary_condition("mirror")),
       m_penalty(penalty)
{
       TRACE_FUNCTION;
       assert(m_range.x > 0);
       assert(m_range.y > 0);
       assert(m_range.z > 0);
       assert(c_rate.x >= 1.0);
       assert(c_rate.y >= 1.0);
       assert(c_rate.z >= 1.0);
       assert(kernel);
       unsigned int s = m_kernel->get_active_halfrange() - 1;
       m_shift = C3DBounds(s, s, s);
       m_enlarge = 2 * m_shift;
       C3DBounds csize( (C3DFVector(range - C3DBounds::_1 ) + c_rate ) / c_rate);

       if (csize.x <= 1 || csize.y <= 1 || csize.z <= 1)  {
              throw create_exception<invalid_argument>("C3DSplineTransformation: your coefficient rate [",
                            c_rate, "] is too large of the current image dimensions [",
                            range, "], reduce the crate parameter of the transformation "
                            "or the number of multi-resolution levels if you run a "
                            "registration algorithm.");
       }

       m_coefficients = C3DFVectorfield(csize + m_enlarge);
       m_x_boundary->set_width(m_coefficients.get_size().x);
       m_y_boundary->set_width(m_coefficients.get_size().y);
       m_z_boundary->set_width(m_coefficients.get_size().z);
}

C3DSplineTransformation::~C3DSplineTransformation()
{
}

C3DBounds C3DSplineTransformation::get_minimal_supported_image_size() const
{
       return C3DBounds(ceil(m_target_c_rate.x), ceil(m_target_c_rate.y), ceil(m_target_c_rate.z))  + C3DBounds::_1;
}

void C3DSplineTransformation::set_coefficients(const C3DFVectorfield& field)
{
       TRACE_FUNCTION;

       if ((field.get_size().x <= 1 + m_enlarge.x) ||
           (field.get_size().y <= 1 + m_enlarge.y) ||
           (field.get_size().z <= 1 + m_enlarge.z)) {
              throw create_exception<invalid_argument>("C3DSplineTransformation::set_coefficients: "
                            "your coefficient field of size [", field.get_size(), "] "
                            "is too small, dimensions must be larger than[",
                            m_enlarge + C3DBounds::_1, "]");
       }

       m_scales_valid = (m_coefficients.get_size() == field.get_size());
       m_coefficients = field;
       m_x_boundary->set_width(m_coefficients.get_size().x);
       m_y_boundary->set_width(m_coefficients.get_size().y);
       m_z_boundary->set_width(m_coefficients.get_size().z);
       init_grid();
}

void C3DSplineTransformation::set_coefficients_and_prefilter(const C3DFVectorfield& field)
{
       TRACE_FUNCTION;
       C3DFVectorfield help1(field.get_size());
       m_x_boundary->set_width(field.get_size().x);
       m_y_boundary->set_width(field.get_size().y);
       m_z_boundary->set_width(field.get_size().z);
       vector<C3DFVector> buffer(field.get_size().x);

       for (size_t z = 0; z < field.get_size().z; ++z)
              for (size_t y = 0; y < field.get_size().y; ++y) {
                     field.get_data_line_x(y, z, buffer);
                     m_x_boundary->filter_line(buffer, m_kernel->get_poles());
                     help1.put_data_line_x(y, z, buffer);
              }

       buffer.resize(field.get_size().y);

       for (size_t z = 0; z < field.get_size().z; ++z)
              for (size_t x = 0; x < field.get_size().x; ++x) {
                     help1.get_data_line_y(x, z, buffer);
                     m_y_boundary->filter_line(buffer, m_kernel->get_poles());
                     help1.put_data_line_y(x, z, buffer);
              }

       buffer.resize(field.get_size().z);

       for (size_t y = 0; y < field.get_size().y; ++y)
              for (size_t x = 0; x < field.get_size().x; ++x) {
                     help1.get_data_line_z(x, y, buffer);
                     m_z_boundary->filter_line(buffer, m_kernel->get_poles());
                     help1.put_data_line_z(x, y, buffer);
              }

       set_coefficients(help1);
}


C3DBounds C3DSplineTransformation::get_enlarge() const
{
       TRACE_FUNCTION;
       return m_enlarge;
}


void C3DSplineTransformation::reinit() const
{
       if (m_scales_valid)
              return;

       TRACE_FUNCTION;
       CScopedLock lock(m_mutex);

       if (!m_scales_valid) {
              if (m_penalty)
                     m_penalty->initialize(m_coefficients.get_size(), C3DFVector(m_range), m_kernel);

              TRACE("C3DSplineTransformation::reinit applies");
              m_scale = C3DFVector(m_coefficients.get_size() - C3DBounds::_1 - m_enlarge) /
                        C3DFVector(m_range - C3DBounds::_1);
              m_inv_scale = C3DFVector::_1 / m_scale;
              m_mx = get_derivative_row(m_range.x, m_coefficients.get_size().x, m_scale.x);
              m_my = get_derivative_row(m_range.y, m_coefficients.get_size().y, m_scale.y);
              m_mz = get_derivative_row(m_range.z, m_coefficients.get_size().z, m_scale.z);
              cvdebug() << "C3DSplineTransformation:"
                        << " m_coefficients.get_size() = " << m_coefficients.get_size()
                        << " m_range= " << m_range
                        << "  m_scale = " << m_scale << "\n";
              m_scales_valid = true;
              // pre-evaluateof fixed-grid coefficients
              size_t n_elms = m_kernel->size();

              for (size_t i = 0; i < m_range.x; ++i) {
                     m_x_weights[i].resize(n_elms);
                     m_x_indices[i] = m_kernel->get_start_idx_and_value_weights(i * m_scale.x + m_shift.x,
                                      m_x_weights[i]);
              }

              m_x_weights[m_range.x - 1].resize(n_elms - 1);

              for (size_t i = 0; i < m_range.y; ++i) {
                     m_y_weights[i].resize(n_elms);
                     m_y_indices[i] = m_kernel->get_start_idx_and_value_weights(i * m_scale.y + m_shift.y,
                                      m_y_weights[i]);
              }

              m_y_weights[m_range.y - 1].resize(n_elms - 1);

              for (size_t i = 0; i < m_range.z; ++i) {
                     m_z_weights[i].resize(n_elms);
                     m_z_indices[i] = m_kernel->get_start_idx_and_value_weights(i * m_scale.z + m_shift.z,
                                      m_z_weights[i]);
              }

              m_z_weights[m_range.z - 1].resize(n_elms - 1);
       }
}

C3DFVector C3DSplineTransformation::interpolate(const C3DFVector& x) const
{
       TRACE_FUNCTION;
       vector<double> xweights(m_kernel->size());
       vector<double> yweights(m_kernel->size());
       vector<double> zweights(m_kernel->size());
       C3DBounds start(m_kernel->get_start_idx_and_value_weights(x.x, xweights),
                       m_kernel->get_start_idx_and_value_weights(x.y, yweights),
                       m_kernel->get_start_idx_and_value_weights(x.z, zweights));
       return sum(start, xweights, yweights, zweights);
}

C3DFVector C3DSplineTransformation::get_displacement_at(const C3DFVector& x) const
{
       TRACE_FUNCTION;
       assert(m_scales_valid);
       return interpolate(scale(x));
}

C3DTransformation *C3DSplineTransformation::do_clone()const
{
       TRACE_FUNCTION;
       return new C3DSplineTransformation(*this);
}

C3DTransformation *C3DSplineTransformation::invert() const
{
       TRACE_FUNCTION;
       assert(0 && "not implemented");
       return new C3DSplineTransformation(*this);
}


C3DFVector C3DSplineTransformation::operator () (const C3DFVector& x) const
{
       TRACE_FUNCTION;
       return x - get_displacement_at(x);
}

C3DFVector C3DSplineTransformation::scale( const C3DFVector& x) const
{
       TRACE_FUNCTION;
       assert(m_scales_valid);
       return x * m_scale + C3DFVector(m_shift);
}

const C3DBounds& C3DSplineTransformation::get_size() const
{
       TRACE_FUNCTION;
       return m_range;
}

CDoubleVector C3DSplineTransformation::get_parameters() const
{
       TRACE_FUNCTION;
       CDoubleVector result(m_coefficients.size() * 3);
       auto r = result.begin();

       for (auto f = m_coefficients.begin(); f != m_coefficients.end(); ++f) {
              *r++ = f->x;
              *r++ = f->y;
              *r++ = f->z;
       }

       return result;
}

void C3DSplineTransformation::set_parameters(const CDoubleVector& params)
{
       TRACE_FUNCTION;
       m_grid_valid = false;
       assert(3 * m_coefficients.size() == params.size());
       auto r = params.begin();

       for (auto f = m_coefficients.begin(); f != m_coefficients.end(); ++f) {
              f->x = *r++;
              f->y = *r++;
              f->z = *r++;
       }

       init_grid();
}

bool C3DSplineTransformation::refine()
{
       TRACE_FUNCTION;
       C3DBounds csize = C3DBounds((C3DFVector(m_range) + m_target_c_rate - C3DFVector::_1) / m_target_c_rate)
                         + m_enlarge;
       cvdebug() << "Refine from " << m_coefficients.get_size() << " to " << csize << "\n";

       // no refinement necessary?
       if (csize.x <= m_coefficients.get_size().x &&
           csize.y <= m_coefficients.get_size().y &&
           csize.z <= m_coefficients.get_size().z)
              return false;

       // now interpolate the new coefficients
       // \todo this should be done faster by a filter
       reinit();
       T3DConvoluteInterpolator<C3DFVector> interp(m_coefficients, m_kernel);
       C3DFVectorfield coeffs(csize);
       C3DFVector dx(C3DFVector(m_coefficients.get_size() - C3DBounds::_1 - m_enlarge) /
                     C3DFVector(csize -  C3DBounds::_1 - m_enlarge));
       auto ic = coeffs.begin();
       cvdebug() << "refine scale = " << dx << "\n";
       C3DFVector fshift(m_shift);

       for (size_t z = 0; z < csize.z; ++z)
              for (size_t y = 0; y < csize.y; ++y)
                     for (size_t x = 0; x < csize.x; ++x, ++ic) {
                            C3DFVector X(x, y, z);
                            *ic = interp(dx * (X - fshift) + fshift);
                     }

       set_coefficients(coeffs);
       reinit();
       m_grid_valid = false;
       init_grid();
       return true;
}


P3DTransformation C3DSplineTransformation::do_upscale(const C3DBounds& size) const
{
       TRACE_FUNCTION;
       C3DFVector mx(C3DFVector(size) / C3DFVector(m_range));
       C3DSplineTransformation *help = new C3DSplineTransformation(size, m_kernel, get_interpolator_factory(), m_penalty);
       C3DFVectorfield new_coefs(m_coefficients.get_size());
       transform(m_coefficients.begin(), m_coefficients.end(), new_coefs.begin(),
       [&mx](const C3DFVector & x) {
              return mx * x;
       });
       help->set_coefficients(new_coefs);
       help->m_target_c_rate = m_target_c_rate;
       return P3DTransformation(help);
}

size_t C3DSplineTransformation::degrees_of_freedom() const
{
       TRACE_FUNCTION;
       return m_coefficients.size() * 3;
}

void C3DSplineTransformation::update(float step, const C3DFVectorfield& a)
{
       TRACE_FUNCTION;
       assert(a.get_size() == m_coefficients.get_size());
       auto ci = m_coefficients.begin();
       auto ce = m_coefficients.end();
       auto ai = a.begin();

       while ( ci != ce ) {
              *ci++ += C3DFVector(step * *ai++);
       }
}

C3DFVector C3DSplineTransformation::sum(const C3DBounds& start,
                                        const vector<double>& xweights,
                                        const vector<double>& yweights,
                                        const vector<double>& zweights) const
{
//	TRACE_FUNCTION;
       C3DFVector result;
       size_t z = start.z;

       for (auto wz = zweights.begin(); z < m_coefficients.get_size().z &&
            wz != zweights.end(); ++z, ++wz)  {
              C3DFVector hy;
              size_t y = start.y;

              for (auto wy = yweights.begin(); y < m_coefficients.get_size().y &&
                   wy != yweights.end(); ++y, ++wy)  {
                     C3DFVector hx;
                     size_t x = start.x;
                     auto wx = xweights.begin();

                     for (auto cx = m_coefficients.begin_at(start.x, y, z);
                          x < m_coefficients.get_size().x &&
                          wx != xweights.end(); ++x, ++wx, ++cx)  {
                            hx += *wx * *cx;
                     }

                     hy += hx * *wy;
              }

              result += hy * *wz;
       }

       return C3DFVector(result);
}


C3DFMatrix C3DSplineTransformation::derivative_at(const C3DFVector& v) const
{
//	TRACE_FUNCTION;
       if (v.x < 0 || v.y < 0 || v.z < 0 ||
           v.x > m_range.x - 1 ||  v.y > m_range.y - 1  || v.z > m_range.z - 1) {
              return C3DFMatrix::_1;
       }

       vector<double> xweights(m_kernel->size());
       vector<double> yweights(m_kernel->size());
       vector<double> zweights(m_kernel->size());
       vector<double> xdweights(m_kernel->size());
       vector<double> ydweights(m_kernel->size());
       vector<double> zdweights(m_kernel->size());
       const C3DFVector x = scale(v);
       C3DBounds start(m_kernel->get_start_idx_and_value_weights(x.x, xweights),
                       m_kernel->get_start_idx_and_value_weights(x.y, yweights),
                       m_kernel->get_start_idx_and_value_weights(x.z, zweights));
       m_kernel->get_start_idx_and_derivative_weights(x.x, xdweights);
       m_kernel->get_start_idx_and_derivative_weights(x.y, ydweights);
       m_kernel->get_start_idx_and_derivative_weights(x.z, zdweights);
       C3DFMatrix result(sum(start, xdweights, yweights,  zweights) * (-m_scale.x),
                         sum(start, xweights,  ydweights, zweights) * (-m_scale.y),
                         sum(start, xweights,  yweights, zdweights) * (-m_scale.z));
       result.x.x += 1.0;
       result.y.y += 1.0;
       result.z.z += 1.0;
       return result;
}

C3DFMatrix C3DSplineTransformation::derivative_at(int x, int y, int z) const
{
       TRACE_FUNCTION;
       return derivative_at(C3DFVector(x, y, z));
}

void C3DSplineTransformation::set_identity()
{
       TRACE_FUNCTION;
       fill(m_coefficients.begin(), m_coefficients.end(), C3DFVector());
       m_grid_valid = false;
}

float C3DSplineTransformation::get_max_transform() const
{
       TRACE_FUNCTION;
       // assuming the maximum spline coef is equal
       auto i = m_coefficients.begin();
       auto e = m_coefficients.end();
       assert(i != e);
       float value = i->norm2();
       ++i;

       while (i != e) {
              float v = i->norm2();

              if (value < v)
                     value = v;

              ++i;
       }

       return sqrt(value);
}


/*
  This versions of the INIT-GRID function evaluate a 3D vector field comprising the deformations
  at each grid point. Other then interpolating at each grid point on  request, this version
  implements a separable filtering of the slices using cblas for fast addition of large ranges.

*/
void C3DSplineTransformation::init_grid()const
{
       reinit();

       if (m_grid_valid)
              return;

       TRACE_FUNCTION;
       CScopedLock lock(m_mutex);

       if (!m_current_grid || (m_current_grid->get_size() != m_range)) {
              cvdebug() << "initialize grid field\n";
              m_current_grid.reset(new C3DFVectorfield(m_range));
       }

       const int size_xy = m_range.x * m_range.y * 3;
       const int size_xz = m_range.x * m_coefficients.get_size().z * 3;
       const int size_yz = m_coefficients.get_size().z * m_coefficients.get_size().y * 3;
       const int max_data_length = max( max(size_xy, size_xz), size_yz);
       // create the buffers
       unsigned int nelm = m_kernel->size();
       vector< vector<float>> in_buffer;

       for (unsigned int i = 0; i < nelm; ++i)
              in_buffer.push_back(vector<float>(max_data_length));

       vector<float> out_buffer(max_data_length);
       C3DFVectorfield tmp(C3DBounds(m_range.x,
                                     m_coefficients.get_size().y,
                                     m_coefficients.get_size().z));
       CCircularIndex idxx(nelm, m_y_indices[0]);

       for (size_t x = 0; x < m_range.x; ++x) {
              auto w = m_x_weights[x];
              int start = m_x_indices[x];
              idxx.new_start(start);
              // fill with slices
              auto fill = idxx.fill();

              while (fill < nelm && start + fill < m_coefficients.get_size().x) {
                     m_coefficients.read_xslice_flat(start + fill, in_buffer[idxx.next()]);
                     idxx.insert_one();
                     fill = idxx.fill();
              }

              memset(&out_buffer[0], 0, size_yz * sizeof(float));

              for (unsigned int i = 0; i < w.size(); ++i) {
                     cblas_saxpy(size_yz, w[i], &in_buffer[idxx.value(i)][0], 1,
                                 &out_buffer[0], 1);
              }

              tmp.write_xslice_flat(x, out_buffer);
       }

       C3DFVectorfield tmp2(C3DBounds(m_range.x,
                                      m_range.y,
                                      m_coefficients.get_size().z));
       CCircularIndex idxy(nelm, m_y_indices[0]);

       for (size_t y = 0; y < m_range.y; ++y) {
              auto w = m_y_weights[y];
              int start = m_y_indices[y];
              idxy.new_start(start);
              // fill with slices
              auto fill = idxy.fill();

              while (fill < nelm  && start + fill < m_coefficients.get_size().y) {
                     tmp.read_yslice_flat(start + fill, in_buffer[idxy.next()]);
                     idxy.insert_one();
                     fill = idxy.fill();
              }

              memset(&out_buffer[0], 0, size_xz * sizeof(float));

              for (unsigned int i = 0; i < w.size(); ++i) {
                     cblas_saxpy(size_xz, w[i], &in_buffer[idxy.value(i)][0], 1,
                                 &out_buffer[0], 1);
              }

              tmp2.write_yslice_flat(y, out_buffer);
       }

       // prepare the output field
       auto i = m_current_grid->begin();
       C3DFVector X;

       for (size_t z = 0; z < m_range.z; ++z) {
              X.z = z;

              for (size_t y = 0; y < m_range.y; ++y) {
                     X.y = y;

                     for (size_t x = 0; x < m_range.x; ++x, ++i) {
                            X.x = x;
                            *i = X;
                     }
              }
       }

       for (size_t iz = 0; iz < m_range.z; ++iz) {
              auto zweight = m_z_weights[iz];
              int i = m_z_indices[iz];

              // warning: this code assumes that the 3DVector is a POD-like structure, i.e. no VMT
              // and that x is the first stored element
              for (auto w  = zweight.begin(); w != zweight.end(); ++w, ++i) {
                     cblas_saxpy(size_xy, -(*w), &tmp2(0, 0, i).x, 1, &(*m_current_grid)(0, 0, iz).x, 1);
              }
       }

       m_grid_valid = true;
}

C3DTransformation::const_iterator C3DSplineTransformation::begin() const
{
       TRACE_FUNCTION;
       init_grid();
       assert(m_current_grid);
       return C3DTransformation::const_iterator(new iterator_impl(C3DBounds(0, 0, 0), get_size(),
                     m_current_grid->begin()));
}

C3DTransformation::const_iterator C3DSplineTransformation::end() const
{
       TRACE_FUNCTION;
       init_grid();
       assert(m_current_grid);
       return C3DTransformation::const_iterator(new iterator_impl(get_size(), get_size(),
                     m_current_grid->end()));
}

C3DTransformation::const_iterator C3DSplineTransformation::begin_range(const C3DBounds& begin, const C3DBounds& end) const
{
       TRACE_FUNCTION;
       init_grid();
       assert(m_current_grid);
       return C3DTransformation::const_iterator(new iterator_impl(begin, begin, end, get_size(),
                     m_current_grid->begin_at(begin.x, begin.y, begin.z)));
}

C3DTransformation::const_iterator C3DSplineTransformation::end_range(const C3DBounds& begin, const C3DBounds& end) const
{
       TRACE_FUNCTION;
       init_grid();
       assert(m_current_grid);
       return C3DTransformation::const_iterator(new iterator_impl(end, begin, end, get_size(),
                     m_current_grid->begin_at(end.x, end.y, end.z)));
}


/*
   This function evaluates the weights of the derivative  of the spline transformation
   w.r.t. the coefficients.
*/

C3DSplineTransformation::CSplineDerivativeRow
C3DSplineTransformation::get_derivative_row(size_t nin, size_t nout, double scale) const
{
       TRACE_FUNCTION;
       CSplineDerivativeRow result;

       for (size_t o = 0; o < nout; ++o) {
              CSplineDerivativeRow::value_type v;
              v.first = -1;
              double sum_y = 0.0;

              for (size_t i = 0; i < nin; ++i) {
                     double x = i * scale + m_shift.x - o;

                     if (std::fabs(x) <  m_kernel->get_nonzero_radius()) {
                            if (v.first < 0)
                                   v.first = i;

                            double y = m_kernel->get_weight_at(x, 0);
                            sum_y += y;

                            if (y > 0)
                                   v.second.push_back(y);
                            else
                                   break;
                     }
              }

              result.push_back(v);
       }

       return result;
}

void C3DSplineTransformation::translate(const C3DFVectorfield& gradient, CDoubleVector& params) const
{
       TRACE_FUNCTION;
       assert(params.size() == m_coefficients.size() * 3);
       assert(gradient.get_size() == m_range);
       reinit();
       C3DFVectorfield tmp(C3DBounds(gradient.get_size().x,
                                     gradient.get_size().y,
                                     m_coefficients.get_size().z));
       const int slice_size = 3 * gradient.get_size().y * gradient.get_size().x;

       for (size_t iz = 0; iz < m_coefficients.get_size().z; ++iz) {
              const CSplineDerivativeRow::value_type& myrow = m_mz[iz];
              int i = myrow.first;

              // warning: this code assumes that the 3DVector is a POD-like structure, i.e. no VMT
              // and that x is the first stored element
              for (auto w  = myrow.second.begin(); w != myrow.second.end(); ++w, ++i) {
                     cblas_saxpy(slice_size, *w, &gradient(0, 0, i).x, 1, &tmp(0, 0, iz).x, 1);
              }
       }

       C3DFVectorfield tmp2(C3DBounds(gradient.get_size().x,
                                      m_coefficients.get_size().y,
                                      m_coefficients.get_size().z));
       vector<C3DFVector> in_buffer(gradient.get_size().y);
       vector<C3DFVector> out_buffer(m_coefficients.get_size().y);

       for (size_t iz = 0; iz < m_coefficients.get_size().z; ++iz) {
              for (size_t ix = 0; ix < gradient.get_size().x; ++ix) {
                     tmp.get_data_line_y(ix, iz, in_buffer);

                     for (size_t i = 0; i < m_coefficients.get_size().y; ++i) {
                            const CSplineDerivativeRow::value_type& myrow = m_my[i];
                            out_buffer[i] = inner_product(myrow.second.begin(), myrow.second.end(),
                                                          in_buffer.begin() + myrow.first, C3DFVector());
                     }

                     tmp2.put_data_line_y(ix, iz, out_buffer);
              }
       }

       in_buffer.resize(gradient.get_size().x);
       // x convolution and copy to output
       auto r = params.begin();

       for (size_t iz = 0; iz < m_coefficients.get_size().z; ++iz) {
              for (size_t iy = 0; iy < m_coefficients.get_size().y; ++iy) {
                     tmp2.get_data_line_x(iy, iz, in_buffer);

                     for (size_t x = 0; x < m_coefficients.get_size().x; ++x, r += 3) {
                            const CSplineDerivativeRow::value_type& mxrow = m_mx[x];
                            const C3DFVector v = inner_product(mxrow.second.begin(), mxrow.second.end(),
                                                               in_buffer.begin() + mxrow.first, C3DFVector());
                            r[0] = -v.x;
                            r[1] = -v.y;
                            r[2] = -v.z;
                     }
              }
       }
}

float  C3DSplineTransformation::pertuberate(C3DFVectorfield& v) const
{
       TRACE_FUNCTION;
       FUNCTION_NOT_TESTED;
       reinit();
       C3DFVectorfield::iterator iv = v.begin();
       float max_gamma = 0.0f;
       C3DFVector lx_max;

       for (size_t z = 0; z < v.get_size().z; ++z)
              for (size_t y = 0; y < v.get_size().y; ++y)
                     for (size_t x = 0; x < v.get_size().x; ++x, ++iv) {
                            C3DFVector lx(x, y, z);
                            C3DFMatrix j = derivative_at(lx);
                            j.x.x = j.x.x * m_scale.x;
                            j.x.y = j.x.y * m_scale.x;
                            j.y.x = j.y.x * m_scale.y;
                            j.y.y = j.y.y * m_scale.y;
                            const C3DFVector u = j *  *iv;
                            *iv -= u;
                            float gamma = iv->norm2();

                            if (gamma > max_gamma) {
                                   lx_max = lx;
                                   max_gamma = gamma;
                            }
                     }

       return sqrt(max_gamma);
}

float C3DSplineTransformation::get_jacobian(const C3DFVectorfield& v, float delta) const
{
       FUNCTION_NOT_TESTED;
       assert(v.get_size() == get_coeff_size());
       const int dx = v.get_size().x;
       float j_min = numeric_limits<float>::max();
       delta *= 0.5;
       C3DFVector lx_min;

       for (size_t z = 1; z < v.get_size().z - 1; ++z) {
              for (size_t y = 1; y < v.get_size().y - 1; ++y) {
                     C3DFVectorfield::const_iterator iv = v.begin_at(1, y, z);

                     for (size_t x = 1; x < v.get_size().x - 1; ++x, ++iv) {
                            const C3DFVector lx(x, y, z);
                            C3DFMatrix J =  derivative_at(lx);
                            J.x.x = m_inv_scale.x - J.x.x;
                            J.x.y = - J.x.y;
                            J.y.x = - J.y.x;
                            J.y.y = m_inv_scale.y - J.y.y;
                            J.x -= delta * (iv[1]  - iv[-1]);
                            J.y -= delta * (iv[dx] - iv[-dx]);
                            const float j = J.x.x * J.y.y - J.x.y * J.y.x;

                            if ( j_min > j) {
                                   j_min = j;
                                   lx_min = lx;
                            }
                     }
              }
       }

       return j_min * m_scale.y * m_scale.x;
}

C3DFVector C3DSplineTransformation::on_grid(const C3DBounds& x) const
{
//	TRACE_FUNCTION;
       // this is a bit expensive, but uses less memory
       // one could evaluate the whole grid using convolution
       // but this would require more memory
       assert(x.x < m_range.x);
       assert(x.y < m_range.y);
       assert(x.z < m_range.z);
       assert(m_grid_valid);
       C3DBounds start(m_x_indices[x.x], m_y_indices[x.y], m_z_indices[x.z]);
       return sum(start, m_x_weights[x.x], m_y_weights[x.y], m_z_weights[x.z]);
}

const C3DBounds& C3DSplineTransformation::get_coeff_size() const
{
       TRACE_FUNCTION;
       return m_coefficients.get_size();
}


C3DSplineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& size,
              C3DFVectorfield::const_iterator value_it):
       C3DTransformation::iterator_impl(pos, size),
       m_value_it(value_it),
       m_delta(1, 1, 1)
{
}

C3DSplineTransformation::iterator_impl::iterator_impl(const C3DBounds& pos, const C3DBounds& begin, const C3DBounds& end,
              const C3DBounds& size, C3DFVectorfield::const_iterator value_it):
       C3DTransformation::iterator_impl(pos, begin, end, size),
       m_value_it(value_it)
{
       m_delta.y = size.x - (end.x - begin.x) + 1;
       m_delta.z = m_delta.y +  size.x * (size.y - (end.y - begin.y));
}


C3DTransformation::iterator_impl *C3DSplineTransformation::iterator_impl::clone() const
{
       TRACE_FUNCTION;
       return new iterator_impl(*this);
}

const C3DFVector&  C3DSplineTransformation::iterator_impl::do_get_value()const
{
       return *m_value_it;
}

void C3DSplineTransformation::iterator_impl::do_x_increment()
{
       ++m_value_it;
}
void C3DSplineTransformation::iterator_impl::do_y_increment()
{
       m_value_it += m_delta.y;
}
void C3DSplineTransformation::iterator_impl::do_z_increment()
{
       m_value_it += m_delta.z;
}

double C3DSplineTransformation::do_get_energy_penalty_and_gradient(CDoubleVector& gradient) const
{
       assert(m_penalty);
       return m_penalty->value_and_gradient(m_coefficients, gradient);
}

double C3DSplineTransformation::do_get_energy_penalty() const
{
       assert(m_penalty);
       return m_penalty->value(m_coefficients);
}

bool C3DSplineTransformation::do_has_energy_penalty() const
{
       return m_penalty.operator bool();
}


class C3DSplinebigTransformCreator: public C3DTransformCreator
{
public:
       C3DSplinebigTransformCreator(PSplineKernel ip, const C3DFVector& rates, const C3DInterpolatorFactory& ipf,
                                    P3DSplineTransformPenalty penalty,
                                    bool debug);
private:
       virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
       PSplineKernel m_kernel;
       C3DFVector m_rates;
       P3DSplineTransformPenalty m_penalty;
       bool m_debug;
};

C3DSplinebigTransformCreator::C3DSplinebigTransformCreator(PSplineKernel kernel, const C3DFVector& rates,
              const C3DInterpolatorFactory& ipf,
              P3DSplineTransformPenalty penalty,
              bool debug):
       C3DTransformCreator(ipf),
       m_kernel(kernel),
       m_rates(rates),
       m_penalty(penalty),
       m_debug(debug)
{
       TRACE_FUNCTION;
}


P3DTransformation C3DSplinebigTransformCreator::do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const
{
       TRACE_FUNCTION;
       assert(m_kernel);
       P3DTransformation result(new C3DSplineTransformation(size, m_kernel, m_rates, ipf, m_penalty));

       if (m_debug)
              result->set_debug();

       return result;
}


class C3DSplineTransformCreatorPlugin: public C3DTransformCreatorPlugin
{
public:
       C3DSplineTransformCreatorPlugin();
       virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
       const std::string do_get_descr() const;
private:
       PSplineKernel m_kernel;
       float m_rate;
       C3DFVector m_rate3d;
       P3DSplineTransformPenalty m_penalty;
       bool m_debug;
};

C3DSplineTransformCreatorPlugin::C3DSplineTransformCreatorPlugin():
       C3DTransformCreatorPlugin("spline"),
       m_rate(10),
       m_rate3d(0, 0, 0),
       m_debug(false)
{
       add_parameter("kernel", make_param(m_kernel, "bspline:d=3", false, "transformation spline kernel"));
       add_parameter("rate", make_lc_param(m_rate, 1, false, "isotropic coefficient rate in pixels"));
       add_parameter("anisorate",
                     new C3DFVectorParameter(m_rate3d, false, "anisotropic coefficient rate in pixels, nonpositive values "
                                             "will be overwritten by the 'rate' value."));
       add_parameter("debug",
                     new CBoolParameter(m_debug, false, "enable additional debugging output"));
       add_parameter("penalty", make_param(m_penalty, "", false, "transformation penalty energy term"));
}

C3DTransformCreator *C3DSplineTransformCreatorPlugin::do_create(const C3DInterpolatorFactory& ipf) const
{
       TRACE_FUNCTION;
       C3DFVector rate3d = m_rate3d;

       if (rate3d.x <= 0)
              rate3d.x = m_rate;

       if (rate3d.y <= 0)
              rate3d.y = m_rate;

       if (rate3d.z <= 0)
              rate3d.z = m_rate;

       return new C3DSplinebigTransformCreator(m_kernel, rate3d, ipf, m_penalty, m_debug);
}

const std::string C3DSplineTransformCreatorPlugin::do_get_descr() const
{
       return "Free-form transformation that can be described by a set of B-spline coefficients "
              "and an underlying B-spline kernel.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       auto p = new C3DGridTransformCreatorPlugin();
       p->append_interface(new C3DSplineTransformCreatorPlugin());
       return p;
}

NS_MIA_END

