/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_gridtransformation_hh
#define mia_2d_gridtransformation_hh


#include <iterator>
#include <mia/2d/transform.hh>


NS_MIA_BEGIN

/**
   2D transformation based on a deformation vector for each grid point.
   Implements the C2DTransformation interface 
 */

class EXPORT_2D C2DGridTransformation : public C2DTransformation {
public:

	typedef C2DFVectorfield::iterator field_iterator;
	typedef C2DFVectorfield::const_iterator const_field_iterator;

	C2DGridTransformation(const C2DBounds& size, const C2DInterpolatorFactory& ipf);

	//	operator C2DFVectorfield&();

	C2DFVector apply(const  C2DFVector& x) const;

	//	C2DFVectorfield& field() __attribute__((deprecated)) ;
	//	const C2DFVectorfield& field() const __attribute__((deprecated)) ;

	class EXPORT_2D iterator_impl: public C2DTransformation::iterator_impl  {
	public:
		iterator_impl(const C2DBounds& pos, const C2DBounds& size, C2DFVectorfield::const_iterator start); 
	private: 
		virtual C2DTransformation::iterator_impl * clone() const; 
		virtual const C2DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 

		C2DFVectorfield::const_iterator m_current;
		C2DFVector m_value; 

	};

	const_iterator begin() const;
	const_iterator end() const;

	field_iterator field_begin();
	field_iterator field_end();

	const_field_iterator field_begin()const;
	const_field_iterator field_end()const;


	virtual const C2DBounds& get_size() const;

	virtual C2DTransformation *invert() const;
	virtual P2DTransformation do_upscale(const C2DBounds& size) const;
	virtual void add(const C2DTransformation& a);
	virtual size_t degrees_of_freedom() const;
	virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFMatrix derivative_at(const C2DFVector& x) const;
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual void set_identity();
	virtual float get_max_transform() const;

	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);

	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	C2DFVector operator ()(const  C2DFVector& x) const;
	double get_divcurl_cost(double wd, double wr, CDoubleVector& gradient) const; 
	double get_divcurl_cost(double wd, double wr) const; 

	// these functions are here for testing 


	double dddgx_xxx(int x, int y) const; 
	double dddgy_yyy(int x, int y) const; 
	
	double dddgy_xxx(int x, int y) const; 
	double dddgx_yyy(int x, int y) const; 


	C2DFVector ddg_xx(int x, int y) const; 
	C2DFVector dddg_xxy(int x, int y) const; 

	C2DFVector ddg_yy(int x, int y) const; 
	C2DFVector dddg_yyx(int x, int y) const; 

	C2DFVector ddg_xy(int x, int y) const; 

	C2DFVector get_graddiv_at(int x, int y) const; 
	C2DFVector get_gradcurl_at(int x, int y) const; 
private:
	virtual C2DTransformation *do_clone() const;
	float grad_divergence(double weight, CDoubleVector& gradient) const; 
	double grad_curl(double weight, CDoubleVector& gradient) const; 
	virtual C2DFMatrix field_derivative_at(int x, int y) const;
	C2DFVectorfield m_field;

	C2DInterpolatorFactory m_upscale_interpolator_factory; 

	friend 
	EXPORT_2D C2DGridTransformation operator + (const C2DGridTransformation& a, const C2DGridTransformation& b);
};

inline C2DFVector C2DGridTransformation::apply(const  C2DFVector& x) const
{
	return m_field.get_interpol_val_at(x);
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
