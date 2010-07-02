/*
** Copyright (c) Leipzig, Madrid 1999-2010
**                    Gert Wollny <wollny@cns.mpg.de>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
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

#ifndef __VField2D_h
#define __VField2D_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "vista.h"


#ifdef __cplusplus
extern "C" {
#endif

	typedef struct VField2DRec {
		VLong x_dim;
		VLong y_dim;
		VRepnKind repn;
		VLong nsize_element;
		VAttrList attr; 
                union {
                        VPointer data;
                        VPointerConst cdata;
                } p; 
		VBoolean owns_data; 
		VLong nsize; 
	} VField2DRec, *VField2D;
	
	EXPORT_VISTA VField2D VCreateField2DFrom(VLong _x_dim, 
				VLong _y_dim, 
				VLong nsize_elemet,
				VRepnKind repn,
				VPointerConst data);
	
	EXPORT_VISTA VField2D VCreateField2D(VLong _x_dim, 
				VLong _y_dim, 
				VLong nsize_elemet,
				VRepnKind repn);
	
	/* destroy the field and release all data (if owned) */
	EXPORT_VISTA void VDestroyField2D (VField2D field);
	
	/* copy the data field with all data */ 
	EXPORT_VISTA VField2D VCopyField2D (VField2D src);
	
	/* mirrors the datafield without copying the real data*/
	EXPORT_VISTA VField2D VMirrorField2D(VField2D src);
	
	
#define VField2DElement(FIELD, X, Y, type) &((type*)FIELD->data)[field->nsize_element * (x + nx * Y )]
	
	EXPORT_VISTA char *VGetField2DListName(VField2D field);	

extern VTypeMethods VField2DMethods; 
	
#ifdef __cplusplus
}
#endif
			   
#endif

