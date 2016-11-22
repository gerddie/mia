/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_3d_axisrottransform_hh
#define mia_3d_axisrottransform_hh

#include <iterator>
#include <mia/3d/transform.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/affine_matrix.hh>


namespace mia_3dtransform_axisrot {



class EXPORT_3D C3DAxisrotTransformation : public mia::C3DTransformation {
public:
	C3DAxisrotTransformation(const mia::C3DBounds& size, const mia::C3DFVector& orig, 
				 const mia::C3DFVector& rot_axis, 
				 const mia::C3DInterpolatorFactory& ipf); 
	
	mia::C3DFVector get_displacement_at(const mia::C3DFVector& x) const;

	class EXPORT_3D iterator_impl: public mia::C3DTransformation::iterator_impl  {
	public:
		iterator_impl(const mia::C3DBounds& pos, const mia::C3DBounds& size, 
			      const C3DAxisrotTransformation& trans); 

		iterator_impl(const mia::C3DBounds& pos, const mia::C3DBounds& begin, 
			      const mia::C3DBounds& end, const mia::C3DBounds& size, 
			      const C3DAxisrotTransformation& trans); 
	private: 
		virtual mia::C3DTransformation::iterator_impl * clone() const; 
		virtual const mia::C3DFVector&  do_get_value()const; 
		virtual void do_x_increment(); 
		virtual void do_y_increment(); 
		virtual void do_z_increment(); 

		const C3DAxisrotTransformation& m_trans;
		mia::C3DFVector m_value;
		mia::C3DFVector m_dx;

	};

	const_iterator begin() const;
	const_iterator end() const;
	const_iterator begin_range(const mia::C3DBounds& begin, const mia::C3DBounds& end) const; 
	const_iterator end_range(const mia::C3DBounds& begin, const mia::C3DBounds& end) const; 


	virtual const mia::C3DBounds& get_size() const;
	virtual mia::C3DTransformation *invert() const;
	virtual mia::P3DTransformation do_upscale(const mia::C3DBounds& size) const;
	virtual void translate(const mia::C3DFVectorfield& gradient, mia::CDoubleVector& params) const;
	virtual size_t degrees_of_freedom() const;
	virtual void update(float step, const mia::C3DFVectorfield& a);
	virtual mia::C3DFMatrix derivative_at(const mia::C3DFVector& x) const; 
	virtual mia::C3DFMatrix derivative_at(int x, int y, int z) const;
	virtual mia::CDoubleVector get_parameters() const;
	virtual void set_parameters(const mia::CDoubleVector& params);
	virtual void set_identity();
	virtual float get_max_transform() const;
	virtual float pertuberate(mia::C3DFVectorfield& v) const;
	virtual mia::C3DFVector operator () (const mia::C3DFVector& x) const;
	virtual float get_jacobian(const mia::C3DFVectorfield& v, float delta) const;
	mia::C3DFVector transform(const mia::C3DFVector& x)const;

private:
	void apply_parameters(); 
	virtual mia::C3DTransformation *do_clone() const;
	C3DAxisrotTransformation(const C3DAxisrotTransformation& other) = default;
	C3DAxisrotTransformation& operator =(const C3DAxisrotTransformation& other)  = default;
	double m_angle;
	mia::CAffinTransformMatrix m_matrix;
        mia::C3DFVector m_relative_origin;
        mia::C3DFVector m_rotation_center;
	mia::C3DFVector m_rotation_axis; 
	mia::C3DBounds m_size;
	mia::Quaternion m_y_align_rot; 
	mia::Quaternion m_y_align_rot_inverse; 
	bool m_y_align_rot_needed; 
};


class C3DAxisrotTransformCreator: public mia::C3DTransformCreator {
public: 
	C3DAxisrotTransformCreator(const mia::C3DFVector& origin, 
				   const mia::C3DFVector& rot_axis,
				   const mia::C3DInterpolatorFactory& ipf); 
private: 
	virtual mia::P3DTransformation do_create(const mia::C3DBounds& size, 
						 const mia::C3DInterpolatorFactory& ipf) const;
	mia::C3DFVector m_origin; 
	mia::C3DFVector m_rotation_axis; 
};

class C3DAxisrotTransformCreatorPlugin: public mia::C3DTransformCreatorPlugin {
public:
	C3DAxisrotTransformCreatorPlugin(); 
	virtual mia::C3DTransformCreator *do_create(const mia::C3DInterpolatorFactory& ipf) const;
	const std::string do_get_descr() const;
private:
	mia::C3DFVector m_origin; 
	mia::C3DFVector m_rotation_axis; 
};



}
#endif
