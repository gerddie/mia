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

/*! \brief Allocates memory for a VistaIOEdges structure and initializes its fields.
 *    
 *  Initially, this contains zero edges, so each edge must still
 *  be created and added to this record.
 *
 *  \param  nrows
 *  \param  ncolumns
 *  \param  nedge_fields
 *  \param  npoint_fields
 *  \return Returns a pointer to the edges if successful, NULL otherwise.
 */

VistaIOEdges VistaIOCreateEdges (int nrows, int ncolumns, int nedge_fields,
		     int npoint_fields)
{
	VistaIOEdges edges;

	/* Check parameters: */
	if (nrows < 1 || ncolumns < 1)
		VistaIOWarning ("VistaIOCreateEdges: Invalid number of rows or columns.");

	/* Allocate memory for the VistaIOEdges, its indices, and pixel values: */
	edges = VistaIOMalloc (sizeof (VistaIOEdgesRec));

	/* Initialize the VistaIOEdges: */
	edges->nrows = nrows;
	edges->ncolumns = ncolumns;
	edges->attributes = VistaIOCreateAttrList ();
	edges->nedge_fields = nedge_fields;
	edges->npoint_fields = npoint_fields;
	edges->nedges = edges->npoints = 0;
	edges->first = edges->last = NULL;
	edges->free = NULL;

	return edges;
}


/*! \brief Add an edge to the given Edges record.  If the "copy" argument is
 *    TRUE, then new space is allocated to copy the points and the fields
 *    of this edge.  Otherwise, a pointer is created to their current 
 *    location.  
 *  
 *  \param  edges
 *  \param  edge_fields
 *  \param  npoints the number of points in this edge
 *  \param  points
 *  \param  closed indicates if this is a closed edge.
 *  \param  copy
 *  \return VistaIOEdge
 */

VistaIOEdge VistaIOAddEdge (VistaIOEdges edges, VistaIOFloat * edge_fields, int npoints,
		VistaIOFloat * points, VistaIOBooleanPromoted closed,
		VistaIOBooleanPromoted copy)
{
	VistaIOEdge edge = VistaIOMalloc (sizeof (VistaIOEdgeRec));
	size_t fsize, psize, isize;
	int i;
	VistaIOPointer p;
	VistaIOFloat *pdata;

	/* Add the edge to the end of the current list of edges in order to
	   maintain a consistent ordering of edges during IO. */
	if (edges->last == NULL)
		edges->first = edge;
	else
		edges->last->next = edge;
	edges->last = edge;
	edge->next = NULL;
	edges->nedges += 1;
	edges->npoints += npoints;
	edge->npoints = npoints;
	edge->closed = closed;
	isize = sizeof (VistaIOFloat *) * npoints;	/* Size of points index array. */

	/* If copying data, enough space is allocated to hold everything. */
	if (copy) {
#ifndef __alpha
		fsize = sizeof (VistaIOFloat) * edges->nedge_fields;
		psize = sizeof (VistaIOFloat) * npoints * edges->npoint_fields;
#else
		/* pointers must be quadword-aligned on a DEC alpha */
#define quadalign(a)	((((a)-1)/8+1)*8)
		fsize = quadalign (sizeof (VistaIOFloat) * edges->nedge_fields);
		psize = quadalign (sizeof (VistaIOFloat) * npoints *
				   edges->npoint_fields);
#endif
		p = VistaIOMalloc (fsize + psize + isize);
		edge->free = p;
		edge->edge_fields = (VistaIOFloat *) p;
		if (fsize > 0)
			memcpy (p, edge_fields, fsize);
		pdata = (VistaIOFloat *) ((char *)p + fsize);
		memcpy (pdata, points, psize);
		edge->point_index = (VistaIOFloat **) ((char *)p + fsize + psize);
	} else {
		p = VistaIOMalloc (isize);
		edge->free = p;
		edge->edge_fields = edge_fields;
		pdata = points;
		edge->point_index = (VistaIOFloat **) p;
	}

	/* Initialize index array into set of points. */
	for (i = 0; i < npoints; i++)
		edge->point_index[i] = pdata + i * edges->npoint_fields;

	return edge;
}


/*! \brief Copy a VistaIOEdges object.
 *
 *  \param  src
 *  \return VistaIOEdges
 */

VistaIOEdges VistaIOCopyEdges (VistaIOEdges src)
{
	VistaIOEdges result;
	VistaIOEdge e;

	result = VistaIOCreateEdges (src->nrows, src->ncolumns, src->nedge_fields,
			       src->npoint_fields);
	for (e = src->first; e != NULL; e = e->next)
		VistaIOAddEdge (result, e->edge_fields, e->npoints,
			  e->point_index[0], e->closed, TRUE);
	if (VistaIOEdgesAttrList (result))
		VistaIODestroyAttrList (VistaIOEdgesAttrList (result));
	if (VistaIOEdgesAttrList (src))
		VistaIOEdgesAttrList (result) =
			VistaIOCopyAttrList (VistaIOEdgesAttrList (src));
	return result;
}


/*! \brief Frees memory occupied by set of edges.
 *
 *  \param edges
 */

void VistaIODestroyEdges (VistaIOEdges edges)
{
	VistaIOEdge edge, next_edge;

	for (edge = edges->first; edge; edge = next_edge) {
		next_edge = edge->next;
		if (edge->free)
			VistaIOFree (edge->free);
	}
	if (edges->free)
		VistaIOFree (edges->free);
	VistaIODestroyAttrList (edges->attributes);
	VistaIOFree (edges);
}


/*! \brief Read a Vista data file, extract the edge sets from it, 
 *         and return a list of them.
 *  
 *  \param  file
 *  \param  attributes
 *  \param  edge_sets
 */

int VistaIOReadEdges (FILE * file, VistaIOAttrList * attributes, VistaIOEdges ** edge_sets)
{
	return VistaIOReadObjects (file, VistaIOEdgesRepn, attributes,
			     (VistaIOPointer **) edge_sets);
}


/*! \brief Write a list of edge sets to a Vista data file.
 *
 *  \param  file
 *  \param  attributes
 *  \param  nedge_sets
 *  \param  edge_sets
 *  \return VistaIOBoolean
 */

VistaIOBoolean VistaIOWriteEdges (FILE * file, VistaIOAttrList attributes, int nedge_sets,
		      VistaIOEdges edge_sets[])
{
	return VistaIOWriteObjects (file, VistaIOEdgesRepn, attributes, nedge_sets,
			      (VistaIOPointer *) edge_sets);
}
