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

#ifndef mia_3d_splinetransform_hh
#define mia_3d_splinetransform_hh

#include <mia/3d/interpolator.hh>
#include <mia/3d/transform.hh>
#include <mia/3d/ppmatrix.hh>
#include <mia/core/scaler1d.hh>

NS_MIA_BEGIN


class EXPORT_3D C3DSplineTransformationBig: public C3DTransformation {
public:
	using C3DTransformation::operator ();

	C3DSplineTransformationBig(const C3DSplineTransformationBig& org);
	C3DSplineTransformationBig(const C3DBounds& range, PBSplineKernel kernel);
	C3DSplineTransformationBig(const C3DBounds& range, PBSplineKernel kernel, const C3DFVector& c_rate);
	~C3DSplineTransformationBig(); 

	void set_coefficients(const C3DFVectorfield& field);
	void set_coefficients_and_prefilter(const C3DFVectorfield& field);
	void reinit()const;
	C3DFVector apply( const C3DFVector& x) const;
	C3DFVector scale( const C3DFVector& x) const;

	virtual const C3DBounds& get_size() const;
	const C3DBounds& get_coeff_size() const;


	class EXPORT_3D iterator_impl: public C3DTransformation::iterator_impl  {
	public:
		iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
			      C3DFVectorfield::const_iterator value_it); 
	private: 
		virtual C3DTransformation::iterator_impl * clone() const; 
		virtual const C3DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		virtual void do_z_increment(); 

		C3DFVectorfield::const_iterator _M_value_it; 
		

	};


	C3DTransformation::const_iterator begin() const;
	C3DTransformation::const_iterator end() const;

	bool refine();

	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual C3DTransformation *invert() const;
	virtual P3DTransformation do_upscale(const C3DBounds& size) const;
	virtual void add(const C3DTransformation& a);
	virtual size_t degrees_of_freedom() const;
        virtual P3DImage apply(const C3DImage& image, const C3DInterpolatorFactory& ipf) const;
	virtual void update(float step, const C3DFVectorfield& a);
	virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const;
	virtual C3DFMatrix derivative_at(int x, int y, int z) const;
	C3DFMatrix derivative_at(const C3DFVector& x) const; 
	virtual float get_max_transform() const;
	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual void set_identity();
	virtual float pertuberate(C3DFVectorfield& v) const;
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const;
	virtual C3DFVector operator () (const C3DFVector& x) const;

	virtual double get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const; 
	virtual double get_divcurl_cost(double wd, double wr) const; 

	C3DFVector on_grid(const mia::C3DBounds& x) const; 

	C3DBounds get_enlarge() const; 
private:
	
	C3DFVector sum(const C3DBounds& start, 
		       const vector<double>& xweights, 
		       const vector<double>& yweights, 
		       const vector<double>& zweights) const;  
	
	typedef std::vector<std::pair<int, std::vector<float> > > CSplineDerivativeRow; 
	CSplineDerivativeRow get_derivative_row(size_t nin, size_t nout, double scale) const; 

	C3DSplineTransformationBig& operator = (const C3DSplineTransformationBig& org); 

	void init_grid()const; 
	C3DFVector interpolate(const C3DFVector& x) const; 

	virtual C3DTransformation *do_clone() const;
	C3DBounds _M_range;
	C3DFVector _M_target_c_rate;
	C3DFVectorfield _M_coefficients;
	PBSplineKernel _M_kernel; 
	C3DBounds _M_shift; 
	C3DBounds _M_enlarge; 
	mutable C3DFVector _M_scale;
	mutable C3DFVector _M_inv_scale;
	mutable bool _M_scales_valid;

	mutable std::shared_ptr<C3DPPDivcurlMatrix > _M_divcurl_matrix; 
	mutable std::vector<std::vector<double> > _M_x_weights; 
	mutable std::vector<int> _M_x_indices; 
	mutable std::vector<std::vector<double> > _M_y_weights; 
	mutable std::vector<int> _M_y_indices; 
	mutable std::vector<std::vector<double> > _M_z_weights; 
	mutable std::vector<int> _M_z_indices; 
	mutable CSplineDerivativeRow  _M_mx; 
	mutable CSplineDerivativeRow  _M_my; 
	mutable CSplineDerivativeRow  _M_mz; 
	mutable bool _M_grid_valid; 
	mutable P3DFVectorfield _M_current_grid; 
};

NS_MIA_END

#endif
