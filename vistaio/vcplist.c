/*
*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
*  
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* $Id: vcplist.c 52 2004-03-02 15:53:19Z tittge $ */

/*! \file  vcplist.c
 *  \brief basic support for list to store critical points
 *  \author Gert Wollny, wollny@cbs.mpg.de
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <string.h>

#include "vistaio/vcplist.h"
#include <assert.h>


/* Later in this file: */
static VDecodeMethod     VCPEListDecodeMethod;
static VEncodeAttrMethod VCPEListEncodeAttrMethod;
static VEncodeDataMethod VCPEListEncodeDataMethod;
static VCPEList VCopyCPEList(VCPEList src);
static void VDestroyCPEList(VCPEList field);  

/* Used in Type.c to register this type: */

VTypeMethods VCPEListMethods = {
	(VCopyMethod*)VCopyCPEList,		  /* copy a VField3D */
	(VDestroyMethod*)VDestroyCPEList,	          /* destroy a VField3D */
	VCPEListDecodeMethod,	  /* decode a VField3D's value */
	VCPEListEncodeAttrMethod,	  /* encode a VField3D's attr list */
	VCPEListEncodeDataMethod	  /* encode a VField3D's binary data */
};

/*! \brief
 *
 *  \param  _n_element
 *  \return VCPEList
 */

VCPEList VCreateCPEList(VLong _n_element)
{
	VCPEList result = (VCPEList)malloc(sizeof(VCPEListRec));
	result->n_length = _n_element; 
	result->nsize = result->n_length * sizeof(VCPERec);

	result->data = malloc(result->nsize);
	if (!result->data) {
		VWarning("VCreateCPEList: Unable to allocate %d byte of memory",result->nsize);
		return NULL; 
	}
	memset(result->data,0,result->nsize);
	result->attr=VCreateAttrList();
	return result; 
}

/*! \brief
 *
 *  \param  field
 */

void VDestroyCPEList (VCPEList field)
{
	free(field->data);
	if (field->attr)
		VDestroyAttrList(field->attr);
	free(field);
}

/*! \brief
 *
 *  \param  src
 *  \return VCPEList
 */

VCPEList VCopyCPEList (VCPEList src)
{
	VCPEList result;
	
	result = VCreateCPEList(src->n_length);
	if (result) {
		memcpy(result->data, src->data, src->nsize);
		VDestroyAttrList(result->attr);
		result->attr = VCopyAttrList(src->attr);
	}
	return result; 
}

/*! \brief The "decode" method registered for the "Graph" type.
 *  
 *  Convert an attribute list plus binary data to a VGraph object.
 *
 *  \param  f
 *  \param  n
 */

#ifdef WORDS_BIGENDIAN
static void convert_float(float *f, long n) 
{
	long *p = (long*)f; 
	while (n--) {
		*p = ((*p & 0xFF) << 24) | ((*p & 0xFF00) << 8) |
			((*p & 0xFF000000) >> 24) | ((*p & 0xFF0000) >> 8);
		p++;
	}
}

static void convert_list(VCPE r,int n) 
{
	int i; 
	for (i = 0; i < n; i++,r++) {
		convert_float(r->location,24);
	}
}

#endif

static VPointer VCPEListDecodeMethod (VStringConst name, VBundle b)
{
	VCPEList cplist;
	VLong n_length;
	VAttrList list;

	if (!VExtractAttr (b->list,CPLIST_LENGTH,NULL, VLongRepn, &n_length, TRUE)) return NULL;
	
	cplist = VCreateCPEList(n_length);
	if (!cplist)
		return NULL; 
	
	
	memcpy(cplist->data, b->data, cplist->nsize);
	
	list = cplist->attr; 
	cplist->attr = b->list; 
	b->list = list; 
	
#ifdef WORDS_BIGENDIAN
	convert_list(cplist->data,cplist->n_length);
#endif
	return cplist;
}


/*
 *  VGraphEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  Encode an attribute list value for a VGraph object.
 */

static VAttrList VCPEListEncodeAttrMethod (VPointer value, size_t *lengthp)
{
	VCPEList cplist = value;
	VAttrList list;

	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = cplist->attr) == NULL)
		list = cplist->attr = VCreateAttrList ();
	
	VPrependAttr (list, CPLIST_LENGTH, NULL, VLongRepn,(VLong)cplist->n_length);  
	*lengthp = cplist->nsize;
	
	return list;
}


/*
 *  VGraphEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  Encode the edge and point lists for a VGraph object.
 */

static VPointer VCPEListEncodeDataMethod (VPointer value, VAttrList list,
					size_t length, VBoolean *free_itp)
{
	VCPEList cplist = value;
	VAttrListPosn posn;
	VPointer p;

	/* Remove the attributes prepended by the VCPEListEncodeAttrsMethod: */
	for (VFirstAttr (list, & posn);
	     strcmp (VGetAttrName (& posn), CPLIST_LENGTH) != 0; VDeleteAttr (& posn));
	VDeleteAttr (& posn);
	
	/* Allocate a buffer for the encoded data: */
	if (length == 0)  {
		*free_itp = FALSE;
		return value;			/* we may return anything != 0 here */
	};
	
	p  = VMalloc (length);
	memcpy(p, cplist->data, length);
	
#ifdef WORDS_BIGENDIAN
	convert_list(p,cplist->n_length);
#endif
	*free_itp = TRUE;
	return p;
}

