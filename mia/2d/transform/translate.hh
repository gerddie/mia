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


#ifndef mia_2d_affinetransform_hh
#define mia_2d_affinetransform_hh

#include <iterator>
#include <boost/lambda/lambda.hpp>
#include <mia/2d/transform.hh>


NS_MIA_BEGIN



class C2DTranslateTransformation;
class EXPORT_2D C2DTranslateTransformation : public C2DTransformation {
public:
	C2DTranslateTransformation(const C2DBounds& size);
	C2DTranslateTransformation(const C2DBounds& size,const C2DFVector& transform);

	void translate(float x, float y);

	C2DFVector apply(const C2DFVector& x) const;


	class EXPORT_2D iterator_impl: public C2DTransformation::iterator_impl  {
	public:
		iterator_impl(const C2DBounds& pos, const C2DBounds& size, const C2DFVector& m_value); 
	private: 
		virtual C2DTransformation::iterator_impl * clone() const; 
		virtual const C2DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		C2DFVector m_translate;
		C2DFVector m_value;
	};

	C2DTransformation::const_iterator begin() const;
	C2DTransformation::const_iterator end() const;

	virtual const C2DBounds& get_size() const;
	virtual C2DTransformation *invert() const;
	virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual P2DTransformation do_upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual void set_identity();
	virtual float get_max_transform() const;
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	C2DFVector transform(const C2DFVector& x)const;
	virtual float divergence() const;
	virtual float curl() const;
	float grad_divergence() const;
	float grad_curl() const;
	virtual double get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const; 
	virtual double get_divcurl_cost(double wd, double wr) const; 
private:
	virtual C2DTransformation *do_clone() const;
	C2DFVector m_transform;
	C2DBounds m_size;
};


NS_MIA_END

#endif
