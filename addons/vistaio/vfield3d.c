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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <string.h>

#include "vfield3d.h"
#include <assert.h>


/* Later in this file: */
static VistaIODecodeMethod     VistaIOField3DDecodeMethod;
static VistaIOEncodeAttrMethod VistaIOField3DEncodeAttrMethod;
static VistaIOEncodeDataMethod VistaIOField3DEncodeDataMethod;
VistaIOField3D VistaIOCopyField3D(VistaIOField3D src);
void VistaIODestroyField3D(VistaIOField3D field);  

/* Used in Type.c to register this type: */

VistaIOTypeMethods VistaIOField3DMethods = {
	(VistaIOCopyMethod*)VistaIOCopyField3D,		  /* copy a VistaIOField3D */
	(VistaIODestroyMethod*)VistaIODestroyField3D,	          /* destroy a VistaIOField3D */
	VistaIOField3DDecodeMethod,	  /* decode a VistaIOField3D's value */
	VistaIOField3DEncodeAttrMethod,	  /* encode a VistaIOField3D's attr list */
	VistaIOField3DEncodeDataMethod	  /* encode a VistaIOField3D's binary data */
};


static char name[1024];

EXPORT_VISTA char *VistaIOGetField3DListName(VistaIOField3D field) 
{

	sprintf(name,"%dDVector",field->nsize_element);
	return name; 
}

EXPORT_VISTA VistaIOField3D VistaIOCreateField3DFrom(VistaIOLong x_dim, 
			VistaIOLong y_dim, 
			VistaIOLong z_dim, 
			VistaIOLong nsize_element,
			VistaIORepnKind repn,
			VistaIOPointerConst data)
{
	VistaIOField3D result = (VistaIOField3D)malloc(sizeof(VistaIOField3DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
	result->z_dim = z_dim;	
	result->repn = repn; 
	result->nsize_element = nsize_element; 
	result->nsize = nsize_element; 

	switch (repn) {
	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:break;
		
	case VistaIOShortRepn:result->nsize *= 2;
		break;
		
	case VistaIOLongRepn:
	case VistaIOFloatRepn:result->nsize *= 4; 
		break;
	case VistaIODoubleRepn:result->nsize *= 8;
		break;
	default:VistaIOWarning("Requested wrong Repn type in VistaIOCreateField3D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim * z_dim; 

	result->p.cdata = data; 
	result->owns_data = FALSE; 
	result->attr=VistaIOCreateAttrList();	
	return result; 
}


EXPORT_VISTA VistaIOField3D VistaIOCreateField3D(VistaIOLong x_dim, 
		    VistaIOLong y_dim, 
		    VistaIOLong z_dim, 
		    VistaIOLong nsize_element,
		    VistaIORepnKind repn)
{
	VistaIOField3D result = (VistaIOField3D)malloc(sizeof(VistaIOField3DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
	result->z_dim = z_dim;	
	result->repn = repn; 
	result->nsize_element = nsize_element;
	result->nsize = nsize_element; 

	switch (repn) {
	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:break;
		
	case VistaIOShortRepn:result->nsize *= 2;
		break;
		
	case VistaIOLongRepn:
	case VistaIOFloatRepn:result->nsize *= 4; 
		break;
	case VistaIODoubleRepn:result->nsize *= 8;
		break;
	default:VistaIOWarning("Requested wrong Repn type in VistaIOCreateField3D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim * z_dim; 

	result->p.data = malloc(result->nsize);
	if (!result->p.data) {
		VistaIOWarning("VistaIOCreateField3D: Unable to allocate %d byte of memory",result->nsize);
		return NULL; 
	}
	memset(result->p.data,0,result->nsize);
	result->owns_data = TRUE;
	result->attr=VistaIOCreateAttrList();
	return result; 
}


EXPORT_VISTA void VistaIODestroyField3D (VistaIOField3D field)
{
	if (field->owns_data) 
		free(field->p.data);
	if (field->attr)
		VistaIODestroyAttrList(field->attr);
	free(field);
}

EXPORT_VISTA VistaIOField3D VistaIOCopyField3D (VistaIOField3D src)
{
	VistaIOField3D result;
	
	result = VistaIOCreateField3D(src->x_dim,
				src->y_dim,
				src->z_dim,
				src->nsize_element,
				src->repn); 
	if (result) {
		memcpy(result->p.data, src->p.data, src->nsize);
		VistaIODestroyAttrList(result->attr);
		result->attr = VistaIOCopyAttrList(src->attr);
	}
	
	return result; 
}

EXPORT_VISTA VistaIOField3D VistaIOMirrorField3D (VistaIOField3D src)
{
	VistaIOField3D result;
	
	result = VistaIOCreateField3DFrom(src->x_dim,
				src->y_dim,
				src->z_dim,
				src->nsize_element,
				src->repn,
				src->p.data);
	if (result) {
		VistaIODestroyAttrList(result->attr);
		result->attr = VistaIOCopyAttrList(src->attr);
	}
	
	return result; 
}



/*
 *  VistaIOField3DDecodeMethod
 *
 *  The "decode" method registered for the "Graph" type.
 *  Convert an attribute list plus binary data to a VistaIOGraph object.
 */

#ifdef WORDS_BIGENDIAN
static void convert_short_field(short *p, long n) 
{
	while (n--) {
		*p = ((*p & 0xFF) << 8) | ((*p & 0xFF00) >> 8);
		p++;
	}
	
}
static void convert_long_field(long *p, long n) 
{
	while (n--) {
		*p = ((*p & 0xFF) << 24) | ((*p & 0xFF00) << 8) |
			((*p & 0xFF000000) >> 24) | ((*p & 0xFF0000) >> 8);
		p++;
	}
	
}
static void convert_double_field(double *d, long n)
{
	long *p = (long *)d; 
	while (n--) {
		long help;
		long help2; 
		
		help = ((*p & 0xFF) << 24) | ((*p & 0xFF00) << 8) |
			((*p & 0xFF000000) >> 24) | ((*p & 0xFF0000) >> 8);
		
		help2 = ((p[1]& 0xFF) << 24) |        ((p[1] & 0xFF00) << 8) |
			((p[1] & 0xFF000000) >> 24) | ((p[1] & 0xFF0000) >> 8);
		*p++ = help2;
		*p++ = help; 
	}
}

#endif

static VistaIOPointer VistaIOField3DDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VistaIOField3D field;
	VistaIOLong x_dim,y_dim,z_dim, node_repn, nelement_size;
	
	VistaIOAttrList list;
	
	
	if (!VistaIOExtractAttr (b->list,"x_dim",NULL, VistaIOLongRepn, &x_dim, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,"y_dim",NULL, VistaIOLongRepn, &y_dim, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,"z_dim",NULL, VistaIOLongRepn, &z_dim, TRUE)) return NULL;	
	if (!VistaIOExtractAttr (b->list,COMPONENTS,NULL, VistaIOLongRepn, &nelement_size, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,VistaIORepnAttr,VistaIONumericRepnDict, VistaIOLongRepn, &node_repn, TRUE)) return NULL;  	  
	
	if (x_dim==0 || y_dim==0 || z_dim==0 || nelement_size <= 0) {
		VistaIOWarning ("VistaIOField3DDecodeMethod: Bad Field3D file attributes");
		return NULL;
	}
	
	field = VistaIOCreateField3D(x_dim,y_dim,z_dim, nelement_size,node_repn);
	if (!field)
		return NULL; 
	
	memcpy(field->p.data, b->data, field->nsize);
	
	list = field->attr; 
	field->attr = b->list; 
	b->list = list; 
	
#ifdef WORDS_BIGENDIAN
	switch (node_repn) {
	case VistaIOShortRepn:
		convert_short_field(field->p.data,field->nsize / 2);
		break;
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
		convert_long_field(field->p.data,field->nsize / 4);
		break; 
	case VistaIODoubleRepn:
		convert_double_field(field->p.data, field->nsize / 8);
		break; 
		
        /*default:  Bit and Byte will not be switched, others are rejected by VistaIOCreateField3D */
	}
#endif
	return field;
}


/*
 *  VistaIOGraphEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  Encode an attribute list value for a VistaIOGraph object.
 */

static VistaIOAttrList VistaIOField3DEncodeAttrMethod (VistaIOPointer value, size_t *lengthp)
{
	VistaIOField3D field = value;
	VistaIOAttrList list;
	
	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = field->attr) == NULL)
		list = field->attr = VistaIOCreateAttrList ();
	
	VistaIOPrependAttr (list, VistaIORepnAttr, VistaIONumericRepnDict,VistaIOLongRepn, (VistaIOLong) field->repn);	
	VistaIOPrependAttr (list, COMPONENTS, NULL, VistaIOLongRepn,(VistaIOLong)field->nsize_element);  
	VistaIOPrependAttr (list, "z_dim", NULL, VistaIOLongRepn,(VistaIOLong)field->z_dim);  
	VistaIOPrependAttr (list, "y_dim", NULL, VistaIOLongRepn,(VistaIOLong)field->y_dim);
	VistaIOPrependAttr (list, "x_dim", NULL, VistaIOLongRepn,(VistaIOLong)field->x_dim);

	*lengthp = field->nsize;
	
	return list;
}


/*
 *  VistaIOGraphEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  Encode the edge and point fields for a VistaIOGraph object.
 */

static VistaIOPointer VistaIOField3DEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean *free_itp)
{
	VistaIOField3D field = value;
	VistaIOAttrListPosn posn;
	VistaIOPointer p;
	
	/* Remove the attributes prepended by the VistaIOField3DEncodeAttrsMethod: */
	for (VistaIOFirstAttr (list, & posn);
	     strcmp (VistaIOGetAttrName (& posn), VistaIORepnAttr) != 0; VistaIODeleteAttr (& posn));
	VistaIODeleteAttr (& posn);
	
	/* Allocate a buffer for the encoded data: */
	if (length == 0)  {
		*free_itp = FALSE;
		return value;			/* we may return anything != 0 here */
	};
	
	p  = VistaIOMalloc (length);
	memcpy(p, field->p.data, length);
	
#ifdef WORDS_BIGENDIAN
	switch (field->repn) {
	case VistaIOShortRepn:
		convert_short_field(p,field->nsize / 2);
		break;
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
		convert_long_field(p,field->nsize / 4);
		break; 
	case VistaIODoubleRepn:
		convert_double_field(p, field->nsize / 8);
		break; 
	default: 
                break; 
        /*default:  Bit and Byte will not be switched, others are rejected by VistaIOCreateField3D */
	}
#endif
	*free_itp = TRUE;
	return p;
}
