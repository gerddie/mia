/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_3d_rotationtransform_hh
#define mia_3d_rotationtransform_hh

#include <iterator>
#include <mia/3d/transform.hh>


NS_MIA_BEGIN



class C3DRotationTransformation;
class EXPORT_3D C3DRotationTransformation : public C3DTransformation {
public:

	C3DRotationTransformation(const C3DBounds& size, const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf);
	C3DRotationTransformation(const C3DBounds& size, const C3DFVector& rotation, 
				  const C3DFVector& relative_rot_center, const C3DInterpolatorFactory& ipf);

	C3DRotationTransformation(const C3DRotationTransformation& other);
	C3DRotationTransformation& operator =(const C3DRotationTransformation& other);

	void rotate(float xy, float xz, float yz);

	C3DFVector apply(const C3DFVector& x) const;


	class EXPORT_3D iterator_impl: public C3DTransformation::iterator_impl  {
	public:
		iterator_impl(const C3DBounds& pos, const C3DBounds& size, 
			      const C3DRotationTransformation& trans); 
		iterator_impl(const C3DBounds& pos, const C3DBounds& begin, 
			      const C3DBounds& end, const C3DBounds& size, 
			      const C3DRotationTransformation& trans); 
	private: 
		virtual C3DTransformation::iterator_impl * clone() const; 
		virtual const C3DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		virtual void do_z_increment(); 

		const C3DRotationTransformation& m_trans;
		C3DFVector m_value;
		C3DFVector m_dx;

	};

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator begin_range(const C3DBounds& begin, const C3DBounds& end) const; 
	const_iterator end_range(const C3DBounds& begin, const C3DBounds& end) const;
	
	virtual const C3DBounds& get_size() const;
	virtual C3DTransformation *invert() const;
	virtual P3DTransformation do_upscale(const C3DBounds& size) const;
	virtual void translate(const C3DFVectorfield& gradient, CDoubleVector& params) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const C3DFVectorfield& a);
	virtual C3DFMatrix derivative_at(const C3DFVector& x) const;
	virtual C3DFMatrix derivative_at(int x, int y, int z) const;
	virtual CDoubleVector get_parameters() const;
	virtual void set_parameters(const CDoubleVector& params);
	virtual void set_identity();
	virtual float get_max_transform() const;
	virtual float pertuberate(C3DFVectorfield& v) const;
	virtual C3DFVector operator () (const C3DFVector& x) const;
	virtual float get_jacobian(const C3DFVectorfield& v, float delta) const;
	C3DFVector transform(const C3DFVector& x)const;
private:
	virtual C3DTransformation *do_clone() const;
	void evaluate_matrix() const;
	void initialize(); 
		
	mutable std::vector<double> m_t;
	C3DBounds m_size;
	C3DFVector m_rotation;
	C3DFVector m_relative_rot_center;
	C3DFVector m_rot_center;
	mutable C3DFVector m_shift; 
	mutable bool m_matrix_valid;
	mutable CMutex m_mutex; 
};


NS_MIA_END

namespace std {
	template <>
	struct iterator_traits<mia::C3DRotationTransformation::const_iterator> {
		typedef input_iterator_tag iterator_category;
		typedef mia::C3DFVector     value_type;
		typedef mia::C3DBounds      difference_type;
		typedef mia::C3DFVector*    pointer;
		typedef mia::C3DFVector&    reference;
	};
}
#endif
