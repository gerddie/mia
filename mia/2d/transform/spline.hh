/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

	class EXPORT_2D const_iterator {
		friend class C2DSplineTransformation;
		const_iterator(const C2DSplineTransformation& trans, const C2DBounds& pos);
	public:
		const_iterator& operator ++();
		const_iterator operator ++(int);
		const C2DFVector& operator *() const;
		const C2DFVector *operator ->() const;
		const_iterator(const const_iterator&  other);

		bool operator == (const const_iterator& o) const;
		bool operator != (const const_iterator& o) const;
	private:
		const_iterator();
		void update_value() const;

		const C2DSplineTransformation& _M_trans;
		C2DBounds  _M_pos;
		mutable C2DFVector _M_value;
		mutable bool _M_value_valid;
	};

	const_iterator begin() const;
	const_iterator end() const;

	C2DSplineTransformation *refine(const C2DBounds& coeff_size) const;

	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual C2DTransformation *clone() const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual size_t degrees_of_freedom() const;
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFVectorfield translate(const C2DFVectorfield& gradient) const;
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual float get_max_transform() const;
	virtual void set_identity();
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float divergence() const; 
	virtual float curl() const; 
	float grad_divergence() const; 
	float grad_curl() const; 

	struct DCKernel {
		DCKernel(const vector<double>& R20_X, 
			 const vector<double>& R20_Y, 
			 const vector<double>& R11_X, 
			 const vector<double>& R11_Y, 
			 const vector<double>& R02_X, 
			 const vector<double>& R02_Y); 
		
		virtual double operator () (int xc, int yc, const C2DFVector& ci, const C2DFVector& cj) const = 0; 
	protected: 
		const vector<double>& _M_R20_X; 
		const vector<double>& _M_R20_Y; 
		const vector<double>& _M_R11_X; 
		const vector<double>& _M_R11_Y; 
		const vector<double>& _M_R02_X; 
		const vector<double>& _M_R02_Y; 
	}; 

private:	
	double get_grad_kernel_at(int x, int y, const DCKernel& kern)const; 
	C2DBounds _M_range;
	C2DFVectorfield _M_coefficients;
	P2DInterpolatorFactory _M_ipf;
	mutable C2DFVector _M_scale;
	mutable C2DFVector _M_inv_scale;
	mutable bool _M_interpolator_valid;
	mutable SHARED_PTR(T2DInterpolator<C2DFVector>)  _M_interpolator;
	mutable bool _M_matrices_valid;
	mutable vector<double> _M_R20_X; 
	mutable vector<double> _M_R20_Y; 
	mutable vector<double> _M_R02_X; 
	mutable vector<double> _M_R02_Y; 
	mutable vector<double> _M_R11_X; 
	mutable vector<double> _M_R11_Y; 
	void evaluate_matrices() const; 
		

};

inline
bool C2DSplineTransformation::const_iterator::operator ==
   (const C2DSplineTransformation::const_iterator& o) const
{
	return _M_pos == o._M_pos;
}

inline
bool C2DSplineTransformation::const_iterator::operator !=
   (const C2DSplineTransformation::const_iterator& o) const
{
	return !(*this == o);
}

NS_MIA_END

#endif
