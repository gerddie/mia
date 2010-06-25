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


#ifndef mia_2d_gridtransfromation_hh
#define mia_2d_gridtransfromation_hh


#include <iterator>
#include <mia/2d/transform.hh>


NS_MIA_BEGIN

/**
   2D transformation based on a deformation vector for each grid point.
 */

class EXPORT_2D C2DGridTransformation : public C2DTransformation {
public:

	typedef C2DFVectorfield::iterator field_iterator;
	typedef C2DFVectorfield::const_iterator const_field_iterator;

	C2DGridTransformation(const C2DBounds& size);

	//	operator C2DFVectorfield&();

	C2DFVector apply(const  C2DFVector& x) const;

	//	C2DFVectorfield& field() __attribute__((deprecated)) ;
	//	const C2DFVectorfield& field() const __attribute__((deprecated)) ;

	class EXPORT_2D const_iterator  {
	public:
		const_iterator& operator ++();
		const_iterator operator ++(int);

		const C2DFVector operator *() const;

		friend EXPORT_2D bool operator == (const const_iterator& a, const const_iterator& b);
		friend EXPORT_2D bool operator != (const const_iterator& a, const const_iterator& b);
		const_iterator();

	private:
		friend class C2DGridTransformation;
		const_iterator(const C2DBounds& pos, const C2DBounds& size, C2DFVectorfield::const_iterator start);

		C2DBounds _M_pos;
		C2DBounds _M_size;

		C2DFVectorfield::const_iterator _M_current;

	};

	const_iterator begin() const;
	const_iterator end() const;

	field_iterator field_begin();
	field_iterator field_end();

	const_field_iterator field_begin()const;
	const_field_iterator field_end()const;


	virtual const C2DBounds& get_size() const;

	virtual bool save(const std::string& filename, const std::string& type) const;
	virtual C2DTransformation *clone() const;
	virtual P2DTransformation upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual size_t degrees_of_freedom() const;
        virtual P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& ipf) const;
	virtual void translate(const C2DFVectorfield& gradient, gsl::DoubleVector& params) const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual void set_identity();
	virtual float get_max_transform() const;

	virtual gsl::DoubleVector get_parameters() const; 
	virtual void set_parameters(const gsl::DoubleVector& params); 

	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	C2DFVector operator ()(const  C2DFVector& x) const;
	virtual float divergence() const; 
	virtual float curl() const; 

	virtual float grad_divergence() const; 
	virtual float grad_curl() const; 
 private:
	C2DFVectorfield divergence_field() const;
	C2DFVectorfield curl_field() const;
	virtual C2DFMatrix field_derivative_at(int x, int y) const;
	C2DFVectorfield _M_field;
};

inline C2DFVector C2DGridTransformation::apply(const  C2DFVector& x) const
{
	return _M_field.get_interpol_val_at(x);
}

EXPORT_2D C2DGridTransformation operator + (const C2DGridTransformation& a, const C2DGridTransformation& b);

NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C2DGridTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C2DFVector       value_type;
		typedef mia::C2DBounds        difference_type;
		typedef mia::C2DFVector*      pointer;
		typedef mia::C2DFVector&      reference;
	};
}



#endif
