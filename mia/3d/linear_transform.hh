/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_3d_linear_transform_hh
#define mia_3d_linear_transform_hh


#include <mia/3d/transform.hh>

NS_MIA_BEGIN

class  EXPORT_3D C3DLinearTransform: public C3DTransformation {
public: 
	C3DLinearTransform();
	C3DLinearTransform(const C3DBounds& size, const C3DInterpolatorFactory& ipf);
	virtual C3DTransformation *invert() const;
	virtual size_t degrees_of_freedom() const;
	virtual void set_identity();
	virtual C3DFMatrix derivative_at(const C3DFVector& x) const;
	virtual C3DFMatrix derivative_at(int x, int y, int z) const;
	virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const;
	virtual float get_max_transform() const;
	virtual CDoubleVector  get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual const C3DBounds& get_size() const;
	virtual float pertuberate(C3DFVectorfield& v) const;
	virtual C3DFVector apply(const C3DFVector& x) const;
	virtual C3DFVector operator () (const C3DFVector& x) const;
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const;

	/// these functions should be removed 
	virtual void add(const C3DTransformation& a);
	virtual void update(float step, const C3DFVectorfield& a);
	virtual float divergence() const;
	virtual float curl() const;
	virtual double get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const; 
	double get_divcurl_cost(double wd, double wr) const; 

	virtual C3DTransformation::const_iterator begin() const; 
	virtual C3DTransformation::const_iterator end() const; 
	virtual C3DTransformation::const_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) const;  
	virtual C3DTransformation::const_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const; 
	
protected: 
	class iterator_impl:  public C3DTransformation::iterator_impl {
		friend class C3DLinearTransform; 
		iterator_impl(const C3DBounds& pos, const C3DBounds& size);  
		iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
			      const C3DBounds& begin, const C3DBounds& end);  

		C3DTransformation::iterator_impl *clone()const; 
		virtual const C3DFVector& do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		virtual void do_z_increment(); 
		C3DFVector m_value; 
	}; 

private:
	virtual P3DTransformation do_upscale(const C3DBounds& size) const;
	virtual C3DTransformation *do_clone() const;
	virtual P3DImage do_transform(const C3DImage& src, const C3DInterpolatorFactory&) const; 
	std::vector<double> m_matrix;
	C3DBounds m_size;

};

NS_MIA_END

#endif 
