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


#ifndef mia_2d_affinetransform_hh
#define mia_2d_affinetransform_hh

#include <iterator>
#include <boost/lambda/lambda.hpp>
#include <mia/2d/transform.hh>


NS_MIA_BEGIN

class C2DAffineTransformation;
class EXPORT_2D C2DAffineTransformation : public C2DTransformation {
public:
	enum EOps  {
		op_rotate = 1,
		op_trans  = 2,
		op_rigid  = 3,
		op_shear  = 4,
		op_scale  = 8,
		op_all    = 0xF
	};

	C2DAffineTransformation(const C2DBounds& size);
	C2DAffineTransformation(const C2DBounds& size,
				std::vector<float> transform,
				C2DAffineTransformation::EOps ops);
	C2DAffineTransformation(const C2DBounds& size, EOps ops);

	void scale(float x, float y);
	void translate(float x, float y);

	void rotate(float angle);

	C2DFVector apply(const C2DFVector& x) const;


	class EXPORT_2D const_iterator  {
	public:
		const_iterator& operator ++();
		const_iterator operator ++(int);

		const C2DFVector operator *() const;

		friend EXPORT_2D bool operator == (const const_iterator& a, const const_iterator& b);
		friend EXPORT_2D bool operator != (const const_iterator& a, const const_iterator& b);

		const_iterator();

	private:
		friend class C2DAffineTransformation;
		const_iterator(const C2DBounds& pos, const C2DBounds& size, const C2DAffineTransformation *trans);

		C2DBounds _M_current;
		C2DBounds _M_size;

		const C2DAffineTransformation *_M_trans;
		C2DFVector _M_value;
		C2DFVector _M_dx;

	};

	const_iterator begin() const;
	const_iterator end() const;

	virtual const C2DBounds& get_size() const;
	virtual C2DTransformation *clone() const;
	virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual void translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual gsl::DoubleVector get_parameters() const; 
	virtual void set_parameters(const gsl::DoubleVector& params); 
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
private:
	
	std::vector<float> _M_t;
	EOps _M_ops;
	C2DBounds _M_size;
	gsl::DoubleVector _M_params; 
};

NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C2DAffineTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C2DFVector        value_type;
		typedef mia::C2DBounds        difference_type;
		typedef mia::C2DFVector*           pointer;
		typedef mia::C2DFVector&          reference;
	};
}
#endif
