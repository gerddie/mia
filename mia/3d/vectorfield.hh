/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#ifndef mia_3d_3dvectorfield_hh
#define mia_3d_3dvectorfield_hh

#include <mia/3d/valueattributetranslator.hh>
#include <mia/3d/datafield.hh>

NS_MIA_BEGIN

extern template class EXPORT_3D T3DDatafield<C3DFVector>;
extern template class EXPORT_3D T3DDatafield<C3DDVector>;

/**
   @ingroup basic 
   @brief a 3D vector field 
   
   This class provides a field of 3D vectors,  
*/

template <typename T>
class T3DVectorfield: public T3DDatafield<T>, public CAttributedData {
public:
	T3DVectorfield()  = default;
	T3DVectorfield(const T3DVectorfield<T>& org):
		T3DDatafield<T>(org),
		CAttributedData(org)
		{
		}
	
	
	T3DVectorfield(const C3DBounds& size):
		T3DDatafield<T>(size) {};

	T3DVectorfield(const CAttributedData& data, const C3DBounds& size) __attribute__((deprecated)):
		T3DDatafield<T>(size),
		CAttributedData(data)
	{
	}

	T3DVectorfield(const C3DBounds& size, const CAttributedData& data):
		T3DDatafield<T>(size),
		CAttributedData(data)
		{
		}
	
	C3DFVector get_voxel_size() const {
		const PAttribute attr = get_attribute("voxel");
		if (!attr) {
			cvinfo() << "T3DVectorfield<T>::get_voxel_size(): "
				"voxel size not defined, default to <1,1,1>\n";
			return C3DFVector(1,1,1);
		}
		
		const CVoxelAttribute * vs = dynamic_cast<const CVoxelAttribute *>(attr.get());
		if (!vs){
			cvinfo() << "T3DImage<T>::get_voxel_size(): voxel size wrong type, "
				"default to <1,1,1>\n";
			return C3DFVector(1,1,1);
		}
		return *vs;
	}
	
	void set_voxel_size(const C3DFVector& voxel){
		set_attribute("voxel", PAttribute(new CVoxelAttribute(voxel)));
	}
	
};

extern template class EXPORT_3D T3DVectorfield<C3DFVector>;
extern template class EXPORT_3D T3DVectorfield<C3DDVector>;
extern template class EXPORT_3D range3d_iterator<T3DDatafield<C3DFVector>::iterator>;
extern template class EXPORT_3D range3d_iterator_with_boundary_flag<T3DDatafield<C3DFVector>::iterator>;
extern template class EXPORT_3D range3d_iterator<T3DDatafield<C3DFVector>::const_iterator>;
extern template class EXPORT_3D range3d_iterator_with_boundary_flag<T3DDatafield<C3DFVector>::const_iterator>;


/**
   @ingroup basic 
   @brief a 3D field of floating point single accuracy 3D vectors 
*/
typedef T3DVectorfield<C3DFVector>  C3DFVectorfield;

/**
   @ingroup basic 
   @brief a 3D field of floating point single accuracy 3D vectors 
*/
typedef std::shared_ptr<C3DFVectorfield > P3DFVectorfield;

/**
   @ingroup basic 
   @brief a 3D field of floating point double accuracy 3D vectors 
*/
typedef T3DVectorfield<C3DDVector>  C3DDVectorfield;

/**
   @ingroup basic 
   Concat two vector fields assuming these define transformations 
   A(x) = x - a(x) and B(x) = x - b(x), hence c(x) = a(x-b(x)) + b(x) 
   \param[in,out] lhs left input vector field and output 
   \param[in] rhs right input vector field and output 
   \returns lhs after processing 
*/
EXPORT_3D C3DFVectorfield& operator += (C3DFVectorfield& lhs, const C3DFVectorfield& rhs);


class EXPORT_3D C3DLinearVectorfieldInterpolator {
public: 
	C3DLinearVectorfieldInterpolator(const C3DFVectorfield& field); 
	
	C3DFVector operator () (const C3DFVector& x) const; 
private: 
	const C3DFVectorfield& m_field; 
}; 

#ifdef __SSE__
class EXPORT_3D C3DSSELinearVectorfieldInterpolator {
public: 
	C3DSSELinearVectorfieldInterpolator(const C3DFVectorfield& field); 
	
	C3DFVector operator ()(const C3DFVector& x) const; 
private: 
	const C3DFVectorfield& m_field; 
}; 
#endif 


EXPORT_3D  void vectorfield_as_inverse_of(C3DFVectorfield& me, const C3DFVectorfield& other, float tol, int maxiter);

NS_MIA_END

#endif
