/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_2d_rigidtransform_hh
#define mia_2d_rigidtransform_hh

#include <iterator>
#include <mia/2d/transform.hh>
#include <mia/2d/transformfactory.hh>


NS_MIA_BEGIN



class EXPORT_2D C2DRotationTransformation : public C2DTransformation {
public:
	C2DRotationTransformation(const C2DBounds& size, const C2DFVector& m_reltative_rot_center, 
				  const C2DInterpolatorFactory& ipf);
	C2DRotationTransformation(const C2DBounds& size,  double rotation, 
				  const C2DFVector& m_reltative_rot_center,
				  const C2DInterpolatorFactory& ipf);

	void rotate(double angle);

	C2DFVector get_displacement_at(const C2DFVector& x) const;


	class EXPORT_2D iterator_impl: public C2DTransformation::iterator_impl  {
	public:
		iterator_impl(const C2DBounds& pos, const C2DBounds& size, 
			      const C2DRotationTransformation& trans); 
	private: 
		virtual C2DTransformation::iterator_impl * clone() const; 
		virtual const C2DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 

		const C2DRotationTransformation& m_trans;
		C2DFVector m_value;
		C2DFVector m_dx;

	};


	C2DTransformation::const_iterator begin() const;
	C2DTransformation::const_iterator end() const;

	virtual const C2DBounds& get_size() const;
	virtual C2DTransformation *invert() const;
	virtual P2DTransformation do_upscale(const C2DBounds& size) const;
	virtual void translate(const C2DFVectorfield& gradient, CDoubleVector& params) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const C2DFVectorfield& a);
	virtual C2DFMatrix derivative_at(const C2DFVector& x) const;
	virtual C2DFMatrix derivative_at(int x, int y) const;
	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual void set_identity();
	virtual float get_max_transform() const;
	virtual float pertuberate(C2DFVectorfield& v) const;
	virtual C2DFVector operator () (const C2DFVector& x) const;
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const;
	C2DFVector transform(const C2DFVector& x)const;
private:
	void initialize(); 
	virtual C2DTransformation *do_clone() const;
	void evaluate_matrix() const;
	C2DRotationTransformation(const C2DRotationTransformation& other);
	C2DRotationTransformation& operator =(const C2DRotationTransformation& other);
	C2DBounds m_size;
	double m_rotation;
	C2DFVector m_relative_rot_center; 
	C2DFVector m_rot_center; 
	mutable std::vector<double> m_t;
	mutable bool m_matrix_valid;
};


class C2DRotationTransformCreator: public C2DTransformCreator {
public: 
	C2DRotationTransformCreator(const C2DFVector& relative_rot_center, const C2DInterpolatorFactory& ipf); 
private: 
	virtual P2DTransformation do_create(const C2DBounds& size, const C2DInterpolatorFactory& ipf) const;
	C2DFVector m_relative_rot_center; 
};

class C2DRotationTransformCreatorPlugin: public C2DTransformCreatorPlugin {
public:
	C2DRotationTransformCreatorPlugin();
	virtual C2DTransformCreator *do_create(const C2DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
	C2DFVector m_relative_rot_center; 
};

NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C2DRotationTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C2DFVector        value_type;
		typedef mia::C2DBounds        difference_type;
		typedef mia::C2DFVector*           pointer;
		typedef mia::C2DFVector&          reference;
	};
}
#endif
