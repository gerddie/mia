/*
 *  Copyright 1993, 1994 University of British Columbia
 *  Copyright (c) Leipzig, Madrid 2004 - 2012
 *  Max-Planck-Institute for Human Cognitive and Brain Science	
 *  Max-Planck-Institute for Evolutionary Anthropology 
 *  BIT, ETSI Telecomunicacion, UPM
 *
 *  Permission to use, copy, modify, distribute, and sell this software and its
 *  documentation for any purpose is hereby granted without fee, provided that
 *  the above copyright notice appears in all copies and that both that
 *  copyright notice and this permission notice appear in supporting
 *  documentation. UBC makes no representations about the suitability of this
 *  software for any purpose. It is provided "as is" without express or
 *  implied warranty.
 *
 *  Author: Arthur Pope, UBC Laboratory for Computational Intelligence
 *  Maintainance and Fixes: Gert Wollny, UPM 
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <string.h>

#include "vistaio/vcplist.h"
#include <assert.h>


/* Later in this file: */
static VistaIODecodeMethod     VCPEListDecodeMethod;
static VistaIOEncodeAttrMethod VCPEListEncodeAttrMethod;
static VistaIOEncodeDataMethod VCPEListEncodeDataMethod;
static VCPEList VistaIOCopyCPEList(VCPEList src);
static void VistaIODestroyCPEList(VCPEList field);  

/* Used in Type.c to register this type: */

VistaIOTypeMethods VCPEListMethods = {
	(VistaIOCopyMethod*)VistaIOCopyCPEList,		  /* copy a VistaIOField3D */
	(VistaIODestroyMethod*)VistaIODestroyCPEList,	          /* destroy a VistaIOField3D */
	VCPEListDecodeMethod,	  /* decode a VistaIOField3D's value */
	VCPEListEncodeAttrMethod,	  /* encode a VistaIOField3D's attr list */
	VCPEListEncodeDataMethod	  /* encode a VistaIOField3D's binary data */
};

/*! \brief
 *
 *  \param  _n_element
 *  \return VCPEList
 */

VCPEList VistaIOCreateCPEList(VistaIOLong _n_element)
{
	VCPEList result = (VCPEList)malloc(sizeof(VCPEListRec));
	result->n_length = _n_element; 
	result->nsize = result->n_length * sizeof(VCPERec);

	result->data = malloc(result->nsize);
	if (!result->data) {
		VistaIOWarning("VistaIOCreateCPEList: Unable to allocate %d byte of memory",result->nsize);
		return NULL; 
	}
	memset(result->data,0,result->nsize);
	result->attr=VistaIOCreateAttrList();
	return result; 
}

/*! \brief
 *
 *  \param  field
 */

void VistaIODestroyCPEList (VCPEList field)
{
	free(field->data);
	if (field->attr)
		VistaIODestroyAttrList(field->attr);
	free(field);
}

/*! \brief
 *
 *  \param  src
 *  \return VCPEList
 */

VCPEList VistaIOCopyCPEList (VCPEList src)
{
	VCPEList result;
	
	result = VistaIOCreateCPEList(src->n_length);
	if (result) {
		memcpy(result->data, src->data, src->nsize);
		VistaIODestroyAttrList(result->attr);
		result->attr = VistaIOCopyAttrList(src->attr);
	}
	return result; 
}

/*! \brief The "decode" method registered for the "Graph" type.
 *  
 *  Convert an attribute list plus binary data to a VistaIOGraph object.
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

static VistaIOPointer VCPEListDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VCPEList cplist;
	VistaIOLong n_length;
	VistaIOAttrList list;

	if (!VistaIOExtractAttr (b->list,CPLIST_LENGTH,NULL, VistaIOLongRepn, &n_length, TRUE)) return NULL;
	
	cplist = VistaIOCreateCPEList(n_length);
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
 *  VistaIOGraphEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  Encode an attribute list value for a VistaIOGraph object.
 */

static VistaIOAttrList VCPEListEncodeAttrMethod (VistaIOPointer value, size_t *lengthp)
{
	VCPEList cplist = value;
	VistaIOAttrList list;

	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = cplist->attr) == NULL)
		list = cplist->attr = VistaIOCreateAttrList ();
	
	VistaIOPrependAttr (list, CPLIST_LENGTH, NULL, VistaIOLongRepn,(VistaIOLong)cplist->n_length);  
	*lengthp = cplist->nsize;
	
	return list;
}


/*
 *  VistaIOGraphEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  Encode the edge and point lists for a VistaIOGraph object.
 */

static VistaIOPointer VCPEListEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean *free_itp)
{
	VCPEList cplist = value;
	VistaIOAttrListPosn posn;
	VistaIOPointer p;

	/* Remove the attributes prepended by the VCPEListEncodeAttrsMethod: */
	for (VistaIOFirstAttr (list, & posn);
	     strcmp (VistaIOGetAttrName (& posn), CPLIST_LENGTH) != 0; VistaIODeleteAttr (& posn));
	VistaIODeleteAttr (& posn);
	
	/* Allocate a buffer for the encoded data: */
	if (length == 0)  {
		*free_itp = FALSE;
		return value;			/* we may return anything != 0 here */
	};
	
	p  = VistaIOMalloc (length);
	memcpy(p, cplist->data, length);
	
#ifdef WORDS_BIGENDIAN
	convert_list(p,cplist->n_length);
#endif
	*free_itp = TRUE;
	return p;
}

