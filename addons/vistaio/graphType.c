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
#include <assert.h>

/*
 *  Table of methods.
 */

/* Later in this file: */
static VistaIODecodeMethod VistaIOGraphDecodeMethod;
static VistaIOEncodeAttrMethod VistaIOGraphEncodeAttrMethod;
static VistaIOEncodeDataMethod VistaIOGraphEncodeDataMethod;

/* Used in Type.c to register this type: */
VistaIOTypeMethods VistaIOGraphMethods = {
	(VistaIOCopyMethod *) VistaIOCopyGraph,	/* copy a VistaIOGraph */
	(VistaIODestroyMethod *) VistaIODestroyGraph,	/* destroy a VistaIOGraph */
	VistaIOGraphDecodeMethod,	/* decode a VistaIOGraph's value */
	VistaIOGraphEncodeAttrMethod,	/* encode a VistaIOGraph's attr list */
	VistaIOGraphEncodeDataMethod	/* encode a VistaIOGraph's binary data */
};

/*! \brief Convert an attribute list plus binary data to a VistaIOGraph object.
 *
 *  The "decode" method registered for the "Graph" type.
 *  
 *  \param  name
 *  \param  b
 *  \return VistaIOPointer
 */

static VistaIOPointer VistaIOGraphDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VistaIOGraph graph;
	VistaIOLong size, nfields, node_repn, useWeights;
	VistaIOAttrList list;
	VistaIOLong idx, nadj;
	int length;
	size_t len;
	VistaIONode n;
	VistaIOPointer p, ptr;
	VistaIOAdjacency adj;

#define Extract(name, dict, locn, required)	\
	VistaIOExtractAttr (b->list, name, dict, VistaIOLongRepn, & locn, required)

	/* Extract the required attribute values for Graph. */
	if (!Extract (VistaIORepnAttr, VistaIONumericRepnDict, node_repn, TRUE) ||
	    !Extract (VNNodeFieldsAttr, NULL, nfields, TRUE) ||
	    !Extract (VNNodeWeightsAttr, NULL, useWeights, TRUE))
		return NULL;
	/* Look for size attribute, if not present, look for nnodes (for backward compatibility */
	if (Extract (VNGraphSizeAttr, NULL, size, TRUE) == FALSE &&
	    Extract (VNGraphNodesAttr, NULL, size, TRUE) == FALSE)
		return NULL;
	if (size <= 0 || nfields <= 0) {
		VistaIOWarning ("VistaIOGraphReadDataMethod: Bad Graph file attributes");
		return NULL;
	}

	/* Create the Graph data structure. */
	graph = VistaIOCreateGraph ((int)size, (int)nfields,
			      (VistaIORepnKind) node_repn, (int)useWeights);
	if (!graph)
		return NULL;

	/* Give it whatever attributes remain: */
	list = VistaIOGraphAttrList (graph);
	VistaIOGraphAttrList (graph) = b->list;
	b->list = list;

	length = b->length;
	if (length == 0)
		return graph;
	p = b->data;

#define unpack(repn, cnt, dest) \
    ptr = dest; \
    if (VistaIOUnpackData(repn, cnt, p, VistaIOMsbFirst, & len, & ptr, 0) == 0) return 0; \
    p = (char *) p + len; length -= len; len = length; \
    if (length < 0) goto Fail;
	len = length;

	while (length > 0) {

		/* Get the index : */
		unpack (VistaIOLongRepn, 1, &idx);
		graph->table[idx - 1] = n = VistaIOCalloc (1, VistaIONodeSize (graph));
		if (idx > graph->lastUsed)
			graph->lastUsed = idx;
		graph->nnodes++;

		/* Get the number of adjacencies : */
		unpack (VistaIOLongRepn, 1, &nadj);

		/* Unpack the adjacencies : */
		while (nadj--) {
			adj = VistaIOMalloc (sizeof (VistaIOAdjRec));
			unpack (VistaIOLongRepn, 1, &adj->id);
			if (graph->useWeights) {
				unpack (VistaIOFloatRepn, 1, &adj->weight);
			} else
				adj->weight = 0.0;
			adj->next = n->base.head;
			n->base.head = adj;
		};

		/* Unpack the node itself: */
		if (graph->useWeights) {
			unpack (VistaIOFloatRepn, 1, &(n->base.weight));
		} else
			n->base.weight = 0.0;
		unpack (graph->node_repn, graph->nfields, n->data);
	}
	return graph;

      Fail:
	VistaIOWarning ("VistaIOGraphDecodeMethod: %s graph has wrong data length", name);
	VistaIODestroyGraph (graph);
	return NULL;
#undef Extract
}


/*! \brief Encode an attribute list value for a VistaIOGraph object.
 *
 *  The "encode_attrs" method registered for the "Graph" type.
 *  
 *  \param  value
 *  \param  lengthp
 *  \return VistaIOAttrList
 */

static VistaIOAttrList VistaIOGraphEncodeAttrMethod (VistaIOPointer value, size_t * lengthp)
{
	VistaIOGraph graph = value;
	VistaIOAttrList list;
	size_t len, nadj;
	int i, slong, sfloat, spriv, nnodes;
	VistaIONode n;
	VistaIOAdjacency adj;

	/* Compute the file space needed for the Graph's binary data: */
	len = 0;
	slong = VistaIORepnPrecision (VistaIOLongRepn) / 8;
	sfloat = VistaIORepnPrecision (VistaIOFloatRepn) / 8;
	spriv = graph->nfields * VistaIORepnPrecision (graph->node_repn) / 8;
	nnodes = 0;
	for (i = 1; i <= graph->size; i++) {
		n = VistaIOGraphGetNode (graph, i);
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
	if ((list = VistaIOGraphAttrList (graph)) == NULL)
		list = VistaIOGraphAttrList (graph) = VistaIOCreateAttrList ();
	VistaIOPrependAttr (list, VistaIORepnAttr, VistaIONumericRepnDict, VistaIOLongRepn,
		      (VistaIOLong) graph->node_repn);
	VistaIOPrependAttr (list, VNNodeFieldsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) graph->nfields);
	VistaIOPrependAttr (list, VNGraphSizeAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) graph->size);
	VistaIOPrependAttr (list, VNNodeWeightsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) graph->useWeights);

	return list;
}


/*! \brief Encode the edge and point fields for a VistaIOGraph object.
 *
 *  The "encode_data" method registered for the "Graph" type.
 *  
 *  \param  value
 *  \param  list
 *  \param  length
 *  \param  free_itp
 *  \return VistaIOPointer
 */

static VistaIOPointer VistaIOGraphEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean * free_itp)
{
	VistaIOGraph graph = value;
	VistaIOAttrListPosn posn;
	VistaIONode n;
	size_t len;
	VistaIOPointer p, ptr;
	VistaIOAdjacency adj;
	int i, nadj;

#define pack(repn, cnt, dest) \
    if (! VistaIOPackData (repn, cnt, dest, VistaIOMsbFirst, &len, &p, NULL)) return NULL; \
    p = (char *) p + len; length -= len; len = length;

	/* Remove the attributes prepended by the VistaIOGraphEncodeAttrsMethod: */
	for (VistaIOFirstAttr (list, &posn);
	     strcmp (VistaIOGetAttrName (&posn), VistaIORepnAttr) != 0;
	     VistaIODeleteAttr (&posn));
	VistaIODeleteAttr (&posn);

	/* Allocate a buffer for the encoded data: */
	if (length == 0) {
		*free_itp = FALSE;
		return value;	/* we may return anything != 0 here */
	};

	p = ptr = VistaIOMalloc (length);
	len = length;

	/* Pack each node: */
	for (i = 1; i <= graph->size; i++) {

		n = VistaIOGraphGetNode (graph, i);
		if (n == 0)
			continue;

		/* Count the number of adjacencies : */
		for (adj = n->base.head, nadj = 0; adj; adj = adj->next)
			nadj++;

		/* Pack the header */
		pack (VistaIOLongRepn, 1, &i);
		pack (VistaIOLongRepn, 1, &nadj);

		/* Pack the adjacencies : */
		for (adj = n->base.head; adj; adj = adj->next) {
			pack (VistaIOLongRepn, 1, &adj->id);
			if (graph->useWeights) {
				pack (VistaIOFloatRepn, 1, &adj->weight);
			};
		};

		/* Pack the node itself: */
		if (graph->useWeights) {
			pack (VistaIOFloatRepn, 1, &(n->base.weight));
		};
		pack (graph->node_repn, graph->nfields, n->data);
	}

	*free_itp = TRUE;
	return ptr;
}

/*! \brief Allocates memory for a VistaIOGraph structure and initializes its fields.
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

EXPORT_VISTA VistaIOGraph VistaIOCreateGraph (int size, int nfields, VistaIORepnKind repn, int useW)
{
	VistaIOGraph graph;

	/* Check parameters: */
	if (size < 1 || nfields < 1)
		VistaIOWarning ("VistaIOCreateGraph: Invalid number of nodes or fields.");

	/* Allocate memory for the VistaIOGraph, and the node table: */
	graph = VistaIOMalloc (sizeof (VistaIOGraphRec));
	if (graph == NULL)
		return NULL;

	graph->table = VistaIOCalloc (size, sizeof (VistaIONode));
	if (graph->table == NULL) {
		VistaIOFree (graph);
		return NULL;
	};

	/* Initialize the VistaIOGraph: */
	graph->nnodes = 0;
	graph->nfields = nfields;
	graph->node_repn = repn;
	graph->attributes = VistaIOCreateAttrList ();
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

static void VistaIODestroyNodeSimple (VistaIOGraph graph, int i)
{
	VistaIOAdjacency p, q;
	VistaIONode n;

	n = VistaIOGraphGetNode (graph, i);
	if (n == 0)
		return;

	/* destroy adjacency list */
	for (p = n->base.head; p; p = q) {
		q = p->next;
		VistaIOFree (p);
	};
	VistaIOFree (n);
	VistaIOGraphGetNode (graph, i) = 0;
	graph->nnodes--;
	assert(graph->nnodes >= 0);
}

/*! \brief Frees memory occupied by a graph.
 *
 *  \param graph
 */

EXPORT_VISTA void VistaIODestroyGraph (VistaIOGraph graph)
{
	int i;

	/* destroy each node */
	for (i = 1; i <= graph->size; i++)
		VistaIODestroyNodeSimple (graph, i);

	/* destroy the table */
	VistaIOFree (graph->table);
	graph->table = 0;
	graph->size = 0;	/* again, make it sure */
	VistaIODestroyAttrList (graph->attributes);
	VistaIOFree (graph);
}

/*! \brief Copy a node excluding links
 *
 *  \param  graph
 *  \param  src
 *  \return VistaIONode
 */

static VistaIONode VistaIOCopyNodeShallow (VistaIOGraph graph, VistaIONode src)
{
	VistaIONode dst;

	if (src == 0)
		return 0;
	dst = VistaIOCalloc (1, VistaIONodeSize (graph));
	memcpy (dst, src, VistaIONodeSize (graph));
	dst->base.head = 0;
	return dst;
}

/*! \brief Copy a node including links
 *
 *  \param  graph
 *  \param  src
 *  \return VistaIONode
 */

static VistaIONode VistaIOCopyNodeDeep (VistaIOGraph graph, VistaIONode src)
{
	VistaIONode dst;
	VistaIOAdjacency o, n;
	int cnt;

	if (src == 0)
		return 0;

	/* allocate and copy base part */
	dst = VistaIOCalloc (1, VistaIONodeSize (graph));
	dst->base.hops = src->base.hops;
	dst->base.visited = src->base.visited;
	dst->base.weight = src->base.weight;
	dst->base.head = 0;

	/* copy all adjacencies */
	for (o = src->base.head; o; o = o->next) {
		n = VistaIOMalloc (sizeof (VistaIOAdjRec));
		n->id = o->id;
		n->weight = o->weight;
		n->next = dst->base.head;
		dst->base.head = n;
	};

	/* copy private area */
	cnt = (graph->nfields * VistaIORepnPrecision (graph->node_repn)) / 8;
	memcpy (dst->data, src->data, cnt);
	return dst;
}

/*! \brief Copy a VistaIOGraph object.
 *
 *  Note that no compaction is performed, since this would require
 *  a recalculation of all indices,
 *
 *  \param  src
 *  \return VistaIOGraph
 */

EXPORT_VISTA VistaIOGraph VistaIOCopyGraph (VistaIOGraph src)
{
	VistaIOGraph dst;
	int i;

	dst = VistaIOCreateGraph (src->size, src->nfields, src->node_repn,
			    src->useWeights);

	/* copy each used node in table */
	for (i = 1; i <= src->size; i++)
		dst->table[i - 1] =
			VistaIOCopyNodeDeep (src, VistaIOGraphGetNode (src, i));

	dst->nnodes = src->nnodes;
	dst->lastUsed = src->lastUsed;

	if (VistaIOGraphAttrList (dst))
		VistaIODestroyAttrList (VistaIOGraphAttrList (dst));
	if (VistaIOGraphAttrList (src))
		VistaIOGraphAttrList (dst) = VistaIOCopyAttrList (VistaIOGraphAttrList (src));
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

EXPORT_VISTA int VistaIOReadGraphs (FILE * file, VistaIOAttrList * attrs, VistaIOGraph ** graphs)
{
	return VistaIOReadObjects (file, VistaIOGraphRepn, attrs, (VistaIOPointer **) graphs);
}


/*! \brief Write a list of graphs to a Vista data file.
 * 
 *  \param  file
 *  \param  attrs
 *  \param  n
 *  \param  graphs
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIOWriteGraphs (FILE * file, VistaIOAttrList attrs, int n, VistaIOGraph graphs[])
{
	return VistaIOWriteObjects (file, VistaIOGraphRepn, attrs, n,
			      (VistaIOPointer *) graphs);
}

/*! \brief Find a node in a Vista graph structure.
 *  
 *  \param  graph
 *  \param  node
 *  \return Return reference to this node.
 */

EXPORT_VISTA int VistaIOGraphLookupNode (VistaIOGraph graph, VistaIONode node)
{
	int n = (graph->nfields * VistaIORepnPrecision (graph->node_repn)) / 8;
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		if (VistaIOGraphNodeIsFree (graph, i))
			continue;
		if (memcmp (node->data, VistaIOGraphGetNode (graph, i)->data, n) == 0)
			return i;
	};
	return 0;
}

static int growGraph (VistaIOGraph graph)
     /* note that we grow just a pointer table */
{
	int newsize = (graph->size * 3) / 2;
	VistaIONode *t = VistaIOCalloc (newsize, sizeof (VistaIONode));
	if (t == 0)
		return 0;
	memcpy (t, graph->table, graph->size * sizeof (VistaIONode));
	VistaIOFree (graph->table);
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

EXPORT_VISTA int VistaIOGraphAddNode (VistaIOGraph graph, VistaIONode node)
{
	int i = VistaIOGraphLookupNode (graph, node);
	if (i) return i;
	if (graph->lastUsed == graph->size)
		if (growGraph (graph) == 0)
			return 0;
	graph->table[graph->lastUsed++] = VistaIOCopyNodeShallow (graph, node);
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

EXPORT_VISTA int VistaIOGraphAddNodeAt (VistaIOGraph graph, VistaIONode node, int position)
{
	VistaIODestroyNodeSimple (graph, position);
	VistaIOGraphGetNode (graph, position) = VistaIOCopyNodeShallow (graph, node);
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

EXPORT_VISTA int VistaIOGraphLinkNodes (VistaIOGraph graph, int a, int b)
{
	VistaIONode n;
	VistaIOAdjacency adj;

	if (VistaIOGraphNodeIsFree (graph, a) || VistaIOGraphNodeIsFree (graph, b))
		return FALSE;
	n = VistaIOGraphGetNode (graph, a);
	adj = VistaIOMalloc (sizeof (VistaIOAdjRec));
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

EXPORT_VISTA int VistaIOGraphUnlinkNodes (VistaIOGraph graph, int a, int b)
{
	VistaIONode n;
	VistaIOAdjacency adj, prev;

	if (VistaIOGraphNodeIsFree (graph, a) || VistaIOGraphNodeIsFree (graph, b))
		return FALSE;
	n = VistaIOGraphGetNode (graph, a);
	prev = 0;
	for (adj = n->base.head; adj; adj = adj->next) {
		if (adj->id == (unsigned int)b) {
			if (prev)
				prev->next = adj->next;
			else
				n->base.head = adj->next;
			VistaIOFree (adj);
			return TRUE;
		};
		prev = adj;
	};
	return FALSE;
}

static VistaIONode seqNode (VistaIOGraph graph, int i)
{
	while (i <= graph->lastUsed) {
		VistaIONode n = VistaIOGraphGetNode (graph, i);
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
 *  \return VistaIOPointer
 */

EXPORT_VISTA VistaIOPointer VistaIOGraphFirstNode (VistaIOGraph graph)
{
	return graph ? seqNode (graph, 1) : 0;
}

/*! \brief
 *
 *  \param  graph
 *  \return VistaIOPointer
 */

EXPORT_VISTA VistaIOPointer VistaIOGraphNextNode (VistaIOGraph graph)
{
	return graph ? seqNode (graph, graph->iter + 1) : 0;
}

/*! \brief
 *
 *  \param  graph
 */

EXPORT_VISTA void VistaIOGraphClearVisit (VistaIOGraph graph)
{
	VistaIONode n;
	int i;

	if (graph == 0)
		return;
	for (i = 1; i <= graph->lastUsed; i++) {
		if (VistaIOGraphNodeIsFree (graph, i))
			continue;
		n = VistaIOGraphGetNode (graph, i);
		VistaIONodeClearVisit (&n->base);
	};
}

/*! \brief Grow private data area of each node to newfields.
 *  
 *  \param  graph
 *  \param  newfields
 *  \return Return TRUE if successful.
 */

EXPORT_VISTA int VistaIOGraphResizeFields (VistaIOGraph graph, int newfields)
{
	VistaIONode o, n;
	int i;
	int nsize =
		sizeof (VistaIONodeBaseRec) +
		(newfields * VistaIORepnPrecision (graph->node_repn)) / 8;
	int osize = VistaIONodeSize (graph);
	if (newfields <= graph->nfields)
		return TRUE;
	for (i = 1; i <= graph->lastUsed; i++) {
		if (VistaIOGraphNodeIsFree (graph, i))
			continue;
		o = VistaIOGraphGetNode (graph, i);
		n = VistaIOCalloc (1, nsize);
		memcpy (n, o, osize);
		VistaIOGraphGetNode (graph, i) = n;
		VistaIOFree (o);
	};
	graph->nfields = newfields;
	return TRUE;
}

static int firstUnvisitedNode (VistaIOGraph graph)
{
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		VistaIONode n = VistaIOGraphGetNode (graph, i);
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

EXPORT_VISTA int VistaIOGraphVisitNodesFrom (VistaIOGraph graph, int i)
{
	VistaIOAdjacency adj;
	VistaIONode n, p;
	int cycles = 0;

	if (graph == 0 || (n = VistaIOGraphGetNode (graph, i)) == 0)
		return 0;
	if (n->base.hops > 0)
		return 1;
	n->base.hops++;

	for (adj = n->base.head; adj; adj = adj->next) {
		p = VistaIOGraphGetNode (graph, adj->id);
		if (p && p->base.hops > 0)
			continue;
		cycles += VistaIOGraphVisitNodesFrom (graph, adj->id);
	};
	return cycles;
}

/*! \brief Clears the hops field in a graph
 *  
 *  \param  graph
 */

EXPORT_VISTA void VistaIOGraphClearHops (VistaIOGraph graph)
{
	int i;

	if (graph == 0)
		return;
	for (i = 1; i <= graph->lastUsed; i++) {
		VistaIONode n = VistaIOGraphGetNode (graph, i);
		if (n)
			n->base.hops = 0;
	};
}

/*! \brief
 *
 *  \param  graph
 *  \return Returns number of cycles in a graph
 */

EXPORT_VISTA int VistaIOGraphNCycles (VistaIOGraph graph)
{
	int cycles = 0;

	VistaIOGraphClearHops (graph);
	while (1) {
		/* get the first unvisited node */
		int n = firstUnvisitedNode (graph);
		if (n == 0)
			return cycles;
		cycles += VistaIOGraphVisitNodesFrom (graph, n);
	};
}

/*! \brief Copies nodes from a graph in which the visited flag is set \
 *         and places them into a new graph
 *
 *  \param  src
 *  \return VistaIOGraph
 */

EXPORT_VISTA VistaIOGraph VistaIOGraphExtractNodes (VistaIOGraph src)
{
	VistaIOGraph dst;
	VistaIOAdjacency adj;
	VistaIONode n;
	int i, j;

	/* create a destination graph much like src */
	dst = VistaIOCreateGraph (src->size, src->nfields, src->node_repn,
			    src->useWeights);

	/* copy selected nodes from src */
	for (i = j = 1; i <= src->lastUsed; i++) {
		n = VistaIOGraphGetNode (src, i);
		if (n && n->base.hops)
			dst->table[j++] = VistaIOCopyNodeShallow (src, n);
	};

	/* set number of nodes used */
	dst->nnodes = j-1;
	dst->lastUsed = j;

	/* now link nodes in new graph */
	for (i = 1; i <= dst->lastUsed; i++) {
		n = VistaIOGraphGetNode (dst, i);
		if (n == 0)
			continue;

		j = VistaIOGraphLookupNode (src, n);
		if (j == 0)
			continue;
		n = VistaIOGraphGetNode (src, j);
		for (adj = n->base.head; adj; adj = adj->next) {
			n = VistaIOGraphGetNode (src, adj->id);
			j = VistaIOGraphLookupNode (dst, n);
			if (j)
				VistaIOGraphLinkNodes (dst, i, j);
		};
	};

	if (VistaIOGraphAttrList (dst))
		VistaIODestroyAttrList (VistaIOGraphAttrList (dst));
	if (VistaIOGraphAttrList (src))
		VistaIOGraphAttrList (dst) = VistaIOCopyAttrList (VistaIOGraphAttrList (src));
	return dst;
}

/*! \brief Visits all node in a graph connected to node i and toggles the hops
 *  field note that the visit field must have been cleared before
 *
 *  \param  graph
 *  \param  i
 */

EXPORT_VISTA void VistaIOGraphToggleNodesFrom (VistaIOGraph graph, int i)
{
	VistaIOAdjacency adj;
	VistaIONode n, p;

	/* find a valid starting point */
	if (graph == 0 || (n = VistaIOGraphGetNode (graph, i)) == 0)
		return;

	/* mark this node and toggle the hops field */
	if (n->base.visited == 1)
		return;
	n->base.visited = 1;
	n->base.hops = n->base.hops ? 0 : 1;

	/* now look at the neighbors */
	for (adj = n->base.head; adj; adj = adj->next) {
		p = VistaIOGraphGetNode (graph, adj->id);
		if (p && p->base.visited)
			continue;
		VistaIOGraphToggleNodesFrom (graph, adj->id);
	};
}

/*! \brief complex deletion: look at all connected structures of this node
 *
 *  \param  graph
 *  \param  i
 */

EXPORT_VISTA void VistaIODestroyNode (VistaIOGraph graph, int i)
{
	VistaIOAdjacency p, q;
	VistaIONode n;

	n = VistaIOGraphGetNode (graph, i);
	if (n == 0)
		return;

	/* destroy adjacency list */
	for (p = n->base.head; p; p = q) {
		/* remove connection from other node to this node */
		VistaIOGraphUnlinkNodes (graph, p->id, i);
		q = p->next;
		VistaIOFree (p);
	};
	VistaIOFree (n);
	VistaIOGraphGetNode (graph, i) = 0;
	graph->nnodes--;
	assert(graph->nnodes >= 0);
}

static void VistaIOGraphRemoveNodes (VistaIOGraph graph)
     /* remove all nodes in which the visited flag is set */
{
	int i;

	for (i = 1; i <= graph->lastUsed; i++) {
		VistaIONode n = VistaIOGraphGetNode (graph, i);
		if (n && n->base.visited)
			VistaIODestroyNode (graph, i);
	};
}

/*! \brief Destroys nodes from a graph in which the hops field is set
 *
 *  \param graph
 *  \param i
 */

EXPORT_VISTA void VistaIOGraphDestroyNodesFrom (VistaIOGraph graph, int i)
{
	VistaIOGraphToggleNodesFrom (graph, i);
	VistaIOGraphRemoveNodes (graph);
}

/*! \brief make bidrectional link between nodes a and b
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 */

EXPORT_VISTA void VistaIOGraphLinkNodesBi (VistaIOGraph graph, VistaIOLong a, VistaIOLong b)
{
	VistaIONode n;
	VistaIOAdjacency adj;

	if (a == 0 || b == 0 || a == b)
		return;
	n = VistaIOGraphGetNode (graph, a);
	if (n == NULL)
		return;
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == (unsigned long)b)
			goto rev;
	VistaIOGraphLinkNodes (graph, a, b);
      rev:
	n = VistaIOGraphGetNode (graph, b);
	if (n == NULL)
		return;
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == (unsigned long)a)
			return;
	VistaIOGraphLinkNodes (graph, b, a);
}

/*! \brief remove bidrectional link between nodes a and b
 * 
 *  \param  graph
 *  \param  a
 *  \param  b
 */

EXPORT_VISTA void VistaIOGraphUnlinkNodesBi (VistaIOGraph graph, VistaIOLong a, VistaIOLong b)
{
	VistaIOGraphUnlinkNodes (graph, a, b);
	VistaIOGraphUnlinkNodes (graph, b, a);
}

/*! \brief
 *
 *  \param  graph
 *  \param  a
 *  \param  b
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIOGraphHasLink (VistaIOGraph graph, int a, int b)
{
	VistaIONode n;
	VistaIOAdjacency adj; 
	
	if (a == 0 || b == 0 || a == b)
		return FALSE;
	n = VistaIOGraphGetNode (graph, a);
	
	for (adj = n->base.head; adj; adj = adj->next)
		if (adj->id == b)
			return TRUE;
	return FALSE;
}
