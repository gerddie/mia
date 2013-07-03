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

#ifndef __mia_3d_oriantation_hh
#define __mia_3d_oriantation_hh

#include <ostream>
#include <istream>

#include <mia/core/attributes.hh>
#include <mia/3d/defines3d.hh>



NS_MIA_BEGIN

/**
   \ingroup basic 
   Basic image orientations based on viewing direction 
 */
enum E3DImageOrientation {ior_axial, /**< look from above or below at the head */  
			  ior_coronal, /**< look from behind or the front on the head */
			  ior_saggital, /**<look from left or right on the head */
			  ior_unknown /**< stopper index */
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


extern EXPORT_3D const char * IDPatientPosition; 

NS_MIA_END

#endif
