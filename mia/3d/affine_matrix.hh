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

#ifndef mia_3d_affine_matrix_hh
#define mia_3d_affine_matrix_hh

#include <mia/3d/quaternion.hh>

NS_MIA_BEGIN


/**
   This class implements an affine 3D transformation as is is used with OpenGL. 
   
   This class implements affine 3D transformations. The storage layout is compatible 
   with OpenGL. 
*/
class EXPORT_3D CAffinTransformMatrix {

public: 
	/// standard constructor, initializes the transformation with the identity. 
        CAffinTransformMatrix(); 

	/** A constructor to set all matrix values. Note, that in the numbering of the parameters 
	    corresponds to (column, row) and that the last column is always [0,0,0,1]. 
	 */
        CAffinTransformMatrix(float a11, float a12, float a13, float a14, 
			      float a21, float a22, float a23, float a24, 
			      float a31, float a32, float a33, float a34); 

	/**
	   multiply the current matrix by a rotation around the x axis centered at the given location and with the given angle. 
	   \param angle rotation angle in radians 
	   \param center rotation center 
	*/
        void rotate_x(float angle, const C3DFVector& center = C3DFVector::_0); 

	/**
	   multiply the current matrix by a rotation around the y axis centered at the given location and with the given angle. 
	   \param angle rotation angle in radians 
	   \param center rotation center 
	*/
        void rotate_y(float angle, const C3DFVector& center = C3DFVector::_0); 

	/**
	   multiply the current matrix by a rotation around the z axis centered at the given location and with the given angle. 
	   \param angle rotation angle in radians 
	   \param center rotation center 
	*/
        void rotate_z(float angle, const C3DFVector& center = C3DFVector::_0); 

	/**
	   multiply the current matrix by a rotation as defined by the given quaternion  centered at the given location
	   \param q quaternion describing rotation axis and angle
	   \param center rotation center 

	*/
        void rotate(const Quaternion& q, const C3DFVector& center = C3DFVector::_0);

	/**
	   multiply the current matrix by a 3x3 transformation matrix centered at the given location 
	   \param m 3x3 transformation matrix 
	   \param center rotation center 

	*/
        void transform_centered(const C3DFMatrix& m, const C3DFVector& center = C3DFVector::_0);

	/**
	   multiply the current matrix by a scaling matrix centered at the given location
	   \param scale saling factors along the three axis
	   \param center of the scaling 

	*/
        void scale(const C3DFVector& scale, const C3DFVector& center = C3DFVector::_0); 


	/**
	   multiply the current matrix by a shearing matrix centered at the given location
	   \param shear sharing factors along the three axis
	   \param center of the scaling 

	*/
        void shear(const C3DFVector& shear, const C3DFVector& center = C3DFVector::_0); 

	/**
	   multiply the current matrix by a translation matrix
	   \param shift the translation 
	*/
        void translate(const C3DFVector& shift); 

	/**
	   Reset the matrix to represent the identity matrix 
	 */
        void identity();

	/**
	   Multiply  the matrix with another matric from the right side. 
	   \param rhs right hand side matrix 
	   \returns a reference to the changed matrix 
	 */
        CAffinTransformMatrix& operator *= (const CAffinTransformMatrix& rhs); 
        
	/**
	   \returns the inverse of the matrix
	 */
        const CAffinTransformMatrix inverse() const; 
	
	/**
	   Apply the transformation to row vector \a x by multiplying it with the matrix 
	   \remark the resulting notation is mentally wrong 
	   \param x the vector to be transformed 
	   \returns the transformed vector 
	 */

        C3DFVector operator * (const C3DFVector& x) const;

	/**
	   Apply only the rotation and scaling part of the transform. This is useful 
	   to transform the rotation axis to the coordinate system the matrix is currently 
	   aligned to. 
	   \remark wrong frasing 
	   \param x the input vector 
	   \returns the rotated and scaled vector. 
	 */
        C3DFVector rotate(const C3DFVector& x) const;

	/**
	   \returns the raw matrix element in the apropriate order usable by OpenGL 
	*/
	const std::vector<float>& data() const; 
private: 
	std::vector<float> m_matrix;
	
}; 
	

/**
   Multiply two affine matrices
 */
EXPORT_3D CAffinTransformMatrix operator * (const CAffinTransformMatrix& lhs, const CAffinTransformMatrix& rhs); 




NS_MIA_END
#endif 
