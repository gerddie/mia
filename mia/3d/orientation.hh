/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef __mia_3d_oriantation_hh
#define __mia_3d_oriantation_hh

#include <ostream>
#include <istream>

#include <mia/core/attributes.hh>
#include <mia/3d/quaternion.hh>
#include <mia/3d/matrix.hh>
#include <mia/core/vector.hh>

NS_MIA_BEGIN


/**
   \ingroup basic 
   Basic image orientations based on the head 
 */
enum E3DImageAxisOrientation {

	ior_default = 1,   /**< look from above or below at the head */  

	ior_xyz         = 1,       /**< transversal head first */  
	ior_xyz_flipped = 2,       /**< transversal feet first */  
	ior_yxz         = 3,       /**< transversal head first xy transposed  */
	ior_yxz_flipped = 4,       /**< transversal feet first xy transposed  */

	ior_xzy         = 5,       /**< coronal face first */
	ior_xzy_flipped = 6,       /**< coronal back first */
	ior_zxy         = 7,       /**< coronal face first xz transposed */
	ior_zxy_flipped = 8,       /**< coronal back first xz transposed */

	ior_yzx         = 9,        /**< saggital left first */
	ior_yzx_flipped =10,        /**< saggital right first */
	ior_zyx         =11,        /**< saggital left first zy transposed */
	ior_zyx_flipped =12,        /**< saggital right first zy transposed */


	ior_axial = 1,     /**< standard axial/transversal orientation  */  
	ior_coronal = 5,   /**< standard coronal orientation */
	ior_saggital = 9,  /**< standard saggital orientation */

	ior_unknown = 13,   /**< stopper index */	
		};

/**
   \brief This class represents the oriantation and position of a 3D data set. 
   
   This class represents the oriantation and position of a 3D data set, by using 
   the following parameters: 
   - the location of the first data pixel (corresponds to DICOM tag (00020,00032)
   - the rotation about this point (corresponds to DICOM tag (00020,00037)
   - the axis ordering (axial, saggital, coronal) 
   - the flipping of the data 
   
*/

class C3DOrientationAndPosition {
	
	C3DOrientationAndPosition(); 
	
	C3DOrientationAndPosition(E3DImageAxisOrientation axis, 
				  const C3DFVector& origin, 
				  const C3DFVector& scale, 
				  const Quaternion& rot); 
	
	void get_transform_parameters(CDoubleVector& params) const; 
	
	void get_inverse_transform_parameters(CDoubleVector& params) const; 

	C3DOrientationAndPosition& operator +=(const C3DOrientationAndPosition& other); 
	
	bool operator == (const C3DOrientationAndPosition& other) const; 
	
private:
	
	const C3DFMatrix& get_axis_switch_matrix() const; 
	
	E3DImageAxisOrientation m_axisorder; 
	C3DFVector m_origin; 
	C3DFVector m_scale; 
	Quaternion m_rotation; 

	static const C3DFMatrix ms_order_XYZ; 
	static const C3DFMatrix ms_order_YXZ;
	static const C3DFMatrix ms_order_XZY;
	static const C3DFMatrix ms_order_ZYX;
	static const C3DFMatrix ms_order_ZXY;
	static const C3DFMatrix ms_order_YZX;

}; 


/**
   \ingroup basic 
   @brief Stream operator to write orientation orient to stream os 
   \param orient
   \param os
   \returns os 
 */
EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageAxisOrientation orient);

/**
   \ingroup basic 
   @brief Stream operator to read orientation orient from  stream is 
   \param orient
   \param is
   \returns is 
 */
EXPORT_3D  std::istream& operator >> (std::istream& is, E3DImageAxisOrientation& orient);

/**
   @ingroup basic 
   @brief attribute for 3D image orientation 
*/
typedef TAttribute<E3DImageAxisOrientation> C3DImageOrientation;

/**
   @ingroup basic 
   @brief translator for 3D image orientations to and from strings 
*/ 
typedef TTranslator<E3DImageAxisOrientation> COrientationTranslator;

NS_MIA_END

#endif
