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

/* $Id: graphType.c 51 2004-02-26 12:53:22Z jaenicke $ */

/*! \file graphType.c
 *  \brief methods for the graph (VGraph) type
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"
#include <assert.h>

/*
 *  Table of methods.
 */

/* Later in this file: */
static VDecodeMethod VGraphDecodeMethod;
static VEncodeAttrMethod VGraphEncodeAttrMethod;
static VEncodeDataMethod VGraphEncodeDataMethod;

/* Used in Type.c to register this type: */
VTypeMethods VGraphMethods = {
	(VCopyMethod *) VCopyGraph,	/* copy a VGraph */
	(VDestroyMethod *) VDestroyGraph,	/* destroy a VGraph */
	VGraphDecodeMethod,	/* decode a VGraph's value */
	VGraphEncodeAttrMethod,	/* encode a VGraph's attr list */
	VGraphEncodeDataMethod	/* encode a VGraph's binary data */
};

/*! \brief Convert an attribute list plus binary data to a VGraph object.
 *
 *  The "decode" method registered for the "Graph" type.
 *  
 *  \param  name
 *  \param  b
 *  \return VPointer
 */

static VPointer VGraphDecodeMethod (VStringConst name, VBundle b)
{
	VGraph graph;
	VLong size, nfields, node_repn, useWeights;
	VAttrList list;
	VLong idx, nadj;
	int length;
	size_t len;
	VNode n;
	VPointer p, ptr;
	VAdjacency adj;

#define Extract(name, dict, locn, required)	\
	VExtractAttr (b->list, name, dict, VLongRepn, & locn, required)

	/* Extract the required attribute values for Graph. */
	if (!Extract (VRepnAttr, VNumericRepnDict, node_repn, TRUE) ||
	    !Extract (VNNodeFieldsAttr, NULL, nfields, TRUE) ||
	    !Extract (VNNodeWeightsAttr, NULL, useWeights, TRUE))
		return NULL;
	/* Look for size attribute, if not present, look for nnodes (for backward compatibility */
	if (Extract (VNGraphSizeAttr, NULL, size, TRUE) == FALSE &&
	    Extract (VNGraphNodesAttr, NULL, size, TRUE) == FALSE)
		return NULL;
	if (size <= 0 || nfields <= 0) {
		VWarning ("VGraphReadDataMethod: Bad Graph file attributes");
		return NULL;
	}

	/* Create the Graph data structure. */
	graph = VCreateGraph ((int)size, (int)nfields,
			      (VRepnKind) node_repn, (int)useWeights);
	if (!graph)
		return NULL;

	/* Give it whatever attributes remain: */
	list = VGraphAttrList (graph);
	VGraphAttrList (graph) = b->list;
	b->list = list;

	length = b->length;
	if (length == 0)
		return graph;
	p = b->data;

#define unpack(repn, cnt, dest) \
    ptr = dest; \
    if (VUnpackData(repn, cnt, p, VMsbFirst, & len, & ptr, 0) == 0) return 0; \
    p = (char *) p + len; length -= len; len = length; \
    if (length < 0) goto Fail;
	len = length;

	while (length > 0) {

		/* Get the index : */
		unpack (VLongRepn, 1, &idx);
		graph->table[idx - 1] = n = VCalloc (1, VNodeSize (graph));
		if (idx > graph->lastUsed)
			graph->lastUsed = idx;
		graph->nnodes++;

		/* Get the number of adjacencies : */
		unpack (VLongRepn, 1, &nadj);

		/* Unpack the adjacencies : */
		while (nadj--) {
			adj = VMalloc (sizeof (VAdjRec));
			unpack (VLongRepn, 1, &adj->id);
			if (graph->useWeights) {
				unpack (VFloatRepn, 1, &adj->weight);
			} else
				adj->weight = 0.0;
			adj->next = n->base.head;
			n->base.head = adj;
		};

		/* Unpack the node itself: */
		if (graph->useWeights) {
			unpack (VFloatRepn, 1, &(n->base.weight));
		} else
			n->base.weight = 0.0;
		unpack (graph->node_repn, graph->nfields, n->data);
	}
	return graph;

      Fail:
	VWarning ("VGraphDecodeMethod: %s graph has wrong data length", name);
	VDestroyGraph (graph);
	return NULL;
#undef Extract
}


/*! \brief Encode an attribute list value for a VGraph object.
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  
 *  \param  value
 *  \param  lengthp
 *  \return VAttrList
 */

static VAttrList VGraphEncodeAttrMethod (VPointer value, size_t * lengthp)
{
	VGraph graph = value;
	VAttrList list;
	size_t len, nadj;
	int i, slong, sfloat, spriv, nnodes;
	VNode n;
	VAdjacency adj;

	/* Compute the file space needed for the Graph's binary data: */
	len = 0;
	slong = VRepnPrecision (VLongRepn) / 8;
	sfloat = VRepnPrecision (VFloatRepn) / 8;
	spriv = graph->nfields * VRepnPrecision (graph->node_repn) / 8;
	nnodes = 0;
	for (i = 1; i <= graph->size; i++) {
		n = VGraphGetNode (graph, i);
		if (n == 0)
			continue;
		nnodes++;

		/* Count the number of adjacencies : */
		for (adj = n->base.head, nadj = 0; adj; adj = adj->next)
			nadj++;

		/* each node contains:
		 * an index and the number of adjacencies
		 * the private data area
		 * the list of adjacencies
		 * optionally reserve space for weights
		 */
		len += 2 * slong + nadj * slong + spriv;
		if (graph->useWeights)
			len += (nadj + 1) * sfloat;
	};
	*lengthp = len;
	assert(nnodes == graph->nnodes);	/* for debugging */
	graph->nnodes = nnodes;			/* just to be safe for now... */

	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = VGraphAttrList (graph)) == NULL)
		list = VGraphAttrList (graph) = VCreateAttrList ();
	VPrependAttr (list, VRepnAttr, VNumericRepnDict, VLongRepn,
		      (VLong) graph->node_repn);
	VPrependAttr (list, VNNodeFieldsAttr, NULL, VLongRepn,
		      (VLong) graph->nfields);
	VPrependAttr (list, VNGraphSizeAttr, NULL, VLongRepn,
		      (VLong) graph->size);
	VPrependAttr (list, VNNodeWeightsAttr, NULL, VLongRepn,
		      (VLong) graph->useWeights);

	return list;
}


/*! \brief Encode the edge and point fields for a VGraph object.
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  
 *  \param  value
 *  \param  list
 *  \param  length
 *  \param  free_itp
 *  \return VPointer
 */

static VPointer VGraphEncodeDataMethod (VPointer value, VAttrList list,
					size_t length, VBoolean * free_itp)
{
	VGraph graph = value;
	VAttrListPosn posn;
	VNode n;
	size_t len;
	VPointer p, ptr;
	VAdjacency adj;
	int i, nadj;

#define pack(repn, cnt, dest) \
    if (! VPackData (repn, cnt, dest, VMsbFirst, &len, &p, NULL)) return NULL; \
    p = (char *) p + len; length -= len; len = length;

	/* Remove the attributes prepended by the VGraphEncodeAttrsMethod: */
	for (VFirstAttr (list, &posn);
	     strcmp (VGetAttrName (&posn), VRepnAttr) != 0;
	     VDeleteAttr (&posn));
	VDeleteAttr (&posn);

	/* Allocate a buffer for the encoded data: */
	if (length == 0) {
		*free_itp = FALSE;
		return value;	/* we may return anything != 0 here */
	};

	p = ptr = VMalloc (length);
	len = length;

	/* Pack each node: */
	for (i = 1; i <= graph->size; i++) {

		n = VGraphGetNode (graph, i);
		if (n == 0)
			continue;

		/* Count the number of adjacencies : */
		for (adj = n->base.head, nadj = 0; adj; adj = adj->next)
			nadj++;

		/* Pack the header */
		pack (VLongRepn, 1, &i);
		pack (VLongRepn, 1, &nadj);

		/* Pack the adjacencies : */
		for (adj = n->base.head; adj; adj = adj->next) {
			pack (VLongRepn, 1, &adj->id);
			if (graph->useWeights) {
				pack (VFloatRepn, 1, &adj->weight);
			};
		};

		/* Pack the node itself: */
		if (graph->useWeights) {
			pack (VFloatRepn, 1, &(n->base.weight));
		};
		pack (graph->node_repn, graph->nfields, n->data);
	}

	*free_itp = TRUE;
	return ptr;
}

/*! \brief Allocates memory for a VGraph structure and initializes its fields.
 *    
 *  Initially, this contains an empty node table, so each node must still
 *  be initialized.
 *
 *  \param  size
 *  \param  nfields
 *  \param  repn
 *  \param  useW
 *  \return Returns a pointer to the graph if successful, NULL otherwise.
 */

EXPORT_VISTA VGraph VCreateGraph (int size, int nfields, VRepnKind repn, int useW)
{
	VGraph graph;

	/* Check parameters: */
	if (size < 1 || nfields < 1)
		VWarning ("VCreateGraph: Invalid number of nodes or fields.");

	/* Allocate memory for the VGraph, and the node table: */
	graph = VMalloc (sizeof (VGraphRec));
	if (graph == NULL)
		return NULL;

	graph->table = VCalloc (size, sizeof (VNode));
	if (graph->table == NULL) {
		VFree (graph);
		return NULL;
	};

	/* Initialize the VGraph: */
	graph->nnodes = 0;
	graph->nfields = nfields;
	graph->node_repn = repn;
	graph->attributes = VCreateAttrList ();
	graph->lastUsed = 0;
	graph->size = size;
	graph->useWeights = useW;
	graph->iter = 0;

	return graph;
}

/*! \brief simple deletion: just look at structures of this node
 *
 *  \param graph
 *  \param i
 */

static void VDestroyNodeSimple (VGraph graph, int i)
{
	VAdjacency p, q;
	VNode n;

	n = VGraphGetNode (graph, i);
	if (n == 0)
		return;

	/* destroy adjacency list */
	for (p = n->base.head; p; p = q) {
		q = p->next;
		VFree (p);
	};
	VFree (n);
	VGraphGetNode (graph, i) = 0;
	graph->nnodes--;
	assert(graph->nnodes >= 0);
}

/*! \brief Frees memory occupied by a graph.
 *
 *  \param graph
 */

EXPORT_VISTA void VDestroyGraph (VGraph graph)
{
	int i;

	/* destroy each node */
	for (i = 1; i <= graph->size; i++)
		VDestroyNodeSimple (graph, i);

	/* destroy the table */
	VFree (graph->table);
	graph->table = 0;
	graph->size = 0;	/* again, make it sure */
	VDestroyAttrList (graph->attributes);
	VFree (graph);
}

/*! \brief Copy a node excluding links
 *
 *  \param  graph
 *  \param  src
 *  \return VNode
 */

static VNode VCopyNodeShallow (VGraph graph, VNode src)
{
	VNode dst;

	if (src == 0)
		return 0;
	dst = VCalloc (1, VNodeSize (graph));
	memcpy (dst, src, VNodeSize (graph));
	dst->base.head = 0;
	return dst;
}

/*! \brief Copy a node including links
 *
 *  \param  graph
 *  \param  src
 *  \return VNode
 */

static VNode VCopyNodeDeep (VGraph graph, VNode src)
{
	VNode dst;
	VAdjacency o, n;
	int cnt;

	if (src == 0)
		return 0;

	/* allocate and copy base part */
	dst = VCalloc (1, VNodeSize (graph));
	dst->base.hops = src->base.hops;
	dst->base.visited = src->base.visited;
	dst->base.weight = src->base.weight;
	dst->base.head = 0;

	/* copy all adjacencies */
	for (o = src->base.head; o; o = o->next) {
		n = VMalloc (sizeof (VAdjRec));
		n->id = o->id;
		n->weight = o->weight;
		n->next = dst->base.head;
		dst->base.head = n;
	};

	/* copy private area */
	cnt = (graph->nfields * VRepnPrecision (graph->node_repn)) / 8;
	memcpy (dst->data, src->data, cnt);
	return dst;
}

/*! \brief Copy a VGraph object.
 *
 *  Note that no compaction is performed, since this would require
 *  a recalculation of all indices,
 *
 *  \param  src
 *  \return VGraph
 */

EXPORT_VISTA VGraph VCopyGraph (VGraph src)
{
	VGraph dst;
	int i;

	dst = VCreateGraph (src->size, src->nfields, src->node_repn,
			    src->useWeights);

	/* copy each used node in table */
	for (i = 1; i <= src->size; i++)
		dst->table[i - 1] =
			VCopyNodeDeep (src, VGraphGetNode (src, i));

	dst->nnodes = src->nnodes;
	dst->lastUsed = src->lastUsed;

	if (VGraphAttrList (dst))
		VDestroyAttrList (VGraphAttrList (dst));
	if (VGraphAttrList (src))
		VGraphAttrList (dst) = VCopyAttrList (VGraphAttrList (src));
	return dst;
}


/*! \brief Read a Vista data file, extract the graphs from it, and return 
 *         a list of them.
 *
 *  \param  file
 *  \param  attrs
 *  \param  graphs
 *  \return int
 */

EXPORT_VISTA int VReadGraphs (FILE * file, VAttrList * attrs, VGraph ** graphs)
{
	return VReadObjects (file, VGraphRepn, attrs, (VPointer **) graphs);
}


/*! \brief Write a list of graphs to a Vista data file.
 * 
 *  \param  file
 *  \param  attrs
 *  \param  n
 *  \param  graphs
 *  \return VBoolean
 */

EXPORT_VISTA VBoolean VWriteGraphs (FILE * file, VAttrList attrs, int n, VGraph graphs[])
{
	return VWriteObjects (file, VGraphRepn, attrs, n,
			      (VPointer *) graphs);
}

/*! \brief Find a node in a Vista graph structure.
 *  
 *  \param  graph
 *  \param  node
 *  \return Return reference to this node.
 */

EXPORT_VISTA int VGraphLookupNode (VGraph graph, VNode node)
{
	int n = (graph->nfields * VRepnPrecision (graph->node_repn)) / 8;
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		if (VGraphNodeIsFree (graph, i))
			continue;
		if (memcmp (node->data, VGraphGetNode (graph, i)->data, n) == 0)
			return i;
	};
	return 0;
}

static int growGraph (VGraph graph)
     /* note that we grow just a pointer table */
{
	int newsize = (graph->size * 3) / 2;
	VNode *t = VCalloc (newsize, sizeof (VNode));
	if (t == 0)
		return 0;
	memcpy (t, graph->table, graph->size * sizeof (VNode));
	VFree (graph->table);
	graph->table = t;
	graph->size = newsize;
	return newsize;
}

/*! \brief Add a node to a Vista graph structure.
 *  
 *  \param  graph
 *  \param  node
 *  \return Return reference to this node.
 */

EXPORT_VISTA int VGraphAddNode (VGraph graph, VNode node)
{
	int i = VGraphLookupNode (graph, node);
	if (i) return i;
	if (graph->lastUsed == graph->size)
		if (growGraph (graph) == 0)
			return 0;
	graph->table[graph->lastUsed++] = VCopyNodeShallow (graph, node);
	graph->nnodes++;
	return graph->lastUsed;
}

/*! \brief Add a node to a Vista graph structure at a specific position.
 *
 *  Note that we do not check for duplicates.
 *  Return reference to this node.
 *
 *  \param  graph
 *  \param  node
 *  \param  position
 *  \return int
 */

EXPORT_VISTA int VGraphAddNodeAt (VGraph graph, VNode node, int position)
{
	VDestroyNodeSimple (graph, position);
	VGraphGetNode (graph, position) = VCopyNodeShallow (graph, node);
	graph->nnodes++;
	if (position > graph->lastUsed)
		graph->lastUsed = position;
	return position;
}

/*! \brief Make a link between to nodes.
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 *  \return Return TRUE if successful.
 */

EXPORT_VISTA int VGraphLinkNodes (VGraph graph, int a, int b)
{
	VNode n;
	VAdjacency adj;

	if (VGraphNodeIsFree (graph, a) || VGraphNodeIsFree (graph, b))
		return FALSE;
	n = VGraphGetNode (graph, a);
	adj = VMalloc (sizeof (VAdjRec));
	adj->id = b;
	adj->weight = 0;
	adj->next = n->base.head;
	n->base.head = adj;
	return TRUE;
}

/*! \brief unlinks two nodes.
 *
 *  Return TRUE if successful.
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 *  \return int
 */

EXPORT_VISTA int VGraphUnlinkNodes (VGraph graph, int a, int b)
{
	VNode n;
	VAdjacency adj, prev;

	if (VGraphNodeIsFree (graph, a) || VGraphNodeIsFree (graph, b))
		return FALSE;
	n = VGraphGetNode (graph, a);
	prev = 0;
	for (adj = n->base.head; adj; adj = adj->next) {
		if (adj->id == (unsigned int)b) {
			if (prev)
				prev->next = adj->next;
			else
				n->base.head = adj->next;
			VFree (adj);
			return TRUE;
		};
		prev = adj;
	};
	return FALSE;
}

static VNode seqNode (VGraph graph, int i)
{
	while (i <= graph->lastUsed) {
		VNode n = VGraphGetNode (graph, i);
		if (n) {
			graph->iter = i;
			return n;
		};
		i++;
	};
	return 0;
}

/*! \brief
 *
 *  \param  graph
 *  \return VPointer
 */

EXPORT_VISTA VPointer VGraphFirstNode (VGraph graph)
{
	return graph ? seqNode (graph, 1) : 0;
}

/*! \brief
 *
 *  \param  graph
 *  \return VPointer
 */

EXPORT_VISTA VPointer VGraphNextNode (VGraph graph)
{
	return graph ? seqNode (graph, graph->iter + 1) : 0;
}

/*! \brief
 *
 *  \param  graph
 */

EXPORT_VISTA void VGraphClearVisit (VGraph graph)
{
	VNode n;
	int i;

	if (graph == 0)
		return;
	for (i = 1; i <= graph->lastUsed; i++) {
		if (VGraphNodeIsFree (graph, i))
			continue;
		n = VGraphGetNode (graph, i);
		VNodeClearVisit (&n->base);
	};
}

/*! \brief Grow private data area of each node to newfields.
 *  
 *  \param  graph
 *  \param  newfields
 *  \return Return TRUE if successful.
 */

EXPORT_VISTA int VGraphResizeFields (VGraph graph, int newfields)
{
	VNode o, n;
	int i;
	int nsize =
		sizeof (VNodeBaseRec) +
		(newfields * VRepnPrecision (graph->node_repn)) / 8;
	int osize = VNodeSize (graph);
	if (newfields <= graph->nfields)
		return TRUE;
	for (i = 1; i <= graph->lastUsed; i++) {
		if (VGraphNodeIsFree (graph, i))
			continue;
		o = VGraphGetNode (graph, i);
		n = VCalloc (1, nsize);
		memcpy (n, o, osize);
		VGraphGetNode (graph, i) = n;
		VFree (o);
	};
	graph->nfields = newfields;
	return TRUE;
}

static int firstUnvisitedNode (VGraph graph)
{
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		VNode n = VGraphGetNode (graph, i);
		if (n && n->base.hops == 0)
			return i;
	};
	return 0;
}

/*! \brief Visits all node in a graph connected to node i
 *
 *  \param  graph
 *  \param  i
 *  \return int
 */

EXPORT_VISTA int VGraphVisitNodesFrom (VGraph graph, int i)
{
	VAdjacency adj;
	VNode n, p;
	int cycles = 0;

	if (graph == 0 || (n = VGraphGetNode (graph, i)) == 0)
		return 0;
	if (n->base.hops > 0)
		return 1;
	n->base.hops++;

	for (adj = n->base.head; adj; adj = adj->next) {
		p = VGraphGetNode (graph, adj->id);
		if (p && p->base.hops > 0)
			continue;
		cycles += VGraphVisitNodesFrom (graph, adj->id);
	};
	return cycles;
}

/*! \brief Clears the hops field in a graph
 *  
 *  \param  graph
 */

EXPORT_VISTA void VGraphClearHops (VGraph graph)
{
	int i;

	if (graph == 0)
		return;
	for (i = 1; i <= graph->lastUsed; i++) {
		VNode n = VGraphGetNode (graph, i);
		if (n)
			n->base.hops = 0;
	};
}

/*! \brief
 *
 *  \param  graph
 *  \return Returns number of cycles in a graph
 */

EXPORT_VISTA int VGraphNCycles (VGraph graph)
{
	int cycles = 0;

	VGraphClearHops (graph);
	while (1) {
		/* get the first unvisited node */
		int n = firstUnvisitedNode (graph);
		if (n == 0)
			return cycles;
		cycles += VGraphVisitNodesFrom (graph, n);
	};
}

/*! \brief Copies nodes from a graph in which the visited flag is set \
 *         and places them into a new graph
 *
 *  \param  src
 *  \return VGraph
 */

EXPORT_VISTA VGraph VGraphExtractNodes (VGraph src)
{
	VGraph dst;
	VAdjacency adj;
	VNode n;
	int i, j;

	/* create a destination graph much like src */
	dst = VCreateGraph (src->size, src->nfields, src->node_repn,
			    src->useWeights);

	/* copy selected nodes from src */
	for (i = j = 1; i <= src->lastUsed; i++) {
		n = VGraphGetNode (src, i);
		if (n && n->base.hops)
			dst->table[j++] = VCopyNodeShallow (src, n);
	};

	/* set number of nodes used */
	dst->nnodes = j-1;
	dst->lastUsed = j;

	/* now link nodes in new graph */
	for (i = 1; i <= dst->lastUsed; i++) {
		n = VGraphGetNode (dst, i);
		if (n == 0)
			continue;

		j = VGraphLookupNode (src, n);
		if (j == 0)
			continue;
		n = VGraphGetNode (src, j);
		for (adj = n->base.head; adj; adj = adj->next) {
			n = VGraphGetNode (src, adj->id);
			j = VGraphLookupNode (dst, n);
			if (j)
				VGraphLinkNodes (dst, i, j);
		};
	};

	if (VGraphAttrList (dst))
		VDestroyAttrList (VGraphAttrList (dst));
	if (VGraphAttrList (src))
		VGraphAttrList (dst) = VCopyAttrList (VGraphAttrList (src));
	return dst;
}

/*! \brief Visits all node in a graph connected to node i and toggles the hops
 *  field note that the visit field must have been cleared before
 *
 *  \param  graph
 *  \param  i
 */

EXPORT_VISTA void VGraphToggleNodesFrom (VGraph graph, int i)
{
	VAdjacency adj;
	VNode n, p;

	/* find a valid starting point */
	if (graph == 0 || (n = VGraphGetNode (graph, i)) == 0)
		return;

	/* mark this node and toggle the hops field */
	if (n->base.visited == 1)
		return;
	n->base.visited = 1;
	n->base.hops = n->base.hops ? 0 : 1;

	/* now look at the neighbors */
	for (adj = n->base.head; adj; adj = adj->next) {
		p = VGraphGetNode (graph, adj->id);
		if (p && p->base.visited)
			continue;
		VGraphToggleNodesFrom (graph, adj->id);
	};
}

/*! \brief complex deletion: look at all connected structures of this node
 *
 *  \param  graph
 *  \param  i
 */

EXPORT_VISTA void VDestroyNode (VGraph graph, int i)
{
	VAdjacency p, q;
	VNode n;

	n = VGraphGetNode (graph, i);
	if (n == 0)
		return;

	/* destroy adjacency list */
	for (p = n->base.head; p; p = q) {
		/* remove connection from other node to this node */
		VGraphUnlinkNodes (graph, p->id, i);
		q = p->next;
		VFree (p);
	};
	VFree (n);
	VGraphGetNode (graph, i) = 0;
	graph->nnodes--;
	assert(graph->nnodes >= 0);
}

static void VGraphRemoveNodes (VGraph graph)
     /* remove all nodes in which the visited flag is set */
{
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		VNode n = VGraphGetNode (graph, i);
		if (n && n->base.visited)
			VDestroyNode (graph, i);
	};
}

/*! \brief Destroys nodes from a graph in which the hops field is set
 *
 *  \param graph
 *  \param i
 */

EXPORT_VISTA void VGraphDestroyNodesFrom (VGraph graph, int i)
{
	VGraphToggleNodesFrom (graph, i);
	VGraphRemoveNodes (graph);
}

/*! \brief make bidrectional link between nodes a and b
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 */

EXPORT_VISTA void VGraphLinkNodesBi (VGraph graph, VLong a, VLong b)
{
	VNode n;
	VAdjacency adj;

	if (a == 0 || b == 0 || a == b)
		return;
	n = VGraphGetNode (graph, a);
	if (n == NULL)
		return;
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == (unsigned long)b)
			goto rev;
	VGraphLinkNodes (graph, a, b);
      rev:
	n = VGraphGetNode (graph, b);
	if (n == NULL)
		return;
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == (unsigned long)a)
			return;
	VGraphLinkNodes (graph, b, a);
}

/*! \brief remove bidrectional link between nodes a and b
 * 
 *  \param  graph
 *  \param  a
 *  \param  b
 */

EXPORT_VISTA void VGraphUnlinkNodesBi (VGraph graph, VLong a, VLong b)
{
	VGraphUnlinkNodes (graph, a, b);
	VGraphUnlinkNodes (graph, b, a);
}

/*! \brief
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 *  \return VBoolean
 */

EXPORT_VISTA VBoolean VGraphHasLink (VGraph graph, int a, int b)
{
	VNode n;
	VAdjacency adj; 
	
	if (a == 0 || b == 0 || a == b)
		return FALSE;
	n = VGraphGetNode (graph, a);
	
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == b)
			return TRUE;
	return FALSE;
}
