/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_splinetransform_hh
#define mia_2d_splinetransform_hh

#include <mia/2d/interpolator.hh>
#include <mia/2d/transform.hh>
#include <mia/2d/ppmatrix.hh>
#include <mia/core/scaler1d.hh>

NS_MIA_BEGIN


class EXPORT_2D C2DSplineTransformation: public C2DTransformation {
public:
	using C2DTransformation::operator ();

	C2DSplineTransformation(const C2DSplineTransformation& org);
	C2DSplineTransformation(const C2DBounds& range, P2DInterpolatorFactory ipf);
	C2DSplineTransformation(const C2DBounds& range, P2DInterpolatorFactory ipf, const C2DFVector& c_rate);

	void set_coefficients(const C2DFVectorfield& field);
	void reinit()const;
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

		const C2DSplineTransformation& _M_trans;
		mutable C2DFVector _M_value;
		mutable bool _M_value_valid; 

	};


	C2DTransformation::const_iterator begin() const;
	C2DTransformation::const_iterator end() const;

	bool refine();

	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual C2DTransformation *invert() const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual size_t degrees_of_freedom() const;
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual void translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const;
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual float get_max_transform() const;
	virtual gsl::DoubleVector get_parameters() const;
	virtual void set_parameters(const gsl::DoubleVector& params);
	virtual void set_identity();
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;

	virtual double get_divcurl_cost(double wd, double wr, gsl::DoubleVector& gradient) const; 
	virtual double get_divcurl_cost(double wd, double wr) const; 

	C2DFVector on_grid(const mia::C2DBounds& x) const; 
private:
	void init_grid()const; 
	void run_downscaler(C1DScalarFixed& scaler, vector<double>& out_buffer)const; 
	virtual C2DTransformation *do_clone() const;
	C2DBounds _M_range;
	C2DFVector _M_target_c_rate;
	C2DFVectorfield _M_coefficients;
	P2DInterpolatorFactory _M_ipf;
	int _M_shift; 
	int _M_enlarge; 
	mutable C2DFVector _M_scale;
	mutable C2DFVector _M_inv_scale;
	mutable bool _M_interpolator_valid;
	mutable std::shared_ptr<T2DConvoluteInterpolator<C2DFVector> >  _M_interpolator;
	mutable std::shared_ptr<C2DPPDivcurlMatrix > _M_divcurl_matrix; 
	mutable std::vector<std::vector<double> > _M_x_weights; 
	mutable std::vector<std::vector<int> > _M_x_indices; 
	mutable std::vector<std::vector<double> > _M_y_weights; 
	mutable std::vector<std::vector<int> > _M_y_indices; 
	mutable bool _M_grid_valid; 

};

NS_MIA_END

#endif
