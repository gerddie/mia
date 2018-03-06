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

#ifndef mia_3d_transform_spline_hh
#define mia_3d_transform_spline_hh

#include <mia/3d/interpolator.hh>
#include <mia/3d/transform.hh>
#include <mia/3d/ppmatrix.hh>
#include <mia/3d/splinetransformpenalty.hh>


NS_MIA_BEGIN


class EXPORT_3D C3DSplineTransformation: public C3DTransformation
{
public:
       using C3DTransformation::operator ();

       C3DSplineTransformation(const C3DSplineTransformation& org);
       C3DSplineTransformation(const C3DBounds& range, PSplineKernel kernel, const C3DInterpolatorFactory& ipf, P3DSplineTransformPenalty penalty);
       C3DSplineTransformation(const C3DBounds& range, PSplineKernel kernel, const C3DFVector& c_rate,
                               const C3DInterpolatorFactory& ipf, P3DSplineTransformPenalty penalty);
       ~C3DSplineTransformation();

       void set_coefficients(const C3DFVectorfield& field);
       void set_coefficients_and_prefilter(const C3DFVectorfield& field);
       void reinit()const;
       C3DFVector get_displacement_at( const C3DFVector& x) const;
       C3DFVector scale( const C3DFVector& x) const;

       virtual const C3DBounds& get_size() const;
       const C3DBounds& get_coeff_size() const;


       class EXPORT_3D iterator_impl: public C3DTransformation::iterator_impl
       {
       public:
              iterator_impl(const C3DBounds& pos, const C3DBounds& size,
                            C3DFVectorfield::const_iterator value_it);
              iterator_impl(const C3DBounds& pos, const C3DBounds& begin, const C3DBounds& end,
                            const C3DBounds& size, C3DFVectorfield::const_iterator value_it);

       private:
              virtual C3DTransformation::iterator_impl *clone() const;
              virtual const C3DFVector&  do_get_value()const;
              virtual void do_x_increment();
              virtual void do_y_increment();
              virtual void do_z_increment();

              C3DFVectorfield::const_iterator m_value_it;
              C3DBounds m_delta;

       };


       const_iterator begin() const;
       const_iterator end() const;
       const_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) const;
       const_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const;


       bool refine();

       virtual C3DTransformation *invert() const;
       virtual P3DTransformation do_upscale(const C3DBounds& size) const;
       virtual size_t degrees_of_freedom() const;
       virtual void update(float step, const C3DFVectorfield& a);
       virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const;
       virtual C3DFMatrix derivative_at(int x, int y, int z) const;
       virtual C3DFMatrix derivative_at(const C3DFVector& x) const;
       virtual float get_max_transform() const;
       virtual CDoubleVector get_parameters() const;
       virtual void set_parameters(const CDoubleVector& params);
       virtual void set_identity();
       virtual float pertuberate(C3DFVectorfield& v) const;
       virtual float get_jacobian(const C3DFVectorfield& v, float delta) const;
       virtual C3DFVector operator () (const C3DFVector& x) const;
       virtual C3DBounds get_minimal_supported_image_size() const;

       C3DFVector on_grid(const mia::C3DBounds& x) const;

       C3DBounds get_enlarge() const;
private:

       double do_get_energy_penalty_and_gradient(CDoubleVector& gradient) const;
       double do_get_energy_penalty() const;
       bool do_has_energy_penalty() const;

       C3DFVector sum(const C3DBounds& start,
                      const std::vector<double>& xweights,
                      const std::vector<double>& yweights,
                      const std::vector<double>& zweights) const;

       typedef std::vector<std::pair<int, std::vector<float>>> CSplineDerivativeRow;
       CSplineDerivativeRow get_derivative_row(size_t nin, size_t nout, double scale) const;

       C3DSplineTransformation& operator = (const C3DSplineTransformation& org);

       void init_grid()const;
       C3DFVector interpolate(const C3DFVector& x) const;

       virtual C3DTransformation *do_clone() const;
       C3DBounds m_range;
       C3DFVector m_target_c_rate;
       C3DFVectorfield m_coefficients;
       PSplineKernel m_kernel;
       C3DBounds m_shift;
       C3DBounds m_enlarge;
       mutable C3DFVector m_scale;
       mutable C3DFVector m_inv_scale;
       mutable bool m_scales_valid;

       mutable std::vector<std::vector<double>> m_x_weights;
       mutable std::vector<int> m_x_indices;
       mutable std::vector<std::vector<double>> m_y_weights;
       mutable std::vector<int> m_y_indices;
       mutable std::vector<std::vector<double>> m_z_weights;
       mutable std::vector<int> m_z_indices;
       mutable CSplineDerivativeRow  m_mx;
       mutable CSplineDerivativeRow  m_my;
       mutable CSplineDerivativeRow  m_mz;
       mutable bool m_grid_valid;
       mutable P3DFVectorfield m_current_grid;
       PSplineBoundaryCondition m_x_boundary;
       PSplineBoundaryCondition m_y_boundary;
       PSplineBoundaryCondition m_z_boundary;

       mutable CMutex m_mutex;

       P3DSplineTransformPenalty m_penalty;
};

NS_MIA_END

#endif
