/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_2d_transformmock_hh
#define mia_2d_transformmock_hh

#include <mia/2d/transform.hh>

NS_MIA_BEGIN

/**
   This class mocks a transformation by implementing all the pure virtual 
   methods of the \a C2DTransformation interface. 
   It is used only for testing purpouses. 
 */

struct  EXPORT_2D C2DTransformMock: public C2DTransformation {
	C2DTransformMock();
	C2DTransformMock(const C2DBounds& size);
	virtual C2DTransformation *invert() const;
	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual void add(const C2DTransformation& a);
	virtual void update(float step, const C2DFVectorfield& a);
	virtual size_t degrees_of_freedom() const;
	virtual void set_identity();
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const;
	virtual float get_max_transform() const;
	virtual CDoubleVector  get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual const C2DBounds& get_size() const;
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual C2DFVector apply(const C2DFVector& x) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	virtual float divergence() const;
	virtual float curl() const;
	virtual double get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const; 
	double get_divcurl_cost(double wd, double wr) const; 

	virtual C2DTransformation::const_iterator begin() const; 
	virtual C2DTransformation::const_iterator end() const; 
	
protected: 
	class iterator_impl:  public C2DTransformation::iterator_impl {
		friend class C2DTransformMock; 
		iterator_impl(const C2DBounds& pos, const C2DBounds& size);  

		C2DTransformation::iterator_impl *clone()const; 
		virtual const C2DFVector& do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		C2DFVector m_value; 
	}; 

private:
	virtual P2DTransformation do_upscale(const C2DBounds& size) const;
	virtual C2DTransformation *do_clone() const;
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	C2DBounds m_size;

};

NS_MIA_END

#endif
