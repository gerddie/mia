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

#include "vistaio/vistaio.h"

/*
 *  Table of methods.
 */

/* Later in this file: */
static VistaIODecodeMethod VistaIOEdgesDecodeMethod;
static VistaIOEncodeAttrMethod VistaIOEdgesEncodeAttrMethod;
static VistaIOEncodeDataMethod VistaIOEdgesEncodeDataMethod;

/* Used in Type.c to register this type: */
VistaIOTypeMethods VistaIOEdgesMethods = {
	(VistaIOCopyMethod *) VistaIOCopyEdges,	/* copy a VistaIOEdges */
	(VistaIODestroyMethod *) VistaIODestroyEdges,	/* destroy a VistaIOEdges */
	VistaIOEdgesDecodeMethod,	/* decode a VistaIOEdges's value */
	VistaIOEdgesEncodeAttrMethod,	/* encode a VistaIOEdges's attr list */
	VistaIOEdgesEncodeDataMethod	/* encode a VistaIOEdges's binary data */
};


/*
 *  VistaIOEdgesDecodeMethod
 *
 *  The "decode" method registered for the "edges" type.
 *  Convert an attribute list plus binary data to a VistaIOEdges object.
 */

static VistaIOPointer VistaIOEdgesDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VistaIOEdges edges;
	VistaIOLong nrows, ncolumns, nedge_fields, npoint_fields, nedges, npoints;
	VistaIOAttrList list;
	int npnts, closed, i, nels;
	size_t length;
	VistaIOFloat *p;

#define Extract(name, dict, locn, required)	\
	VistaIOExtractAttr (b->list, name, dict, VistaIOLongRepn, & locn, required)

	/* Extract the required attribute values for edges. */
	if (!Extract (VistaIONRowsAttr, NULL, nrows, TRUE) ||
	    !Extract (VistaIONColumnsAttr, NULL, ncolumns, TRUE) ||
	    !Extract (VistaIONEdgeFieldsAttr, NULL, nedge_fields, TRUE) ||
	    !Extract (VistaIONPointFieldsAttr, NULL, npoint_fields, TRUE) ||
	    !Extract (VistaIONEdgesAttr, NULL, nedges, TRUE) ||
	    !Extract (VistaIONPointsAttr, NULL, npoints, TRUE))
		return NULL;
	if (npoint_fields <= 0) {
		VistaIOWarning ("VistaIOEdgesReadDataMethod: Bad edges file attributes");
		return NULL;
	}

	/* Create the edges data structure. */
	edges = VistaIOCreateEdges ((int)nrows, (int)ncolumns, (int)nedge_fields,
			      (int)npoint_fields);
	if (!edges)
		return NULL;

	/* Give it whatever attributes remain: */
	list = VistaIOEdgesAttrList (edges);
	VistaIOEdgesAttrList (edges) = b->list;
	b->list = list;

	/* Check that the expected amount of binary data was read: */
	nels = (nedges * (nedge_fields + 1) + npoints * npoint_fields);
	length = nels * VistaIORepnPrecision (VistaIOFloatRepn) / 8;
	if (length != b->length) {
		VistaIOWarning ("VistaIOEdgesDecodeMethod: %s image has wrong data length", name);
	      Fail:VistaIODestroyEdges (edges);
		return NULL;
	}

	/* Allocate storage for the edges binary data: */
	edges->free = VistaIOMalloc (length = nels * sizeof (VistaIOFloat));

	/* Unpack the binary data: */
	if (!VistaIOUnpackData
	    (VistaIOFloatRepn, nels, b->data, VistaIOMsbFirst, &length, &edges->free,
	     NULL))
		goto Fail;

	/* Create an edge record for each edge in this list.  The first number
	   in each edge's data encodes number of points and closed flag. */
	p = (VistaIOFloat *) edges->free;
	for (i = 0; i < nedges; i++) {
		closed = (int)(*p < 0.0);
		npnts = (int)(closed ? -*p++ : *p++);
		VistaIOAddEdge (edges, p, npnts, p + nedge_fields, closed, FALSE);
		p += nedge_fields + npnts * npoint_fields;
	}
	return edges;

#undef Extract
}


/*
 *  VistaIOEdgesEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "edges" type.
 *  Encode an attribute list value for a VistaIOEdges object.
 */

static VistaIOAttrList VistaIOEdgesEncodeAttrMethod (VistaIOPointer value, size_t * lengthp)
{
	VistaIOEdges edges = value;
	VistaIOAttrList list;

	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = VistaIOEdgesAttrList (edges)) == NULL)
		list = VistaIOEdgesAttrList (edges) = VistaIOCreateAttrList ();
	VistaIOPrependAttr (list, VistaIONColumnsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->ncolumns);
	VistaIOPrependAttr (list, VistaIONRowsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->nrows);
	VistaIOPrependAttr (list, VistaIONPointFieldsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->npoint_fields);
	VistaIOPrependAttr (list, VistaIONEdgeFieldsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->nedge_fields);
	VistaIOPrependAttr (list, VistaIONPointsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->npoints);
	VistaIOPrependAttr (list, VistaIONEdgesAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) edges->nedges);

	/* Compute the file space needed for the edges's binary data: */
	*lengthp = VistaIORepnPrecision (VistaIOFloatRepn) / 8 *
		(edges->nedges * (1 + edges->nedge_fields) +
		 edges->npoints * edges->npoint_fields);

	return list;
}


/*
 *  VistaIOEdgesEncodeDataMethod
 *
 *  The "encode_data" method registered for the "edges" type.
 *  Encode the edge and point fields for a VistaIOEdges object.
 */

static VistaIOPointer VistaIOEdgesEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean * free_itp)
{
	VistaIOEdges edges = value;
	VistaIOAttrListPosn posn;
	VistaIOEdge e;
	float pcount;
	size_t len;
	VistaIOPointer ptr, p;

	/* Remove the attributes prepended by the VistaIOEdgesEncodeAttrsMethod: */
	for (VistaIOFirstAttr (list, &posn);
	     strcmp (VistaIOGetAttrName (&posn), VistaIONColumnsAttr) != 0;
	     VistaIODeleteAttr (&posn));
	VistaIODeleteAttr (&posn);

	/* Allocate a buffer for the encoded data: */
	p = ptr = VistaIOMalloc (length);

	/* Pack each edge: */
	for (e = edges->first; e != NULL; e = e->next) {

		/* Pack the number of points (negated if closed): */
		pcount = (float)e->npoints;
		if (e->closed)
			pcount = -pcount;
		len = length;
		if (!VistaIOPackData
		    (VistaIOFloatRepn, 1, &pcount, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		/* Pack any edge fields: */
		if (edges->nedge_fields > 0) {
			len = length;
			if (!VistaIOPackData
			    (VistaIOFloatRepn, edges->nedge_fields, e->edge_fields,
			     VistaIOMsbFirst, &len, &p, NULL))
				return NULL;
			p = (char *)p + len;
			length -= len;
		}

		/* Pack point fields: */
		len = length;
		if (!VistaIOPackData (VistaIOFloatRepn, edges->npoint_fields * e->npoints,
				e->point_index[0], VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;
	}

	*free_itp = TRUE;
	return ptr;
}
