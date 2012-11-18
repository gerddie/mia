/*
** Copyright (c) Leipzig, Madrid 1999-2012
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

	typedef struct VistaIOField2DRec {
		VistaIOLong x_dim;
		VistaIOLong y_dim;
		VistaIORepnKind repn;
		VistaIOLong nsize_element;
		VistaIOAttrList attr; 
                union {
                        VistaIOPointer data;
                        VistaIOPointerConst cdata;
                } p; 
		VistaIOBoolean owns_data; 
		VistaIOLong nsize; 
	} VistaIOField2DRec, *VistaIOField2D;
	
	EXPORT_VISTA VistaIOField2D VistaIOCreateField2DFrom(VistaIOLong _x_dim, 
				VistaIOLong _y_dim, 
				VistaIOLong nsize_elemet,
				VistaIORepnKind repn,
				VistaIOPointerConst data);
	
	EXPORT_VISTA VistaIOField2D VistaIOCreateField2D(VistaIOLong _x_dim, 
				VistaIOLong _y_dim, 
				VistaIOLong nsize_elemet,
				VistaIORepnKind repn);
	
	/* destroy the field and release all data (if owned) */
	EXPORT_VISTA void VistaIODestroyField2D (VistaIOField2D field);
	
	/* copy the data field with all data */ 
	EXPORT_VISTA VistaIOField2D VistaIOCopyField2D (VistaIOField2D src);
	
	/* mirrors the datafield without copying the real data*/
	EXPORT_VISTA VistaIOField2D VistaIOMirrorField2D(VistaIOField2D src);
	
	
#define VistaIOField2DElement(FIELD, X, Y, type) &((type*)FIELD->data)[field->nsize_element * (x + nx * Y )]
	
	EXPORT_VISTA char *VistaIOGetField2DListName(VistaIOField2D field);	

extern VistaIOTypeMethods VistaIOField2DMethods; 
	
#ifdef __cplusplus
}
#endif
			   
#endif

