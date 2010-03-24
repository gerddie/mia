/*
** Copyright (C) 1999 Max-Planck-Institute of Cognitive Neurosience
**                    Gert Wollny <wollny@cns.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef __VField3D_h
#define __VField3D_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vista.h"


#ifdef __cplusplus
extern "C" {
#endif

	typedef struct VField3DRec {
		VLong x_dim;
		VLong y_dim;
		VLong z_dim; 
		VRepnKind repn;
		VLong nsize_element;
		VAttrList attr; 
                union {
                        VPointer data;
                        VPointerConst cdata;
                } p; 
		VBoolean owns_data; 
		VLong nsize; 
	} VField3DRec, *VField3D;
	
	EXPORT_VISTA VField3D VCreateField3DFrom(VLong _x_dim, 
				VLong _y_dim, 
				VLong _z_dim, 
				VLong nsize_elemet,
				VRepnKind repn,
				VPointerConst data);
	
	EXPORT_VISTA VField3D VCreateField3D(VLong _x_dim, 
				VLong _y_dim, 
				VLong _z_dim, 
				VLong nsize_elemet,
				VRepnKind repn);
	
	/* destroy the field and release all data (if owned) */
	EXPORT_VISTA void VDestroyField3D (VField3D field);
	
	/* copy the data field with all data */ 
	EXPORT_VISTA VField3D VCopyField3D (VField3D src);
	
	/* mirrors the datafield without copying the real data*/
	EXPORT_VISTA VField3D VMirrorField3D(VField3D src);
	
	
#define VField3DElement(FIELD, X, Y, Z, type) &((type*)FIELD->data)[field->nsize_element * (x + nx * ( Y + ny * Z))]
	
	EXPORT_VISTA char *VGetField3DListName(VField3D field);	

extern VTypeMethods VField3DMethods; 
	
#ifdef __cplusplus
}
#endif
			   
#endif

