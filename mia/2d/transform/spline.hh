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

#ifndef mia_2d_splinetransform_hh
#define mia_2d_splinetransform_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/splinetransformpenalty.hh>
#include <mia/2d/ppmatrix.hh>


NS_MIA_BEGIN


class EXPORT_2D C2DSplineTransformation: public C2DTransformation {
public:
	using C2DTransformation::operator ();

	C2DSplineTransformation(const C2DSplineTransformation& org);
	C2DSplineTransformation(const C2DBounds& range, PSplineKernel kernel, const C2DInterpolatorFactory& ipf, 
				P2DSplineTransformPenalty penalty);
	C2DSplineTransformation(const C2DBounds& range, PSplineKernel kernel, 
				const C2DFVector& c_rate, const C2DInterpolatorFactory& ipf, 
				P2DSplineTransformPenalty penalty);

	void set_coefficients(const C2DFVectorfield& field);
	void set_coefficients_and_prefilter(const C2DFVectorfield& field);
	virtual void reinit();
	C2DFVector apply( const C2DFVector& x) const;
	C2DFVector scale( const C2DFVector& x) const;

	virtual const C2DBounds& get_size() const;
	const C2DBounds& get_coeff_size() const;


	class EXPORT_2D iterator_impl: public C2DTransformation::iterator_impl  {
	public:
		iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
			      const C2DSplineTransformation& trans); 
	private: 
		virtual C2DTransformation::iterator_impl * clone() const; 
		virtual const C2DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 

		const C2DSplineTransformation& m_trans;
		mutable C2DFVector m_value;
		mutable bool m_value_valid; 

	};


	C2DTransformation::const_iterator begin() const;
	C2DTransformation::const_iterator end() const;

	bool refine();

	virtual C2DTransformation *invert() const;
	virtual P2DTransformation do_upscale(const C2DBounds& size) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const;
	C2DFMatrix derivative_at(const C2DFVector& x) const;
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual float get_max_transform() const;
	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual void set_identity();
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual C2DBounds get_minimal_supported_image_size() const; 

	C2DFVector on_grid(const mia::C2DBounds& x) const; 


private:
	C2DFMatrix do_derivative_at(const C2DFVector& x) const;
	C2DFVector find_inverse(const C2DBounds& x) const; 
	
	typedef std::vector<std::pair<int, std::vector<float> > > CSplineDerivativeRow; 
	CSplineDerivativeRow get_derivative_row(size_t nin, size_t nout, double scale) const; 

	C2DSplineTransformation& operator = (const C2DSplineTransformation& org); 

	double do_get_energy_penalty_and_gradient(CDoubleVector& gradient) const;
	double do_get_energy_penalty() const;
	bool   do_has_energy_penalty() const;


	void init_grid(); 
	C2DFVector interpolate(const C2DFVector& x) const; 

	virtual C2DTransformation *do_clone() const;
	C2DBounds m_range;
	C2DFVector m_target_c_rate;
	C2DFVectorfield m_coefficients;
	PSplineKernel m_kernel; 
	int m_shift; 
	C2DBounds m_enlarge; 
	C2DFVector m_scale;
	C2DFVector m_inv_scale;
	bool m_interpolator_valid;
	//mutable std::shared_ptr<T2DConvoluteInterpolator<C2DFVector> >  m_interpolator;
	std::vector<std::vector<double> > m_x_weights; 
	std::vector<int> m_x_indices; 
	std::vector<std::vector<double> > m_y_weights; 
	std::vector<int> m_y_indices; 
	CSplineDerivativeRow  m_mx; 
	CSplineDerivativeRow  m_my; 

	PSplineBoundaryCondition m_xbc; 
	PSplineBoundaryCondition m_ybc; 

	P2DSplineTransformPenalty m_penalty; 

};



class C2DSplineTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	C2DSplineTransformCreatorPlugin();
	virtual C2DTransformCreator *do_create(const C2DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
private:
	PSplineKernel m_interpolator;
	float m_rate; 
	C2DFVector m_rate2d;
	P2DSplineTransformPenalty m_penalty; 
};

NS_MIA_END

#endif
