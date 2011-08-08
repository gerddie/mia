/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
#include <mia/3d/defines3d.hh>



NS_MIA_BEGIN

/**
   Basic image orientations based on the head 
 */
enum E3DImageOrientation {ior_axial, /**< look from above or below at the head */  
			  ior_coronal, /**< look from behind or the font on the head */
			  ior_saggital, /**<look from left or right on the head */
			  ior_unknown /**< stopper index */
};

/**
   Stream operator to write orientation orient to stream os 
   \param orient
   \param os
   \returns os 
 */
EXPORT_3D  std::ostream& operator << (std::ostream& os, E3DImageOrientation orient);

/**
   Stream operator to read orientation orient from  stream is 
   \param orient
   \param is
   \returns is 
 */
EXPORT_3D  std::istream& operator >> (std::istream& is, E3DImageOrientation& orient);

/// attribute for 3D image orientation 
typedef TAttribute<E3DImageOrientation> C3DImageOrientation;

/// translator for 3D image orientations to and from strings 
typedef TTranslator<E3DImageOrientation> COrientationTranslator;

NS_MIA_END

#endif
