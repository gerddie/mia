/*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
*
* The origional VISTA library is copyrighted of University of British Columbia.
* Copyright © 1993, 1994 University of British Columbia.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser Public License as published by
* the Free Software Foundation; either version 3 of the License, or
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

/* $Id: edges.c 52 2004-03-02 15:53:19Z tittge $ */

/*! \file  edges.c
 *  \brief basic support for edges
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/*! \brief Allocates memory for a VEdges structure and initializes its fields.
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

VEdges VCreateEdges (int nrows, int ncolumns, int nedge_fields,
		     int npoint_fields)
{
	VEdges edges;

	/* Check parameters: */
	if (nrows < 1 || ncolumns < 1)
		VWarning ("VCreateEdges: Invalid number of rows or columns.");

	/* Allocate memory for the VEdges, its indices, and pixel values: */
	edges = VMalloc (sizeof (VEdgesRec));

	/* Initialize the VEdges: */
	edges->nrows = nrows;
	edges->ncolumns = ncolumns;
	edges->attributes = VCreateAttrList ();
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
 *  \return VEdge
 */

VEdge VAddEdge (VEdges edges, VFloat * edge_fields, int npoints,
		VFloat * points, VBooleanPromoted closed,
		VBooleanPromoted copy)
{
	VEdge edge = VMalloc (sizeof (VEdgeRec));
	size_t fsize, psize, isize;
	int i;
	VPointer p;
	VFloat *pdata;

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
	isize = sizeof (VFloat *) * npoints;	/* Size of points index array. */

	/* If copying data, enough space is allocated to hold everything. */
	if (copy) {
#ifndef __alpha
		fsize = sizeof (VFloat) * edges->nedge_fields;
		psize = sizeof (VFloat) * npoints * edges->npoint_fields;
#else
		/* pointers must be quadword-aligned on a DEC alpha */
#define quadalign(a)	((((a)-1)/8+1)*8)
		fsize = quadalign (sizeof (VFloat) * edges->nedge_fields);
		psize = quadalign (sizeof (VFloat) * npoints *
				   edges->npoint_fields);
#endif
		p = VMalloc (fsize + psize + isize);
		edge->free = p;
		edge->edge_fields = (VFloat *) p;
		if (fsize > 0)
			memcpy (p, edge_fields, fsize);
		pdata = (VFloat *) ((char *)p + fsize);
		memcpy (pdata, points, psize);
		edge->point_index = (VFloat **) ((char *)p + fsize + psize);
	} else {
		p = VMalloc (isize);
		edge->free = p;
		edge->edge_fields = edge_fields;
		pdata = points;
		edge->point_index = (VFloat **) p;
	}

	/* Initialize index array into set of points. */
	for (i = 0; i < npoints; i++)
		edge->point_index[i] = pdata + i * edges->npoint_fields;

	return edge;
}


/*! \brief Copy a VEdges object.
 *
 *  \param  src
 *  \return VEdges
 */

VEdges VCopyEdges (VEdges src)
{
	VEdges result;
	VEdge e;

	result = VCreateEdges (src->nrows, src->ncolumns, src->nedge_fields,
			       src->npoint_fields);
	for (e = src->first; e != NULL; e = e->next)
		VAddEdge (result, e->edge_fields, e->npoints,
			  e->point_index[0], e->closed, TRUE);
	if (VEdgesAttrList (result))
		VDestroyAttrList (VEdgesAttrList (result));
	if (VEdgesAttrList (src))
		VEdgesAttrList (result) =
			VCopyAttrList (VEdgesAttrList (src));
	return result;
}


/*! \brief Frees memory occupied by set of edges.
 *
 *  \param edges
 */

void VDestroyEdges (VEdges edges)
{
	VEdge edge, next_edge;

	for (edge = edges->first; edge; edge = next_edge) {
		next_edge = edge->next;
		if (edge->free)
			VFree (edge->free);
	}
	if (edges->free)
		VFree (edges->free);
	VDestroyAttrList (edges->attributes);
	VFree (edges);
}


/*! \brief Read a Vista data file, extract the edge sets from it, 
 *         and return a list of them.
 *  
 *  \param  file
 *  \param  attributes
 *  \param  edge_sets
 */

int VReadEdges (FILE * file, VAttrList * attributes, VEdges ** edge_sets)
{
	return VReadObjects (file, VEdgesRepn, attributes,
			     (VPointer **) edge_sets);
}


/*! \brief Write a list of edge sets to a Vista data file.
 *
 *  \param  file
 *  \param  attributes
 *  \param  nedge_sets
 *  \param  edge_sets
 *  \return VBoolean
 */

VBoolean VWriteEdges (FILE * file, VAttrList attributes, int nedge_sets,
		      VEdges edge_sets[])
{
	return VWriteObjects (file, VEdgesRepn, attributes, nedge_sets,
			      (VPointer *) edge_sets);
}
