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
   Basic image orientations based on viewing direction 
 */

enum E3DImageOrientation {

	ior_undefined = 0,  /** undefined image orientation */
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
	ior_axial_flipped = 2,     /**< standard axial/transversal orientation  */  
	ior_coronal = 5,   /**< standard coronal orientation */
	ior_coronal_flipped = 6,   /**< standard coronal orientation */
	ior_saggital = 9,  /**< standard saggital orientation */
	ior_saggital_flipped =10,  /**< standard saggital orientation */


	ior_unknown = 13,   /**< stopper index */	
};


extern const TDictMap<E3DImageOrientation> g_image_orientation_map; 

/**
   \brief This class represents the oriantation and position of a 3D data set. 
   
   This class represents the oriantation and position of a 3D data set, by using 
   the following parameters: 
   - the location of the first data pixel (corresponds to DICOM tag (00020,00032)
   - the rotation about this point (corresponds to DICOM tag (00020,00037)
   - the axis ordering (axial, saggital, coronal) 
   - the flipping of the data 
   
*/

class EXPORT_3D C3DOrientationAndPosition {
public: 
	C3DOrientationAndPosition(); 
	
	explicit C3DOrientationAndPosition(E3DImageOrientation axis); 

	C3DOrientationAndPosition(E3DImageOrientation axis, 
				  const C3DFVector& origin, 
				  const C3DFVector& scale, 
				  const Quaternion& rot); 
	
	void get_transform_parameters(CDoubleVector& params) const; 
	
	void get_inverse_transform_parameters(CDoubleVector& params) const; 

	C3DOrientationAndPosition& operator +=(const C3DOrientationAndPosition& other); 
	
	bool operator == (const C3DOrientationAndPosition& other) const; 

	bool operator < (const C3DOrientationAndPosition& other) const; 
	

	void print(std::ostream& os) const; 

	E3DImageOrientation get_axis_orientation() const; 

        /// \returns the voxel scale 
        const mia::C3DFVector& get_scale() const; 

        /// \returns the origin in physical coordinates 
        const mia::C3DFVector& get_origin()const; 

        /// \returns the rotation of the object 
        const mia::Quaternion& get_rotation()const;

private:
	
	const C3DFMatrix& get_axis_switch_matrix() const; 
	
	E3DImageOrientation m_axisorder; 
	C3DFVector m_origin; 
	C3DFVector m_scale; 
	Quaternion m_rotation; 

	static const C3DFMatrix ms_order_XYZ; 
	static const C3DFMatrix ms_order_YXZ;
	static const C3DFMatrix ms_order_XZY;
	static const C3DFMatrix ms_order_ZYX;
	static const C3DFMatrix ms_order_ZXY;
	static const C3DFMatrix ms_order_YZX;

	static const C3DFMatrix ms_order_XYZ_F; 
	static const C3DFMatrix ms_order_YXZ_F;
	static const C3DFMatrix ms_order_XZY_F;
	static const C3DFMatrix ms_order_ZXY_F;
	static const C3DFMatrix ms_order_ZYX_F;
	static const C3DFMatrix ms_order_YZX_F;

}; 


/**
   \ingroup basic 
   The Patient Position (DICOM 7.3.1.1 specifies the position of the patient relative to 
   the imaging equipment space. This attribute is intended for annotation purposes only. 
   It does not provide an exact mathematical relationship of the patient to the imaging equipment.
 
   When facing the front of the imaging equipment, Head First is defined as the patient’s head 
   being positioned toward the front of the imaging equipment. Feet First is defined as the 
   patient’s feet being positioned toward the front of the imaging equipment. Prone is defined 
   as the patient’s face being positioned in a downward (gravity) direction. Supine is defined as 
   the patient’s face being in an upward direction. Decubitus Right is defined as the patient’s 
   right side being in a downward direction. Decubitus Left is defined as the patient’s left 
   side being in a downward direction.
*/
enum E3DPatientPositioning {
	ipp_undefined = 0, /**< undefined patient position */  
	ipp_hfp, /**< head first prone */  
	ipp_hfs, /**< head first supine */  
	ipp_hfdr, /**< head first Decubitus Right */
	ipp_hfdl, /**< head first Decubitus Left */
	ipp_ffp, /**< feet first prone */  
	ipp_ffs, /**< feet first supine */  
	ipp_ffdr, /**< feet first Decubitus Right */
	ipp_ffdl, /**< feet first Decubitus Left */
	ipp_lastindex
};
 

extern const TDictMap<E3DPatientPositioning> g_patient_position_map; 

/**
   \ingroup basic 
   @brief Stream operator to write orientation orient to stream \a os 
   \param os
   \param orient
   \returns os 
 */
EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageOrientation orient);

/**
   \ingroup basic 
   @brief Stream operator to read orientation orient from  stream \a is 
   \param is
   \param[out] orient
   \returns is 
 */
EXPORT_3D  std::istream& operator >> (std::istream& is, E3DImageOrientation& orient);


/**
   \ingroup basic 
   @brief Stream operator to write orientation+position to stream \a os 
   \param os
   \param orient
   \returns os 
 */
inline std::ostream& operator << (std::ostream& os, const C3DOrientationAndPosition& orient) 
{
	orient.print(os); 
	return os; 
}

/**
   \ingroup basic 
   @brief Stream operator to read orientation+position from  stream \a is 
   \param is
   \param[out] orient
   \returns is 
 */
EXPORT_3D  std::istream& operator >> (std::istream& is, C3DOrientationAndPosition& orient);


/**
   \ingroup basic 
   @brief Stream operator to write patient position orient to stream \a  os 
   \param os
   \param pp
   \returns os 
 */
EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DPatientPositioning pp);

/**
   \ingroup basic 
   @brief Stream operator to read patient position orient from stream \a is 
   \param is
   \param[out] pp
   \returns is 
 */
EXPORT_3D  std::istream& operator >> (std::istream& is, E3DPatientPositioning& pp);

/**
   @ingroup basic 
   @brief attribute for 3D image orientation 
*/
typedef TAttribute<E3DImageOrientation> C3DImageOrientation;

/**
   @ingroup basic 
   @brief translator for 3D image orientations to and from strings 
*/ 
typedef TTranslator<E3DImageOrientation> COrientationTranslator;

/**
   @ingroup basic 
   @brief attribute for the patient position 
*/
typedef TAttribute<E3DPatientPositioning> CPatientPositionAttribute;


typedef TAttribute<C3DOrientationAndPosition> C3DImageOrientationPositionAttribute;
typedef TTranslator<C3DOrientationAndPosition> COrientationPositionTranslator;
/**
   @ingroup basic 
   @brief translator for the patient position 
*/
typedef TTranslator<E3DPatientPositioning> CPatientPositionTranslator;

template <> 
struct attribute_type<E3DImageOrientation> : public EAttributeType {
        static const int value = 3000;
}; 

template <> 
struct attribute_type<E3DPatientPositioning> : public EAttributeType {
        static const int value = 3001;
}; 

template <> 
struct attribute_type<C3DOrientationAndPosition>: public EAttributeType {
        static const int value = 3002;
}; 



extern EXPORT_3D const char * IDPatientPosition; 

NS_MIA_END

#endif
