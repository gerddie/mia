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

#ifndef mia_3d_gridtransformation_hh
#define mia_3d_gridtransformation_hh


#include <iterator>
#include <mia/3d/transform.hh>
#include <mia/3d/transformfactory.hh>

NS_MIA_BEGIN

/**
   3D transformation based on a deformation vector for each grid point.
   Implements the C3DTransformation interface 
 */

class EXPORT_3D C3DGridTransformation : public C3DTransformation {
public:

	typedef C3DFVectorfield::iterator field_iterator;
	typedef C3DFVectorfield::const_iterator const_field_iterator;

	C3DGridTransformation(const C3DBounds& size, const C3DInterpolatorFactory& ipf);

	//	operator C3DFVectorfield&();

	C3DFVector apply(const  C3DFVector& x) const;

	//	C3DFVectorfield& field() __attribute__((deprecated)) ;
	//	const C3DFVectorfield& field() const __attribute__((deprecated)) ;

	class EXPORT_3D iterator_impl: public C3DTransformation::iterator_impl  {
	public:
		iterator_impl(const C3DBounds& pos, const C3DBounds& size, C3DFVectorfield::const_iterator start); 
		iterator_impl(const C3DBounds& pos, const C3DBounds& begin, const C3DBounds& end, 
			      const C3DBounds& size, C3DFVectorfield::const_iterator value_it); 

	private: 
		virtual C3DTransformation::iterator_impl * clone() const; 
		virtual const C3DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		virtual void do_z_increment(); 

		C3DFVectorfield::const_iterator m_current;
		C3DFVector m_value; 

	};

	const_iterator begin() const;
	const_iterator end() const;

	const_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) const; 
	const_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const; 

	field_iterator field_begin();
	field_iterator field_end();

	const_field_iterator field_begin()const;
	const_field_iterator field_end()const;


	virtual const C3DBounds& get_size() const;

	virtual C3DTransformation *invert() const;
	virtual P3DTransformation do_upscale(const C3DBounds& size) const;
	virtual size_t degrees_of_freedom() const;
	virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const;
	virtual void update(float step, const C3DFVectorfield& a);
	virtual C3DFMatrix derivative_at(const  C3DFVector& x) const;
	virtual C3DFMatrix derivative_at(int x, int y, int z) const;
	virtual void set_identity();
	virtual float get_max_transform() const;

	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);

	virtual float pertuberate(C3DFVectorfield& v) const;
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const;
	C3DFVector operator ()(const  C3DFVector& x) const;

private:
	virtual C3DTransformation *do_clone() const;
	virtual C3DFMatrix field_derivative_at(int x, int y, int z) const;
	C3DFVectorfield m_field;

	C3DInterpolatorFactory m_upscale_interpolator_factory; 

	friend 
	EXPORT_3D C3DGridTransformation operator + (const C3DGridTransformation& a, const C3DGridTransformation& b);
};

inline C3DFVector C3DGridTransformation::apply(const  C3DFVector& x) const
{
	return m_field.get_interpol_val_at(x);
}

EXPORT_3D C3DGridTransformation operator + (const C3DGridTransformation& a, const C3DGridTransformation& b);


class C3DGridTransformCreator: public C3DTransformCreator {
public: 
	C3DGridTransformCreator(const C3DInterpolatorFactory& ipf); 
private: 
	virtual P3DTransformation do_create(const C3DBounds& size, const C3DInterpolatorFactory& ipf) const;
};

/**
   Plugin class to create the creater.  
 */
class C3DGridTransformCreatorPlugin: public C3DTransformCreatorPlugin {
public:
	C3DGridTransformCreatorPlugin();
	virtual C3DTransformCreator *do_create(const C3DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
};

NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C3DGridTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C3DFVector       value_type;
		typedef mia::C3DBounds        difference_type;
		typedef mia::C3DFVector*      pointer;
		typedef mia::C3DFVector&      reference;
	};
}

#endif
