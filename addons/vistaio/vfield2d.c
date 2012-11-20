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

#include "vfield2d.h"
#include <assert.h>


/* Later in this file: */
static VistaIODecodeMethod     VistaIOField2DDecodeMethod;
static VistaIOEncodeAttrMethod VistaIOField2DEncodeAttrMethod;
static VistaIOEncodeDataMethod VistaIOField2DEncodeDataMethod;
VistaIOField2D VistaIOCopyField2D(VistaIOField2D src);
void VistaIODestroyField2D(VistaIOField2D field);  

/* Used in Type.c to register this type: */

VistaIOTypeMethods VistaIOField2DMethods = {
	(VistaIOCopyMethod*)VistaIOCopyField2D,		  /* copy a VistaIOField2D */
	(VistaIODestroyMethod*)VistaIODestroyField2D,	          /* destroy a VistaIOField2D */
	VistaIOField2DDecodeMethod,	  /* decode a VistaIOField2D's value */
	VistaIOField2DEncodeAttrMethod,	  /* encode a VistaIOField2D's attr list */
	VistaIOField2DEncodeDataMethod	  /* encode a VistaIOField2D's binary data */
};



EXPORT_VISTA VistaIOField2D VistaIOCreateField2DFrom(VistaIOLong x_dim, 
			VistaIOLong y_dim, 
			VistaIOLong nsize_element,
			VistaIORepnKind repn,
			VistaIOPointerConst data)
{
	VistaIOField2D result = (VistaIOField2D)malloc(sizeof(VistaIOField2DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
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
	default:VistaIOWarning("Requested wrong Repn type in VistaIOCreateField2D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim; 

	result->p.cdata = data; 
	result->owns_data = FALSE; 
	result->attr=VistaIOCreateAttrList();	
	return result; 
}


EXPORT_VISTA VistaIOField2D VistaIOCreateField2D(VistaIOLong x_dim, 
		    VistaIOLong y_dim, 
		    VistaIOLong nsize_element,
		    VistaIORepnKind repn)
{
	VistaIOField2D result = (VistaIOField2D)malloc(sizeof(VistaIOField2DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
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
	default:VistaIOWarning("Requested wrong Repn type in VistaIOCreateField2D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim; 

	result->p.data = malloc(result->nsize);
	if (!result->p.data) {
		VistaIOWarning("VistaIOCreateField2D: Unable to allocate %d byte of memory",result->nsize);
		return NULL; 
	}
	memset(result->p.data,0,result->nsize);
	result->owns_data = TRUE;
	result->attr=VistaIOCreateAttrList();
	return result; 
}


EXPORT_VISTA void VistaIODestroyField2D (VistaIOField2D field)
{
	if (field->owns_data) 
		free(field->p.data);
	if (field->attr)
		VistaIODestroyAttrList(field->attr);
	free(field);
}

EXPORT_VISTA VistaIOField2D VistaIOCopyField2D (VistaIOField2D src)
{
	VistaIOField2D result;
	
	result = VistaIOCreateField2D(src->x_dim,
				src->y_dim,
				src->nsize_element,
				src->repn); 
	if (result) {
		memcpy(result->p.data, src->p.data, src->nsize);
		VistaIODestroyAttrList(result->attr);
		result->attr = VistaIOCopyAttrList(src->attr);
	}
	
	return result; 
}

EXPORT_VISTA VistaIOField2D VistaIOMirrorField2D (VistaIOField2D src)
{
	VistaIOField2D result;
	
	result = VistaIOCreateField2DFrom(src->x_dim,
				src->y_dim,
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
 *  VistaIOField2DDecodeMethod
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

static VistaIOPointer VistaIOField2DDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VistaIOField2D field;
	VistaIOLong x_dim,y_dim, node_repn, nelement_size;
	
	VistaIOAttrList list;
	
	
	if (!VistaIOExtractAttr (b->list,"x_dim",NULL, VistaIOLongRepn, &x_dim, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,"y_dim",NULL, VistaIOLongRepn, &y_dim, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,COMPONENTS,NULL, VistaIOLongRepn, &nelement_size, TRUE)) return NULL;
	if (!VistaIOExtractAttr (b->list,VistaIORepnAttr,VistaIONumericRepnDict, VistaIOLongRepn, &node_repn, TRUE)) return NULL;  	  
	
	if (x_dim==0 || y_dim==0 || nelement_size <= 0) {
		VistaIOWarning ("VistaIOField2DDecodeMethod: Bad Field2D file attributes");
		return NULL;
	}
	
	field = VistaIOCreateField2D(x_dim,y_dim,nelement_size,node_repn);
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
		
        /*default:  Bit and Byte will not be switched, others are rejected by VistaIOCreateField2D */
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

static VistaIOAttrList VistaIOField2DEncodeAttrMethod (VistaIOPointer value, size_t *lengthp)
{
	VistaIOField2D field = value;
	VistaIOAttrList list;
	
	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = field->attr) == NULL)
		list = field->attr = VistaIOCreateAttrList ();
	
	VistaIOPrependAttr (list, VistaIORepnAttr, VistaIONumericRepnDict,VistaIOLongRepn, (VistaIOLong) field->repn);	
	VistaIOPrependAttr (list, COMPONENTS, NULL, VistaIOLongRepn,(VistaIOLong)field->nsize_element);  
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

static VistaIOPointer VistaIOField2DEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean *free_itp)
{
	VistaIOField2D field = value;
	VistaIOAttrListPosn posn;
	VistaIOPointer p;
	
	/* Remove the attributes prepended by the VistaIOField2DEncodeAttrsMethod: */
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
        /*default:  Bit and Byte will not be switched, others are rejected by VistaIOCreateField2D */
	}
#endif
	*free_itp = TRUE;
	return p;
}
