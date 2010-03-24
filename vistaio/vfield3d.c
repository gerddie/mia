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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <string.h>

#include "vfield3d.h"
#include <assert.h>


/* Later in this file: */
static VDecodeMethod     VField3DDecodeMethod;
static VEncodeAttrMethod VField3DEncodeAttrMethod;
static VEncodeDataMethod VField3DEncodeDataMethod;
VField3D VCopyField3D(VField3D src);
void VDestroyField3D(VField3D field);  

/* Used in Type.c to register this type: */

VTypeMethods VField3DMethods = {
	(VCopyMethod*)VCopyField3D,		  /* copy a VField3D */
	(VDestroyMethod*)VDestroyField3D,	          /* destroy a VField3D */
	VField3DDecodeMethod,	  /* decode a VField3D's value */
	VField3DEncodeAttrMethod,	  /* encode a VField3D's attr list */
	VField3DEncodeDataMethod	  /* encode a VField3D's binary data */
};


static char name[1024];

EXPORT_VISTA char *VGetField3DListName(VField3D field) 
{

	sprintf(name,"%dDVector",field->nsize_element);
	return name; 
}

EXPORT_VISTA VField3D VCreateField3DFrom(VLong x_dim, 
			VLong y_dim, 
			VLong z_dim, 
			VLong nsize_element,
			VRepnKind repn,
			VPointerConst data)
{
	VField3D result = (VField3D)malloc(sizeof(VField3DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
	result->z_dim = z_dim;	
	result->repn = repn; 
	result->nsize_element = nsize_element; 
	result->nsize = nsize_element; 

	switch (repn) {
	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:break;
		
	case VShortRepn:result->nsize *= 2;
		break;
		
	case VLongRepn:
	case VFloatRepn:result->nsize *= 4; 
		break;
	case VDoubleRepn:result->nsize *= 8;
		break;
	default:VWarning("Requested wrong Repn type in VCreateField3D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim * z_dim; 

	result->p.cdata = data; 
	result->owns_data = FALSE; 
	result->attr=VCreateAttrList();	
	return result; 
}


EXPORT_VISTA VField3D VCreateField3D(VLong x_dim, 
		    VLong y_dim, 
		    VLong z_dim, 
		    VLong nsize_element,
		    VRepnKind repn)
{
	VField3D result = (VField3D)malloc(sizeof(VField3DRec));
	result->x_dim = x_dim;
	result->y_dim = y_dim;
	result->z_dim = z_dim;	
	result->repn = repn; 
	result->nsize_element = nsize_element;
	result->nsize = nsize_element; 

	switch (repn) {
	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:break;
		
	case VShortRepn:result->nsize *= 2;
		break;
		
	case VLongRepn:
	case VFloatRepn:result->nsize *= 4; 
		break;
	case VDoubleRepn:result->nsize *= 8;
		break;
	default:VWarning("Requested wrong Repn type in VCreateField3D");
		return NULL; 
	}
	
	result->nsize *= x_dim * y_dim * z_dim; 

	result->p.data = malloc(result->nsize);
	if (!result->p.data) {
		VWarning("VCreateField3D: Unable to allocate %d byte of memory",result->nsize);
		return NULL; 
	}
	memset(result->p.data,0,result->nsize);
	result->owns_data = TRUE;
	result->attr=VCreateAttrList();
	return result; 
}


EXPORT_VISTA void VDestroyField3D (VField3D field)
{
	if (field->owns_data) 
		free(field->p.data);
	if (field->attr)
		VDestroyAttrList(field->attr);
	free(field);
}

EXPORT_VISTA VField3D VCopyField3D (VField3D src)
{
	VField3D result;
	
	result = VCreateField3D(src->x_dim,
				src->y_dim,
				src->z_dim,
				src->nsize_element,
				src->repn); 
	if (result) {
		memcpy(result->p.data, src->p.data, src->nsize);
		VDestroyAttrList(result->attr);
		result->attr = VCopyAttrList(src->attr);
	}
	
	return result; 
}

EXPORT_VISTA VField3D VMirrorField3D (VField3D src)
{
	VField3D result;
	
	result = VCreateField3DFrom(src->x_dim,
				src->y_dim,
				src->z_dim,
				src->nsize_element,
				src->repn,
				src->p.data);
	if (result) {
		VDestroyAttrList(result->attr);
		result->attr = VCopyAttrList(src->attr);
	}
	
	return result; 
}



/*
 *  VField3DDecodeMethod
 *
 *  The "decode" method registered for the "Graph" type.
 *  Convert an attribute list plus binary data to a VGraph object.
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

static VPointer VField3DDecodeMethod (VStringConst name, VBundle b)
{
	VField3D field;
	VLong x_dim,y_dim,z_dim, node_repn, nelement_size;
	
	VAttrList list;
	
	
	if (!VExtractAttr (b->list,"x_dim",NULL, VLongRepn, &x_dim, TRUE)) return NULL;
	if (!VExtractAttr (b->list,"y_dim",NULL, VLongRepn, &y_dim, TRUE)) return NULL;
	if (!VExtractAttr (b->list,"z_dim",NULL, VLongRepn, &z_dim, TRUE)) return NULL;	
	if (!VExtractAttr (b->list,COMPONENTS,NULL, VLongRepn, &nelement_size, TRUE)) return NULL;
	if (!VExtractAttr (b->list,VRepnAttr,VNumericRepnDict, VLongRepn, &node_repn, TRUE)) return NULL;  	  
	
	if (x_dim==0 || y_dim==0 || z_dim==0 || nelement_size <= 0) {
		VWarning ("VField3DDecodeMethod: Bad Field3D file attributes");
		return NULL;
	}
	
	field = VCreateField3D(x_dim,y_dim,z_dim, nelement_size,node_repn);
	if (!field)
		return NULL; 
	
	memcpy(field->p.data, b->data, field->nsize);
	
	list = field->attr; 
	field->attr = b->list; 
	b->list = list; 
	
#ifdef WORDS_BIGENDIAN
	switch (node_repn) {
	case VShortRepn:
		convert_short_field(field->p.data,field->nsize / 2);
		break;
	case VLongRepn:
	case VFloatRepn:
		convert_long_field(field->p.data,field->nsize / 4);
		break; 
	case VDoubleRepn:
		convert_double_field(field->p.data, field->nsize / 8);
		break; 
		
        /*default:  Bit and Byte will not be switched, others are rejected by VCreateField3D */
	}
#endif
	return field;
}


/*
 *  VGraphEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  Encode an attribute list value for a VGraph object.
 */

static VAttrList VField3DEncodeAttrMethod (VPointer value, size_t *lengthp)
{
	VField3D field = value;
	VAttrList list;
	
	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = field->attr) == NULL)
		list = field->attr = VCreateAttrList ();
	
	VPrependAttr (list, VRepnAttr, VNumericRepnDict,VLongRepn, (VLong) field->repn);	
	VPrependAttr (list, COMPONENTS, NULL, VLongRepn,(VLong)field->nsize_element);  
	VPrependAttr (list, "z_dim", NULL, VLongRepn,(VLong)field->z_dim);  
	VPrependAttr (list, "y_dim", NULL, VLongRepn,(VLong)field->y_dim);
	VPrependAttr (list, "x_dim", NULL, VLongRepn,(VLong)field->x_dim);

	*lengthp = field->nsize;
	
	return list;
}


/*
 *  VGraphEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  Encode the edge and point fields for a VGraph object.
 */

static VPointer VField3DEncodeDataMethod (VPointer value, VAttrList list,
					size_t length, VBoolean *free_itp)
{
	VField3D field = value;
	VAttrListPosn posn;
	VPointer p;
	
	/* Remove the attributes prepended by the VField3DEncodeAttrsMethod: */
	for (VFirstAttr (list, & posn);
	     strcmp (VGetAttrName (& posn), VRepnAttr) != 0; VDeleteAttr (& posn));
	VDeleteAttr (& posn);
	
	/* Allocate a buffer for the encoded data: */
	if (length == 0)  {
		*free_itp = FALSE;
		return value;			/* we may return anything != 0 here */
	};
	
	p  = VMalloc (length);
	memcpy(p, field->p.data, length);
	
#ifdef WORDS_BIGENDIAN
	switch (field->repn) {
	case VShortRepn:
		convert_short_field(p,field->nsize / 2);
		break;
	case VLongRepn:
	case VFloatRepn:
		convert_long_field(p,field->nsize / 4);
		break; 
	case VDoubleRepn:
		convert_double_field(p, field->nsize / 8);
		break; 
	default: 
                break; 
        /*default:  Bit and Byte will not be switched, others are rejected by VCreateField3D */
	}
#endif
	*free_itp = TRUE;
	return p;
}
