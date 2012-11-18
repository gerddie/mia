/*
*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
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

/* $Id: vista.h 976 2007-04-02 10:40:26Z wollny $ */

/*! \file vista.h
 *  \brief main interface for code defined in libvistaio
 *  \author Marc Tittgemeyer, tittge@cbs.mpg.de, 2004
 */

#ifndef VISTA_H
#define VISTA_H 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#  ifdef vistaio_minimal_EXPORTS
#     define EXPORT_VISTA __declspec(dllexport)
#  else
#     define EXPORT_VISTA __declspec(dllimport)
#  endif
#else 
#  define EXPORT_VISTA 
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <limits.h>
#include <stdarg.h>
#include <math.h>

#ifndef FALSE
#define FALSE	 		0
#define TRUE 			1
#endif
#define MIN_VERBOSE_LVL		0
#define MAX_VERBOSE_LVL		2

#define VistaIOMax(a,b)		((a) > (b) ? (a) : (b))
#define VistaIOMin(a,b)		((a) < (b) ? (a) : (b))
#define VistaIOOffset(type, field) 	((size_t) (((char *) & ((type) 0)->field) - (char *) 0))
#define VistaIOOffsetOf(type, field)	VistaIOOffset(type *, field)
#define VistaIONumber(array)		((size_t) (sizeof (array) / sizeof ((array)[0])))
#define VistaIOZero(array, nels) 	((void) memset ((void *) array, 0, (size_t) (nels) * sizeof ((array)[0])))
#define VistaIOCopy(from, to, nels) 	((void) memcpy ((void *) (to), (void *) (from), \
		    			(size_t) (nels) * sizeof ((from)[0])))
#define VistaIONew(type)		((type *) VistaIOMalloc (sizeof (type)))
#define VistaIONewString(str) 	((VistaIOString) ((str) ? strcpy ((char *) VistaIOMalloc (strlen (str) + 1), str) : 0))
#define VistaIOFileHeader		"V-data"
#define VistaIOFileVersion		2
#define VistaIOFileDelimiter		"\f\n"
#define VistaIOMaxAttrNameLength	256
#define VistaIORequiredOpt		(& VistaIO_RequiredOpt)
#define VistaIOOptionalOpt		(& VistaIO_OptionalOpt)
#define VistaIOAllBands		-1	/* all bands */
#define VistaIOAttrListEmpty(l)	((l) == NULL || (l)->next == NULL)
#define VistaIOFirstAttr(l,p)		((void) ((p)->list = (l), (p)->ptr = (l)->next))
#define VistaIOLastAttr(l,p)		((void) ((p)->list = (l), (p)->ptr = (l)->prev))
#define VistaIOAttrExists(p)		((p)->ptr != NULL)
#define VistaIONextAttr(p)		((void) ((p)->ptr = (p)->ptr ? (p)->ptr->next : NULL))
#define VistaIOPrevAttr(p)		((void) ((p)->ptr = (p)->ptr ? (p)->ptr->prev : NULL))
#define VistaIOGetAttrName(p)		((p)->ptr->name)
#define VistaIOGetAttrRepn(p)		((p)->ptr->repn)
#define VistaIOListCount(vlist)	((vlist)->count)
#define VistaIOListCurr(vlist)	((vlist)->current->item)
#define VistaIOListGetCurr(vlist)	((vlist)->current)
#define VistaIOListSetCurr(vlist,cur) ((void)((vlist)->current = (cur)))
#define VistaIOImageNBands(image)	((image)->nbands)
#define VistaIOImageNRows(image)	((image)->nrows)
#define VistaIOImageNColumns(image)	((image)->ncolumns)
#define VistaIOImageNFrames(image)	((image)->nframes)
#define VistaIOImageNViewpoints(image) ((image)->nviewpoints)
#define VistaIOImageNColors(image)	((image)->ncolors)
#define VistaIOImageNComponents(image) ((image)->ncomponents)
#define VistaIOPixelRepn(image)	((image)->pixel_repn)
#define VistaIOImageData(image)	((image)->data)
#define VistaIOImageAttrList(image)	((image)->attributes)
#define VistaIOImageNPixels(image) 	((image)->nbands * (image)->nrows * (image)->ncolumns)
#define VistaIOPixelSize(image)	(VistaIORepnSize ((image)->pixel_repn))
#define VistaIOPixelPrecision(image)  (VistaIORepnPrecision ((image)->pixel_repn))
#define VistaIOPixelRepnName(image)	(VistaIORepnName ((image)->pixel_repn))
#define VistaIOPixelMinValue(image)	(VistaIORepnMinValue ((image)->pixel_repn))
#define VistaIOPixelMaxValue(image)	(VistaIORepnMaxValue ((image)->pixel_repn))
#define VistaIOImageSize(image)	(VistaIOImageNPixels(image) * VistaIOPixelSize(image))
#define VistaIOPixelPtr(image, band, row, column) \
				((VistaIOPointer) ((char *) ((image)->band_index[band][row])+(column) * VistaIOPixelSize (image)))
#define VistaIOPixel(image, band, row, column, type) \
				(* ((type *) (image)->band_index[band][row] + (column)))
#define VistaIOPixelArray(image, type) ((type ***) (image)->band_index)
#define VistaIOBandIndex(image, frame, viewpoint, color, component) \
				(((((frame) * (image)->nviewpoints + (viewpoint)) * (image)->ncolors + \
					(color)) * (image)->ncomponents) + (component))
#define VistaIOSameImageRange(image1, image2)	\
				((image1)->nbands == (image2)->nbands && (image1)->nrows == (image2)->nrows && \
				(image1)->ncolumns == (image2)->ncolumns && (image1)->pixel_repn == (image2)->pixel_repn)
#define VistaIOSameImageSize(image1, image2) \
				((image1)->nbands == (image2)->nbands && (image1)->nrows == (image2)->nrows && \
				(image1)->ncolumns == (image2)->ncolumns)
#define VistaIOGraphNNodes(graph)	(graph->nnodes)
#define VistaIOGraphNFields(graph)	(graph->nfields)
#define VistaIOGraphNSize(graph)	(graph->size)
#define VistaIOGraphAttrList(graph)	(graph->attributes)
#define VistaIOGraphGetNode(graph, nid)	(graph->table[nid-1])
#define VistaIOGraphNodeIsFree(graph, nid)	(graph->table[nid-1] == 0)
#define VistaIONodeRepn(graph)	(graph->node_repn)
#define VistaIONodeSize(graph) 	(sizeof(VistaIONodeBaseRec) + (graph->nfields * VistaIORepnPrecision(graph->node_repn)) / 8)
#define VistaIONodeTestVisit(node)	(((VistaIONodeBase)node)->visited == TRUE)
#define VistaIONodeSetVisit(node)	(((VistaIONodeBase)node)->visited = TRUE)
#define VistaIONodeClearVisit(node)	(((VistaIONodeBase)node)->visited = FALSE)

#define VistaIOEdgesNRows(edges)	((edges)->nrows)
#define VistaIOEdgesNColumns(edges)	((edges)->ncolumns)
#define VistaIOEdgesAttrList(edges)	((edges)->attributes)
#define VNEdgeFields(edges)	((edges)->edge_fields)
#define VNPointFields(edges)	((edges)->npoints)
#define VNEdges(edges)		((edges)->nedges)
#define VistaIOFirstEdge(edges)	((edges)->first)
#define VistaIONextEdge(edge)		((edge)->next)
#define VistaIOEdgeExists(edge)	((edge) != NULL)
#define VistaIOEdgeFields(edge)	((edge)->edge_fields)
#define VistaIOEdgeNPoints(edge)	((edge)->npoints)
#define VistaIOEdgeClosed(edge)	((edge)->closed)
#define VistaIOEdgePointArray(edge)	((edge)->point_index)

/* Following are old macro names which should no longer be used.
   They can be removed in a future version once all of the documentation
   is in place and has been announced. */
#define VistaIOEdgesCount(edges)	((edges)->nedges)
#define VistaIOEdgePoints(edge)	((edge)->point_index)
#define VistaIOEdgesEdgeFields(edges) ((edges)->nedge_fields)
#define VistaIOEdgesPointFields(edges) ((edges)->npoint_fields)
#define VistaIOEdgesRows(edges)	((edges)->nrows)
#define VistaIOEdgesColumns(edges)	((edges)->ncolumns)
#define VistaIOEdgePointCount(edge)	((edge)->npoints)

#define VolumesAttrList(volumes) ((volumes)->attributes)
#define VolumesNum(volumes) ((volumes)->nvolumes)
#define VolumesNBands(volumes) ((volumes)->nbands)
#define VolumesNRows(volumes) ((volumes)->nrows)
#define VolumesNColumns(volumes) ((volumes)->ncolumns)
#define VolumesNVolumes(volumes) ((volumes)->ntracks)

#define VolumeNBands(volume) ((volume)->nbands)
#define VolumeNRows(volume) ((volume)->nrows)
#define VolumeNColumns(volume) ((volume)->ncolumns)
#define VolumeNBuckets(volume) ((volume)->nbuckets)
#define VolumeNTracks(volume) ((volume)->ntracks)
#define VolumeLabel(volume) ((volume)->label)
#define VistaIOFirstVolume(volumes) ((volumes)->first)
#define VistaIONextVolume(volume) ((volume)->next)
#define VolumeExists(volume) ((volume) != NULL)

#define VistaIOTrackLength(track) ((track)->length)
#define VistaIOTrackExists(track) ((track) != NULL)
#define VistaIOFirstTrack(volume,i) ((volume)->bucket[(i)].first)
#define VistaIONextTrack(track) ((track)->next)
#define VistaIOPreviousTrack(track) ((track)->previous)

/* Names of generic attributes: */
#define VistaIOCommentAttr		"comment"
#define VistaIODataAttr		"data"
#define VistaIOHistoryAttr		"history"
#define VistaIOLengthAttr		"length"
#define VistaIONameAttr		"name"
#define VNColumnsAttr		"ncolumns"
#define VNRowsAttr		"nrows"
#define VistaIORepnAttr               "repn"

/* Image attribute type names: */
#define VistaIOColorInterpAttr	"color_interp"
#define VistaIOComponentInterpAttr	"component_interp"
#define VistaIOFrameInterpAttr	"frame_interp"
#define VNBandsAttr		"nbands"
#define VNColorsAttr		"ncolors"
#define VNComponentsAttr	"ncomponents"
#define VNFramesAttr		"nframes"
#define VNViewpointsAttr	"nviewpoints"
#define VistaIOPixelAspectRatioAttr	"pixel_aspect_ratio"
#define VistaIOViewpointInterpAttr	"viewpoint_interp"

/* Graph attribute type names: */
#define VistaIOGraphAttr		"Graph"
#define VNGraphNodesAttr	"nnodes"
#define VNGraphSizeAttr		"size"
#define VNNodeFieldsAttr	"nfields"
#define VNNodeWeightsAttr	"useWeights"

/* Edge type names: */
#define VistaIOEdgesAttr		"edges"
#define VNEdgeFieldsAttr	"nedge_fields"
#define VNPointFieldsAttr	"npoint_fields"
#define VNEdgesAttr		"nedges"
#define VNPointsAttr		"npoints"

/* Volume type names: */
#define VolumesAttr     	"volumes"
#define VolNVolumesAttr 	"nvolumes"
#define VolNTracksAttr  	"ntracks"
#define VolNBandsAttr   	"nbands"
#define VolNRowsAttr    	"nrows"
#define VolNColumnsAttr 	"ncolumns"
#define COMPONENTS           "component_count"
#define VolumeHash(nbands, b, r, len) (((b) * (nbands) + (r)) % (len))

/* Macros for generating constants of particular numeric types: */
/* (These definitions may be platform-specific.) */
#define VistaIOBitConst(c)			(c)
#define VUByteConst(c)		(c)
#define VSByteConst(c)		(c)
#define VistaIOShortConst(c)		(c)
#define VistaIOLongConst(c)		(c ## l)
#define VistaIOFloatConst(c)		(c ## f)
#define VistaIODoubleConst(c) 	(c)

/* (These definitions may be platform-specific.) */
typedef char VistaIOBit;		/*!< 0 or 1 */
typedef double VistaIODouble;		/*!< >= 64-bit IEEE floating point */
typedef float VistaIOFloat;		/*!< >= 32-bit IEEE floating point */
typedef int VistaIOLong;		/*!< !! changed, G.L. 19.9.95 !! */
typedef signed char VSByte;	/*!< integer in [-128,127] */
typedef short VistaIOShort;		/*!< >= 16-bit signed integer */
typedef unsigned char VUByte;	/*!< integer in [0,255] */
typedef char VistaIOBoolean;		/*!< TRUE or FALSE */
typedef void *VistaIOPointer;		/*!< generic pointer */
typedef const void *VistaIOPointerConst;		/*!< generic pointer */
typedef const char *VistaIOStringConst;	/* null-terminated string constant */
typedef char *VistaIOString;		/*!< null-terminated string */
typedef int VistaIOBitPromoted;
typedef int VistaIOBooleanPromoted;
typedef double VistaIODoublePromoted;
typedef double VistaIOFloatPromoted;
typedef long VistaIOLongPromoted;
typedef int VSBytePromoted;
typedef int VistaIOShortPromoted;
typedef unsigned int VUBytePromoted;
typedef struct VistaIO_ImageRec *VistaIOImage;
typedef int VistaIOBand;
typedef void VistaIOErrorHandler (VistaIOStringConst);
typedef void VistaIOWarningHandler (VistaIOStringConst);
typedef VistaIOPointer VistaIOCopyMethod (VistaIOPointer);
typedef void VistaIODestroyMethod (VistaIOPointer);

extern VistaIOBoolean VistaIO_RequiredOpt, VistaIO_OptionalOpt;

/*! \brief Codes for referring to representations: */
typedef enum {
	VistaIOUnknownRepn,
	VistaIOBitRepn,		/*!< 1-bit integer, [0, 1] */
	VUByteRepn,		/*!< 8-bit integer, [0, 255] */
	VSByteRepn,		/*!< 8-bit integer, [-128, 127] */
	VistaIOShortRepn,		/*!< 16-bit integer, [-32768, 32767] */
	VistaIOLongRepn,		/*!< 32-bit integer, [-2**31, 2**31-1] */
	VistaIOFloatRepn,		/*!< 32-bit IEEE floating point */
	VistaIODoubleRepn,		/*!< 64-bit IEEE floating point */
	VistaIOAttrListRepn,		/*!< attribute list */
	VistaIOBooleanRepn,		/*!< TRUE or FALSE */
	VistaIOBundleRepn,		/*!< object of named type */
	VistaIOListRepn,		/*!< list of opaque objects */
	VistaIOPointerRepn,		/*!< pointer to opaque object */
	VistaIOStringRepn,		/*!< null-terminated string */
	VistaIOEdgesRepn,		/*!< edge set */
	VistaIOImageRepn,		/*!< image */
	VistaIOGraphRepn,		/*!< graph */
	VolumesRepn,		/*!< volumes */
	VCPEListRepn,           /*!< list of critical points */ 
	VistaIOField3DRepn,           /*!< A 3D field of 3D Vectors */
	VistaIOField2DRepn,           /*!< A 3D field of 3D Vectors */
	VNRepnKinds		/*!< number of predefined types */
} VistaIORepnKind;

/*! \brief  Values of band interpretation attributes: 
 *  
 *  Returns information about how an image's bands are to be interpreted
 */
typedef enum {
	VistaIOBandInterpNone,	/*!< no interpretation specified */
	VistaIOBandInterpOther,	/*!< unknown interpretation specified */
	VistaIOBandInterpStereoPair,
	VistaIOBandInterpRGB,
	VistaIOBandInterpComplex,
	VistaIOBandInterpGradient,
	VistaIOBandInterpIntensity,
	VistaIOBandInterpOrientation
} VistaIOBandInterp;

/*! \brief Dictionary entry: */
typedef struct {
	/* The following are initialized by the dictionary provider: */
	
	VistaIOStringConst keyword;	/*!< keyword string */
	VistaIOLong ivalue;		/*!< value, if an integer */
	VistaIOStringConst svalue;	/*!< value, if a string */

	/* The following are used only by code in VistaIOLookupDictValue: */
	VistaIOBoolean icached;	/*!< whether integer value cached */
	VistaIOBoolean fcached;	/*!< whether float value cached */
	VistaIODouble fvalue;		/*!< cached floating-point value */
} VistaIODictEntry;

/*! \brief Accepted command options are described by a table of these entries */
typedef struct {
	VistaIOStringConst keyword;	/*!< keyword signalling option */
	VistaIORepnKind repn; 	/*!< type of value supplied */
	int number; 		/*!< number of values supplied*/
	VistaIOPointer value; 	/*!< location for storing value(s) */
	VistaIOBoolean *found; 	/*!< whether optionl arguments */
	VistaIODictEntry *dict;	/*!< optional dictionary of value */
	VistaIOStringConst blurb;	/*!< online help blurb */
} VistaIOOptionDescRec;

/*! \brief If an option takes multiple values, they are represented by 
 *         a VistaIOArgVector: 
 */
typedef struct {
	int number;		/*!< number of arguments */
	VistaIOPointer vector;	/*!< vector of arguments */
} VistaIOArgVector;

/*! \brief Each attribute name/value pair is represented by: */
typedef struct VistaIO_AttrRec {
	struct VistaIO_AttrRec *next;	/*!< next in list */
	struct VistaIO_AttrRec *prev;	/*!< previous in list */
	VistaIORepnKind repn;		/*!< rep'n of attribute value */
	VistaIOPointer value;		/*!< pointer to attribute value */
	char name[1];		/*!< beginning of name string */
} VistaIOAttrRec;

typedef VistaIOAttrRec *VistaIOAttrList;

/*! \brief Position within a list of attributes: */
typedef struct {
	VistaIOAttrList list;		/*!< the list */
	struct VistaIO_AttrRec *ptr;	/*!< position within the list */
} VistaIOAttrListPosn;

/*! \brief Result of trying to retrieve an attribute's value: */
typedef enum {
	VistaIOAttrFound,		/*!< successfully retrieved value */
	VistaIOAttrMissing,		/*!< didn't find attribute */
	VistaIOAttrBadValue		/*!< incompatible value */
} VistaIOGetAttrResult;

/*! \brief An object whose type is named but not registered: */
typedef struct {
	VistaIOAttrList list;		/*!< object's attribute list value */
	size_t length;		/*!< length of binary data */
	VistaIOPointer data;		/*!< pointer to binary data */
	char type_name[1];	/*!< beginning of object's type's name */
} VistaIOBundleRec, *VistaIOBundle;

typedef VistaIOPointer VistaIODecodeMethod (VistaIOStringConst, VistaIOBundle);
typedef VistaIOAttrList VistaIOEncodeAttrMethod (VistaIOPointer, size_t *);
typedef VistaIOPointer VistaIOEncodeDataMethod (VistaIOPointer, VistaIOAttrList, size_t, VistaIOBoolean *);

/*! \brief Set of methods supporting an object type: */
typedef struct {
	VistaIOCopyMethod *copy;
	VistaIODestroyMethod *destroy;
	VistaIODecodeMethod *decode;
	VistaIOEncodeAttrMethod *encode_attr;
	VistaIOEncodeDataMethod *encode_data;
} VistaIOTypeMethods;

/*! \brief Information about a representation: */
typedef struct {
	VistaIOStringConst name;	/*!< name string */
	size_t size;		/*!< size, in bytes */
	int precision;		/*!< precision, in bits */
	VistaIODouble min_value;	/*!< min and max representable values */
	VistaIODouble max_value;
	VistaIOTypeMethods *methods;	/*!< associated methods */
} VistaIORepnInfoRec;

/*! \brief List element: */
typedef struct VistaIO_Node *VistaIONodePtrType;
/*! \brief List element: */
struct VistaIO_Node {
	VistaIOPointer item;		/*!< pointer to data item */
	VistaIONodePtrType prev;	/*!< pointer to previous node */
	VistaIONodePtrType next;	/*!< pointer to next node */
};

/*! \brief List head: */
typedef struct VistaIO_List {
	VistaIONodePtrType current;	/*!< pointer to current node */
	VistaIONodePtrType head;	/*!< pointer to head node */
	VistaIONodePtrType tail;	/*!< pointer to tail node */
	int count;		/*!< number of nodes in VistaIOList */
} *VistaIOList;

/*! \brief Description of an image: */
typedef struct VistaIO_ImageRec {
	int nbands;		/*!< number of bands */
	int nrows;		/*!< number of rows */
	int ncolumns;		/*!< number of columns */
	VistaIORepnKind pixel_repn;	/*!< representation of pixel values */
	unsigned long flags;	/*!< various flags */
	VistaIOAttrList attributes;	/*!< list of other image attributes */
	VistaIOPointer data;		/*!< array of image pixel values */
	VistaIOPointer *row_index;	/*!< ptr to first pixel of each row */
	VistaIOPointer **band_index;	/*!< ptr to first row of each band */
	int nframes;		/*!< number of motion frames */
	int nviewpoints;	/*!< number of camera viewpoints */
	int ncolors;		/*!< number of color channels */
	int ncomponents;	/*!< number of vector components */
} VistaIOImageRec;

/*! \brief Codes for flags: */
enum {
	VistaIOImageSingleAlloc = 0x01	/*!< one free() releases everything */
};

/*! \brief Description of a Graph 
 *  
 *  Vista  represents  a  graph as a list of connected nodes. Nodes and
 *  connections may have weights, connections may be uni-  or  bidirec-
 *  tional in a graph.  The actual representation of a node is achieved
 *  by subclassing from a VistaIONodeBase structure, which implements the i/o
 *  and  bookkeeping part.  Subclasses of VistaIONodeBase may implement f.ex.
 *  a vertex by adding x, y, and z coordinates or polygons by recording
 *  a  list  of vertex references.  Besides node data, a graph may also
 *  has an arbitrary list of attributes associated with it.
 *
 *  A graph in memory is referred to by the C pointer type  VistaIOGraph.  In
 *  data  files it is identified by the type name graph, and in memory,
 *  by the VistaIORepnKind code VistaIOGraphRepn. Since it  is  a  standard  object
 *  type with built-in support in the Vista library, graphs can be read
 *  from data files, written to data files, and manipulated as part  of
 *  attribute   lists  by  routines  such  as  VistaIOReadFile(3Vi),  VistaIOWrite-
 *  File(3Vi), and VistaIOGetAttr(3Vi). 
 */
typedef struct VistaIO_GraphRec {
	int nnodes;		/*!< number of nodes */
	int nfields;		/*!< size of fields in a node´s private area */
	VistaIORepnKind node_repn;	/*!< data representation in a node */
	VistaIOAttrList attributes;	/*!< list of other attributes */
	struct VistaIONodestruct **table;	/*!< node table of Graph */
	int size;		/*!< number of places in table */
	int lastUsed;		/*!< last entry used in table */
	int iter;		/*!< iteration counter in sequential access */
	int useWeights;		/*!< TRUE iff weights are used */
} VistaIOGraphRec, *VistaIOGraph;

/*! \brief Description of the base of a node */
typedef struct VistaIONodebaseStruct {
	unsigned int hops:31;	/*!< number of hops in this node */
	unsigned int visited:1;	/*!< true if seen before */
	VistaIOFloat weight;		/*!< weight of this node */
	struct VistaIOAdjstruct *head;
} VistaIONodeBaseRec, *VistaIONodeBase;

/*! \brief Description of a node */
typedef struct VistaIONodestruct {
	VistaIONodeBaseRec base;
	char data[1];		/*!< private data area of node starts here */
} VistaIONodeRec, *VistaIONode;

/*! \brief Information about adjacency of nodes */
typedef struct VistaIOAdjstruct {
	unsigned int id;	/*!< node reference */
	VistaIOFloat weight;		/*!< weight of this node */
	struct VistaIOAdjstruct *next;	/* list of adjacent nodes */
} VistaIOAdjRec, *VistaIOAdjacency;

/*! \brief General information about an edge set
 *
 *  A  Vista  edge  set  is typically used to represent two-dimensional
 *  connected edge points or lines that have been  extracted  from  im-
 *  ages,  but  it can also be used to represent any sets of vectors of
 *  floating point values.
 *
 *  An edge set in memory is referred to by the C pointer type  VistaIOEdges.
 *  In  data files it's identified by the type name edges, and in memo-
 *  ry, by the VistaIORepnKind code VistaIOEdgesRepn. Since it is a standard object
 *  type  with  built-in support in the Vista library, edge sets can be
 *  read from data files, written to data  files,  and  manipulated  as
 *  part of attribute lists by routines such as VistaIOReadFile(3Vi), VistaIOWrite-
 *  File(3Vi), and VistaIOGetAttr(3Vi).
 *								     
 *  A single edge, a member of an edge set, is referred  to  by  the  C
 *  pointer type VistaIOEdge.
 */
typedef struct VistaIO_EdgesRec {
	int nrows;		/*!< number of rows */
	int ncolumns;		/*!< number of columns */
	VistaIOAttrList attributes;	/*!< list of other attributes */
	int nedge_fields;	/*!< number of fields in each edge record */
	int npoint_fields;	/*!< number of fields in each point record */
	int nedges;		/*!< number of edges */
	int npoints;		/*!< total number of points */
	struct VistaIOEdgeStruct *first;	/*!< first edge in linked list of edges */
	struct VistaIOEdgeStruct *last;	/*!< last edge in linked list of edges */
	VistaIOPointer free;		/*!< free this storage when destroying edges */
} VistaIOEdgesRec, *VistaIOEdges;

/*! \brief Topological information about an edge set. */
typedef struct VistaIOEdgeStruct {
	struct VistaIOEdgeStruct *next;	/*!< next edge in linked list of edges */
	VistaIOFloat *edge_fields;	/*!< vector of field entries for this edge */
	VistaIOBoolean closed;	/*!< indicates closed edge (a loop) */
	int npoints;		/*!< number of points in this edge */
	VistaIOFloat **point_index;	/*!< pointers to start of each point */
	VistaIOPointer free;		/*!< free this storage when destroying edges */
} VistaIOEdgeRec, *VistaIOEdge;

/*! \brief Description of a track */
typedef struct VistaIOTrackStruct {
	short band;
	short row;
	short col;
	short length;
	struct VistaIOTrackStruct *next;
	struct VistaIOTrackStruct *previous;
} *VistaIOTrack, VistaIOTrackRec;

/*! \brief Information needed to handel tracks */
typedef struct VistaIOBucketStruct {
	short ntracks;		/*!< number of tracks in one hashtable bucket */
	VistaIOTrack first;		/*!< ptr to first track in bucket             */
	VistaIOTrack last;		/*!< ptr to last track in bucket              */
} *VistaIOBucket, VistaIOBucketRec;

/*! \brief Description of a volume */
typedef struct VolumeStruct {
	short label;
	short nbands;
	short nrows;
	short ncolumns;
	short nbuckets;		/*!< length of hash table (number of buckets) */
	int ntracks;		/*!< total number of tracks in all buckets   */
	VistaIOBucket bucket;		/*!< ptrs to buckets      */
	struct VolumeStruct *next;
} VolumeRec, *Volume;

/*! \brief Description of a set of volume */
typedef struct VistaIO_VolumesRec {
	VistaIOAttrList attributes;
	short nvolumes;		/*!< number of volumes in list       */
	short nbands;
	short nrows;
	short ncolumns;
	Volume first;		/*!< ptr to first volume in list     */
} VolumesRec, *Volumes;

#define MAXHASHLEN 1024		/*!< max length of hash table */


/* A list of attributes is represented by a header node: */
typedef enum { VistaIOLsbFirst, VistaIOMsbFirst } VistaIOPackOrder;
typedef VistaIOBoolean VistaIOReadFileFilterProc (VistaIOBundle, VistaIORepnKind);

/* Macros for accessing information about representations: */
#define VistaIORepnSize(repn)			(VistaIORepnInfo[repn].size)
#define VistaIORepnPrecision(repn)	(VistaIORepnInfo[repn].precision)
#define VistaIORepnName(repn)		(VistaIORepnInfo[repn].name)
#define VistaIORepnMinValue(repn)	(VistaIORepnInfo[repn].min_value)
#define VistaIORepnMaxValue(repn)	(VistaIORepnInfo[repn].max_value)
#define VistaIORepnMethods(repn)	(VistaIORepnInfo[repn].methods)
#define VistaIOIsIntegerRepn(repn)	((repn) >= VistaIOBitRepn && (repn) <= VistaIOLongRepn)
#define VistaIOIsFloatPtRepn(repn)	((repn) == VistaIOFloatRepn || (repn) == VistaIODoubleRepn)
#define VistaIORandomDouble()		((VistaIODouble) drand48 ())
#define VistaIORandomSeed(seed)	srand48 ((long) seed)
#define VistaIORandomLong()			((VistaIOLong) mrand48 ())

/*  Declarations of library routines. */
#ifdef __cplusplus
extern "C" {
#endif

  EXPORT_VISTA extern VistaIORepnInfoRec *VistaIORepnInfo;
  extern EXPORT_VISTA VistaIODictEntry VistaIOBooleanDict[];	/*!< boolean values */
  extern EXPORT_VISTA VistaIODictEntry VistaIONumericRepnDict[];	/*!< numeric representation kinds */
  extern EXPORT_VISTA VistaIODictEntry VistaIOBandInterpDict[];

	typedef void (*VistaIOShowProgressFunc)(int pos, int length, void *data);
	
	extern int VERBOSE;
	
	/*! \brief a function to set a file io progress indicator
	 *  \param   show_read the callback used on read
	 *  \param   show_write the callback used on write
	 *  \param   data some data that is passed to the callback 
	 */
	EXPORT_VISTA void VistaIOSetProgressIndicator(VistaIOShowProgressFunc show_read, VistaIOShowProgressFunc show_write, void *data);

	/*! \brief A function to reaset the progress callback to the standart behavior (be quiet) */
	EXPORT_VISTA void VistaIOResetProgressIndicator(void);
	
	/*! \brief The function to create a VistaIOGraph. 
	 *
	 *  \param size number of nodes in the Graph
	 *  \param nfields number of fields in a node
	 *  \param repn type of fields in the node
	 *  \param use_weight whether the weight is used in the graph nodes
	 *  \returns a newly created Graph
	 */
	EXPORT_VISTA VistaIOGraph VistaIOCreateGraph (int size, int nfields , VistaIORepnKind repn , int use_weight);
	
	/*! \brief Copy a VistaIOGraph object.
	 *
	 *  Note that no compaction is performed, since this would require
	 *  a recalculation of all indices,
	 *
	 *  \param  src
	 *  \return VistaIOGraph
	 */
	EXPORT_VISTA VistaIOGraph VistaIOCopyGraph (VistaIOGraph src);

	/*! \brief Frees memory occupied by a graph.
	 *
	 *  \param graph
	 */
	EXPORT_VISTA void VistaIODestroyGraph (VistaIOGraph graph);

	/*! \brief Read a Vista data file, extract the graphs from it, and return 
	 *         a list of them.
	 *
	 *  \param  file
	 *  \param  attrs
	 *  \param  graphs
	 *  \return int
	 */
	EXPORT_VISTA int VistaIOReadGraphs (FILE *file, VistaIOAttrList *attrs, VistaIOGraph **graphs);

	/*! \brief Write a list of graphs to a Vista data file.
	 * 
	 *  \param  file
	 *  \param  attrs
	 *  \param  n
	 *  \param  graphs
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOWriteGraphs (FILE *file, VistaIOAttrList attrs, int n, VistaIOGraph *graphs);

	/*! \brief Find a node in a Vista graph structure.
	 *  
	 *  \param  graph
	 *  \param  node
	 *  \return Return reference to this node.
	 */
	EXPORT_VISTA int VistaIOGraphLookupNode (VistaIOGraph graph, VistaIONode node);

	/*! \brief Add a node to a Vista graph structure.
	 *  
	 *  \param  graph
	 *  \param  node
	 *  \return Return reference to this node.
	 */
	EXPORT_VISTA int VistaIOGraphAddNode (VistaIOGraph graph, VistaIONode node);

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
	EXPORT_VISTA int VistaIOGraphAddNodeAt (VistaIOGraph graph, VistaIONode node, int position);

	/*! \brief Make a link between to nodes.
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return Return TRUE if successful.
	 */
	EXPORT_VISTA int VistaIOGraphLinkNodes (VistaIOGraph graph, int a, int b);

	/*! \brief unlinks two nodes.
	 *
	 *  Return TRUE if successful.
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return int
	 */
	EXPORT_VISTA int VistaIOGraphUnlinkNodes (VistaIOGraph graph, int a, int b);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return VistaIOPointer
	 */
	EXPORT_VISTA VistaIOPointer VistaIOGraphFirstNode (VistaIOGraph graph);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return VistaIOPointer
	 */
	EXPORT_VISTA VistaIOPointer VistaIOGraphNextNode (VistaIOGraph graph);

	/*! \brief
	 *
	 *  \param  graph
	 */	
	EXPORT_VISTA void VistaIOGraphClearVisit (VistaIOGraph graph);

	/*! \brief Visits all node in a graph connected to node i
	 *
	 *  \param  graph
	 *  \param  i
	 *  \return int
	 */
	EXPORT_VISTA int VistaIOGraphVisitNodesFrom (VistaIOGraph graph, int i);

	/*! \brief Grow private data area of each node to newfields.
	 *  
	 *  \param  graph
	 *  \param  newfields
	 *  \return Return TRUE if successful.
	 */
	EXPORT_VISTA int VistaIOGraphResizeFields (VistaIOGraph graph, int newfields);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return Returns number of cycles in a graph
	 */
	EXPORT_VISTA int VistaIOGraphNCycles (VistaIOGraph graph);

	/*! \brief Visits all node in a graph connected to node i and toggles the hops
	 *  field note that the visit field must have been cleared before
	 *
	 *  \param  graph
	 *  \param  i
	 */
	EXPORT_VISTA void VistaIOGraphToggleNodesFrom (VistaIOGraph graph, int i);

	/*! \brief make bidrectional link between nodes a and b
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 */
	EXPORT_VISTA void VistaIOGraphLinkNodesBi (VistaIOGraph graph, VistaIOLong a, VistaIOLong b);

	/*! \brief remove bidrectional link between nodes a and b
	 * 
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 */
	EXPORT_VISTA void VistaIOGraphUnlinkNodesBi (VistaIOGraph graph, VistaIOLong a, VistaIOLong b);

	/*! \brief
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOGraphHasLink (VistaIOGraph graph, int a, int b);

	/*! \brief complex deletion: look at all connected structures of this node
	 *
	 *  \param  graph
	 *  \param  i
	 */
	EXPORT_VISTA void VistaIODestroyNode (VistaIOGraph graph, int i);

	/*! \brief Destroys nodes from a graph in which the hops field is set
	 *
	 *  \param graph
	 *  \param i
	 */
	EXPORT_VISTA void VistaIOGraphDestroyNodesFrom (VistaIOGraph graph, int i);

	/*! \brief Clears the hops field in a graph
	 *  
	 *  \param  graph
	 */
	EXPORT_VISTA void VistaIOGraphClearHops (VistaIOGraph graph);

	/*! \brief Identify the files specified by command line arguments.
	 *
	 *  Files can be provided in any of three ways:
	 *   (1) as arguments to a switch, -keyword (e.g, -in file1 file2)
	 *   (2) as additional command line arguments, not associated with
	 *       any switch (e.g., vview file1 file2)
	 *   (3) by piping to/from stdin or stdout (e.g., vview < file1).
	 *
	 *  VistaIOIdentifyFiles collects file names from these three sources once
	 *  VistaIOParseCommand has been called to parse the command's switches.
	 *  It looks first for the keyword, then for unclaimed command line
	 *  arguments, and finally for a file or pipe attached to stdin or stdout.
	 *
	 *  \param  noptions
	 *  \param  options
	 *  \param  keyword
	 *  \param  argc
	 *  \param  argv
	 *  \param  fd
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOIdentifyFiles (int noptions, VistaIOOptionDescRec options[], 
		VistaIOStringConst keyword, int *argc, char **argv, int fd);

	/*! \brief Parse command line arguments according to a table of option descriptors.
	 *  Unrecognized options are left in argv, and argc is adjusted to reflect
	 *  their number.
	 *  If an erroneous (as opposed to simply unrecognized) argument is
	 *  encountered, VistaIOParseCommand returns FALSE; otherwise, TRUE.
	 *
	 *  The -help option is recognized explicitly. If it is present, VistaIOParseCommand
	 *  returns indicating that all arguments were recognized, but that an error
	 *  occurred. This should force the caller to simply print usage information.
	 *
	 *  \param  noptions
	 *  \param  options
	 *  \param  argc
	 *  \param  argv
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOParseCommand (int noptions, VistaIOOptionDescRec options[], int *argc, char **argv);

	/*! \brief Does all of the standard command line parsing and file location needed
	 *  by Vista modules with a maximum of one input and output file.
	 *  This routine prints the appropriate error messages and exits if
	 *  necessary, so there is no need to check for a return value.
	 *
	 *  "inp" and "outp" are pointers to file variables.  The files can be
	 *  specified in command line arguments according to Vista conventions,
	 *  or else default to stdin and stdout respectively.  If either pointer 
	 *  is NULL, then it is assumed that the file is not needed.
	 *
	 *  The standard "-in" and "-out" command line options are added by this
	 *  routine so that they do not need to be specified in options arguments.
	 *
	 *  \param  noptions	 
	 *  \param  opts
	 *  \param  argc
	 *  \param  argv
	 *  \param  inp
	 *  \param  outp
	 */
	void VistaIOParseFilterCmd (int noptions, VistaIOOptionDescRec opts[], int argc, 
		char **argv,  FILE **inp, FILE **outp);

	/*! \brief Print the settings of a set of command line options.
	 *
	 *  \param f
	 *  \param noptions
	 *  \param options
	 */
	void VistaIOPrintOptions (FILE *f, int noptions, VistaIOOptionDescRec options[]);

	/*! \brief Print the value of a specified option.
	 *
	 *  \param f
	 *  \param option
	 */
	int VistaIOPrintOptionValue (FILE *f, VistaIOOptionDescRec *option);

	/*! \brief Report the remaining command line arguments in argv as ones that could
	 *  not be recognized.
	 *
	 *  \param argc
	 *  \param argv
	 */
	void VistaIOReportBadArgs (int argc, char **argv);

	/*! \brief Print, to stderr, information about how to use a program based on
	 *  the contents of its command argument parsing table.
	 *
	 *  \param program
	 *  \param noptions
	 *  \param options
	 *  \param other_args
	 */
	void VistaIOReportUsage (VistaIOStringConst program, int noptions, 
		VistaIOOptionDescRec options[], VistaIOStringConst other_args);

	/*! \brief Print, to stderr, a summary of program options based on the contents of
	 *  a command argument parsing table
	 *
	 *  \param noptions
	 *  \param options
	 */
	void VistaIOReportValidOptions (int noptions, VistaIOOptionDescRec options[]);

	/*! \brief Allocates memory for a new image with specified properties.
	 *  
	 *  Returns a pointer to the image if successful, zero otherwise.
	 *  
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncolumns
	 *  \param  pixel_repn
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA VistaIOImage VistaIOCreateImage (int nbands, int nrows, int ncolumns, VistaIORepnKind pixel_repn);

	/*! \brief Create an image with the same properties as an existing one.
	 *
	 *  \param  src
	 *  \return VistaIOImage 
	 */
	EXPORT_VISTA VistaIOImage VistaIOCreateImageLike (VistaIOImage src);

	/*! \brief Frees memory occupied by an image.
	 *
	 *  \param image
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA void VistaIODestroyImage (VistaIOImage image);

	/*! \brief Fetch a pixel value, regardless of type, and return it as a Double.
	 *
	 *  \param  image
	 *  \param  band
	 *  \param  row
	 *  \param  column
	 *  \return VistaIODouble
	 */
	EXPORT_VISTA VistaIODouble VistaIOGetPixel (VistaIOImage image, int band, int row, int column);

	/*! \brief Set a pixel, regardless of type, and to a value passed as a Double.
	 *
	 *  \param image
	 *  \param band
	 *  \param row
	 *  \param column
	 *  \param value
	 */
	EXPORT_VISTA void VistaIOSetPixel (VistaIOImage image, int band, int row, int column, 
		VistaIODoublePromoted value);

	/*! \brief Copy the pixels and attributes of one image to another.
	 *
	 *  Returns a pointer to the destination image if successful, zero otherwise.
	 *  The band parameter may be VistaIOAllBands, in which case all bands of pixel
	 *  values are copied, or a particular band number, in which case only a
	 *  single band is copied to a 1-band destination image.
	 *
	 *  \param  src
	 *  \param  dest 
	 *  \param  band 
	 *  \return Returns a pointer to the destination image if successful, zero 
	 *          otherwise
	 */
	EXPORT_VISTA VistaIOImage VistaIOCopyImage (VistaIOImage src, VistaIOImage dest, VistaIOBand band);

	/*! \brief Give a destination image the same attributes as a source image.
	 *  
	 *  However if the destination image doesn't have the same number of bands
	 *  as the source image, any band interpretation attributes are deleted.
	 *
	 *  \param  src
	 *  \param  dest
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA VistaIOImage VistaIOCopyImageAttrs (VistaIOImage src, VistaIOImage dest);

	/*! \brief Copy the pixels of one image to another.
	 *
	 *  Returns a pointer to the destination image if successful, zero otherwise.
	 *  The band parameter may be VistaIOAllBands, in which case all bands of pixel
	 *  values are copied, or a particular band number, in which case only a
	 *  single band is copied to a 1-band destination image.
	 *
	 *  \param  src
	 *  \param  dest
	 *  \param  band
	 *  \return Returns a pointer to the destination image if successful, 
	 *          zero otherwise.
	 */
	EXPORT_VISTA VistaIOImage VistaIOCopyImagePixels (VistaIOImage src, VistaIOImage dest, VistaIOBand band);

	/*! \brief Copy a band of pixel data from one image to another.
	 *
	 *  Band src_band of image src is copied to band dest_band of image dest.
	 *  The destination image must exist, having the same pixel representation
	 *  and size as the source image. Either src_band or dst_band may be
	 *  VistaIOAllBands, provided they both describe the same number of bands.
	 *
	 *  \param  src
	 *  \param  src_band
	 *  \param  dest
	 *  \param  dest_band
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOCopyBand (VistaIOImage src, VistaIOBand src_band, VistaIOImage dest, VistaIOBand dest_band);

	/*! \brief Copy a series of bands from various images into a destination image.
	 *  
	 *  nbands is the number of bands to be copied, and the number of elements
	 *  in src_images (a list of images from which to obtain the bands) and
	 *  src_bands (a list of their respective band numbers). The bands are
	 *  copied into the destination image in sequence.
	 *
	 *  \param  nels
	 *  \param  src_images
	 *  \param  src_bands
	 *  \param  dest
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA VistaIOImage VistaIOCombineBands (int nels, VistaIOImage src_images[], VistaIOBand src_bands[], 
		VistaIOImage dest);

	/*! \brief A varargs version of VistaIOCombineBands. 
	 *
	 *  It is called by:
	 *
	 *	dest = VistaIOCombineBandsVa (dest, src_image1, src_band1, ...,
	 *				(VistaIOImage) NULL);
	 *  \param  dest
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA VistaIOImage VistaIOCombineBandsVa (VistaIOImage dest, ...);

	/*! \brief Check that a destination image provided for an operation has the
	 *  appropriate number of rows, columns and bands, and a pixel representation.
	 *  Or, if no destination image is provided, create one.
	 *
	 *  \param  routine
	 *  \param  dest
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncolumns
	 *  \param  pixel_repn
	 *  \return VistaIOImage
	 */
	EXPORT_VISTA VistaIOImage VistaIOSelectDestImage (VistaIOStringConst routine, VistaIOImage dest, int nbands, 
		int nrows, int ncolumns,	VistaIORepnKind pixel_repn);

	/*! \brief Check a band specification and use it to determine the number and
	 *         address of a block of pixels.
	 *
	 *  \param  routine
	 *  \param  image
	 *  \param  band
	 *  \param  npixels
	 *  \param  first_pixel
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOSelectBand (VistaIOStringConst routine, VistaIOImage image, VistaIOBand band, 
		int *npixels, VistaIOPointer *first_pixel);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VistaIOBandInterp
	 */
	EXPORT_VISTA VistaIOBandInterp VistaIOImageFrameInterp (VistaIOImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VistaIOBandInterp
	 */ 
	EXPORT_VISTA VistaIOBandInterp VistaIOImageViewpointInterp (VistaIOImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VistaIOBandInterp
	 */
	EXPORT_VISTA VistaIOBandInterp VistaIOImageColorInterp (VistaIOImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VistaIOBandInterp
	 */
	EXPORT_VISTA VistaIOBandInterp VistaIOImageComponentInterp (VistaIOImage image);

	/*! \brief Set all of an image's standard band interpretation attributes.
	 *
	 *  \param  image
	 *  \param  frame_interp
	 *  \param  nframes
	 *  \param  viewpoint_interp
	 *  \param  nviewpoints
	 *  \param  color_interp
	 *  \param  ncolors
	 *  \param  component_interp
	 *  \param  ncomponents
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOSetBandInterp (VistaIOImage image, VistaIOBandInterp frame_interp, 
		int nframes, VistaIOBandInterp viewpoint_interp, int nviewpoints, 
		VistaIOBandInterp color_interp, int ncolors, VistaIOBandInterp component_interp, 
		int ncomponents);

	/*! \brief Read a Vista data file, extract the images from it, and return a 
	 *         list of them.
	 * 
	 *  \param  file
	 *  \param  attributes
	 *  \param  images
	 *  \return int
	 */
	EXPORT_VISTA int VistaIOReadImages (FILE *file, VistaIOAttrList *attributes, VistaIOImage **images);

	/*! \brief Write a list of images to a Vista data file.
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nimages
	 *  \param  images
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOWriteImages (FILE *file, VistaIOAttrList attributes, int nimages, 
		VistaIOImage images[]);

	/*! \brief Open an input or output file, with "-" representing stdin or stdout.
	 *  
	 *  \param  filename
	 *  \param  nofail
	 *  \return If nofail is TRUE, any failure is a fatal error.
	 */
	FILE *VistaIOOpenInputFile (VistaIOStringConst filename, VistaIOBoolean nofail);

	/*! \brief
	 *  
	 *  \param  filename
	 *  \param  nofail
	 *  \return FILE
	 */	
	FILE *VistaIOOpenOutputFile (VistaIOStringConst filename, VistaIOBoolean nofail);

	/*! \brief Read a Vista data file, extract object of a specified type, and 
	 *  return a vector of them plus a list of anything else found in the file.
	 *
	 *  \param  file
	 *  \param  repn
	 *  \param  attributes
	 *  \param  objects
	 *  \return int
	 */
	int VistaIOReadObjects (FILE *file, VistaIORepnKind repn, VistaIOAttrList *attributes, VistaIOPointer **objects);

	/*! \brief Read a Vista data file, returning an attribute list of its contents.
	 *
	 *  \param  f
	 *  \param  filter
	 *  \return VistaIOAttrList
	 */
	EXPORT_VISTA VistaIOAttrList VistaIOReadFile (FILE * f, VistaIOReadFileFilterProc *filter);

	/*! \brief Write a list of objects, plus some other attributes, to a Vista data file.
	 *
	 *  \param  file
	 *  \param  repn
	 *  \param  attributes
	 *  \param  nobjects
	 *  \param  objects
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOWriteObjects (FILE *file, VistaIORepnKind repn, VistaIOAttrList attributes, 
		int nobjects, VistaIOPointer objects[]);

	/*! \brief VistaIOWriteFile
	 *  
	 *  \param  f
	 *  \param  list
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOWriteFile (FILE *f, VistaIOAttrList list);

	/*! \brief Make a new, empty list, and returns its reference.
	 *
	 *  \return VistaIOList
	 */
	VistaIOList VistaIOListCreate ();

	/*! \brief Return a pointer to the first item in vlist, 
	 *         and make the first item the current item.
	 *
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListFirst (VistaIOList vlist);

	/*! \brief Return a pointer to the last item in vlist,
	 *         and make the last item the current item.
	 *
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListLast (VistaIOList  vlist);

	/*! \brief Advance vlist's current item by one, return the
	 *         new current item. Return NULL if the new current
	 *         item is beyond the end of vlist.
	 *
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListNext (VistaIOList vlist);

	/*! \brief Back up vlist's current item by one, return the
	 *         new current item. Return NULL if the new current
	 *         item is before the beginning of vlist.
	 *
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListPrev (VistaIOList vlist);

	/*! \brief Add item to vlist immediately after the current item, 
	 *
	 *  and make item the current item. If the
	 *  current pointer is before the beginning of vlist,
	 *  item is added at the beginning. If the current
	 *  pointer is beyond the end of vlist, item is
	 *  added at the end.
	 *
	 *  \param  vlist
	 *  \param  item
	 */
	void VistaIOListAdd (VistaIOList vlist, VistaIOPointer item);

	/*! \brief Add item to vlist immediately before the current item, 
	 *
	 *  and make item the current item. If the current pointer is before 
	 *  the beginning of vlist, item is added at the beginning. If the current
	 *  pointer is beyond the end of vlist, item is added at the end.
	 *
	 *  \param  vlist
	 *  \param item
	 */	
	void VistaIOListInsert (VistaIOList vlist, VistaIOPointer item);

	/*! \brief Add item to the end of vlist, and make item the current item.
	 *
	 *  \param vlist
	 *  \param item
	 */
	EXPORT_VISTA void VistaIOListAppend (VistaIOList vlist, VistaIOPointer item);

	/*! \brief Add item to the beginning of vlist, and make
	 *         item the current item.
	 *
	 *  \param vlist
	 *  \param item 
	 */
	void VistaIOListPrepend (VistaIOList vlist, VistaIOPointer item);

	/*! \brief Return current item and take it out of vlist.
	 * 
	 *  Make the next item the current one.
	 *
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListRemove (VistaIOList vlist);

	/*! \brief Add vlist2 to the end of vlist1. 
	 *
	 *  The current pointer is set to the current pointer of vlist1.
	 *  vlist2 no longer exists after the operation.
	 *
	 *  \param  vlist1
	 *  \param  vlist2
	 */
	void VistaIOListConcat (VistaIOList vlist1, VistaIOList vlist2);

	/*! \brief Delete vlist. 
	 *
	 *  \param vlist
	 *  \param item_free A pointer to a routine that frees an item.
	 */
	void VistaIOListDestroy (VistaIOList vlist, void (*item_free)(VistaIOPointer));

	/*! \brief Return last item and take it out of vlist. 
	 *
	 *  Make the new last item the current one.
	 *  
	 *  \param  vlist
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListTrim (VistaIOList vlist);

	/*! \brief Searche vlist starting at the current item until the end is 
	 *         reached or a match is found.
	 *
	 *  \param  vlist
	 *  \param  comp
	 *  \param  comp_arg
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOListSearch (VistaIOList vlist, int (*comp)(), VistaIOPointer comp_arg);

	/*! \brief Perform error checking on calloc() call.
	 *
	 *  \param  n
	 *  \param  size
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOCalloc (size_t n, size_t size);

	/*! \brief Perform error checking on free() call.
	 *
	 *  \param p
	 */
	EXPORT_VISTA void VistaIOFree (VistaIOPointer p);

	/*! \brief Perform error checking on malloc() call.
	 *
	 *  \param  size
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIOMalloc (size_t size);

	/*! \brief Perform error checking on realloc() call.
	 *
	 *  \param  p
	 *  \param  size
	 *  \return VistaIOPointer
	 */
	VistaIOPointer VistaIORealloc (VistaIOPointer p, size_t size);

	/*! \brief Append a new attribute to a list.
	 *
	 *  The calling sequence is:
	 *
	 *	VistaIOAppendAttr (VistaIOAttrList list, VistaIOStringConst name,
	 *		     VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  dict
	 *  \param  repn
	 */
	EXPORT_VISTA void VistaIOAppendAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict, 
		VistaIORepnKind repn , ...);

	/*! \brief Make a copy of an attribute list.
	 * 
	 *  \param  list 
	 *  \return VistaIOAttrList
	 */
	EXPORT_VISTA VistaIOAttrList VistaIOCopyAttrList (VistaIOAttrList list);

	/*! \brief Create an attribute list. */
	EXPORT_VISTA VistaIOAttrList VistaIOCreateAttrList ();

	/*! \brief Create a bundle object.
	 *
	 *  \param  type_name
	 *  \param  list
	 *  \param  length
	 *  \param  data
	 *  \return VistaIOBundle
	 */
	EXPORT_VISTA VistaIOBundle VistaIOCreateBundle (VistaIOStringConst type_name, VistaIOAttrList list, size_t  length, 
		VistaIOPointer data);

	/*! \brief Decode an attribute's value from a string to internal representation.
	 *
	 *  \param  str
	 *  \param  dict
	 *  \param  repn
	 *  \param  value
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIODecodeAttrValue (VistaIOStringConst str, VistaIODictEntry *dict,
					  VistaIORepnKind repn, VistaIOPointer value);

	/*! \brief Delete an attribute identified by its position in an attribute list.
	 *  
	 *  The posn parameter is updated to point to the next in the list.
	 * 
	 *  \param posn
	 */
	EXPORT_VISTA void VistaIODeleteAttr (VistaIOAttrListPosn *posn);

	/*! \brief Discard a list of attributes.
	 *
	 *  \param  list
	 */
	EXPORT_VISTA void VistaIODestroyAttrList (VistaIOAttrList list);

	/*! \brief Discard a bundle.
	 *
	 *  \param b
	 */	
	EXPORT_VISTA void VistaIODestroyBundle (VistaIOBundle b);

	/*! \brief Encode an attribute's value from internal representaiton to a string.
	 *  
	 *  This is just a stub for Encode, which is shared by VistaIOSetAttr.
	 *  The calling sequence is:
	 *
	 *	VistaIOEncodeAttrValue (VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations. It returns
	 *  a pointer to an encoded string, valid until the next VistaIOEncodeAttrValue
	 *  call.
	 *
	 *  \param  dict
	 *  \param  repn
	 *  \return  VistaIOStringConst
	 */
	EXPORT_VISTA VistaIOStringConst VistaIOEncodeAttrValue (VistaIODictEntry *dict, VistaIORepnKind repn, ...);

	/*! \brief Fetch an attribute value, removing it from its attribute list if found.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  dict
	 *  \param  repn 
	 *  \param  value
	 *  \param  required
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOExtractAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict,
		VistaIORepnKind repn, VistaIOPointer value, VistaIOBooleanPromoted required);

	/*! \brief Fetch an attribute value, given its name.
	 *
	 *  The value parameter should point to storage sufficient to hold a value
	 *  of the kind described by repn. An optional dictionary, dict, can
	 *  specify string -> value translations.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  dict
	 *  \param  repn
	 *  \param  value
	 *  \return VistaIOGetAttrResult
	 */
	EXPORT_VISTA VistaIOGetAttrResult VistaIOGetAttr (VistaIOAttrList list, VistaIOStringConst name, 
		VistaIODictEntry *dict, VistaIORepnKind repn, VistaIOPointer value);

	/*! \brief Fetch an attribute value given its position within an attribute list.
	 *
	 *  The value parameter should point to storage sufficient to hold a value
	 *  of the kind described by repn. An optional dictionary, dict, can
	 *  specify string -> value translations. 
	 *
	 *  \param  posn
	 *  \param  dict
	 *  \param  repn
	 *  \param  value
	 *  \return Returns TRUE if successful, FALSE
	 *          if the requested representation cannot be provided.
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOGetAttrValue (VistaIOAttrListPosn *posn, VistaIODictEntry *dict,
		VistaIORepnKind repn, VistaIOPointer value);

	/*! \brief Insert a new attribute into a list, before or after a specified position.
	 *
	 *  The calling sequence is:
	 *
	 *	VistaIOInsertAttr (VistaIOAttrListPosn *posn, VistaIOBoolean after, VistaIOStringConst name,
	 *		     VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations. If after is
	 *  TRUE the new attribute is inserted after posn; otherwise, before it.
	 *
	 *  \param posn
	 *  \param after
	 *  \param name
	 *  \param dict
	 *  \param repn
	 */
	EXPORT_VISTA void VistaIOInsertAttr (VistaIOAttrListPosn *posn, VistaIOBooleanPromoted after,
		VistaIOStringConst name, VistaIODictEntry *dict, VistaIORepnKind repn, ...);

	/*! \brief Lookup an attribute, by name, in a list of attributes.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  posn
	 *  \return VistaIOBoolean
	 */
	EXPORT_VISTA VistaIOBoolean VistaIOLookupAttr (VistaIOAttrList list, VistaIOStringConst name,
		VistaIOAttrListPosn *posn );

	/*! \brief Prepend a new attribute to a list.
	 *
	 *  The calling sequence is:
	 *
	 *	VistaIOPrependAttr (VistaIOAttrList list, VistaIOStringConst name,
	 *		      VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param list
	 *  \param name
	 *  \param dict
	 *  \param repn
	 */
	EXPORT_VISTA void VistaIOPrependAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict,
		VistaIORepnKind repn, ...);

	/*! \brief Set an attribute value, where the attribute is specified by name.
	 *
	 *  The calling sequence is:
	 *
	 *	VistaIOSetAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict,
	 *		  VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param list
	 *  \param name
	 *  \param dict
	 *  \param repn 
	 */
	EXPORT_VISTA void VistaIOSetAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict, 
		VistaIORepnKind repn, ...);

	/*! \brief Set an attribute value, where the attribute is specified by its
	 *  position in an attribute list.
	 *
	 *  The calling sequence is:
	 *
	 *	VistaIOSetAttrValue (VistaIOAttrListPosn *posn, VistaIODictEntry *dict,
	 *		       VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param posn
	 *  \param dict
	 *  \param repn
	 */
	EXPORT_VISTA void VistaIOSetAttrValue (VistaIOAttrListPosn *posn, VistaIODictEntry *dict, VistaIORepnKind repn,  ...);

	/*! \brief Look up an entry in an attribute value dictionary, by keyword.
	 *  
	 *  (It's assumed that dictionaries are pretty small -- a linear search
	 *  is done.)
	 *
	 *  \param  dict
	 *  \param  keyword
	 *  \return VistaIODictEntry
	 */
	EXPORT_VISTA VistaIODictEntry *VistaIOLookupDictKeyword (VistaIODictEntry *dict, VistaIOStringConst keyword);

	/*! \brief Look up an entry in an attribute dictionary, by value.
	 *
	 *  Calling sequence:
	 *
	 *	VistaIOLookupDictValue (VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
	 *
	 *  where xxx is a type that corresponds to repn.
	 *
	 *  \param  dict
	 *  \param  repn
	 *  \return VistaIODictEntry
	 */
	EXPORT_VISTA VistaIODictEntry *VistaIOLookupDictValue (VistaIODictEntry *dict, VistaIORepnKind repn, ...);

	/*! \brief Report a fatal program error.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VistaIOError (VistaIOStringConst format, ...);

	/*! \brief Report a non-fatal program error.
	 *
	 *  \param  format
	 */
	EXPORT_VISTA void VistaIOWarning (VistaIOStringConst format , ...);

	/*! \brief Report a application specific messages, but only if verbose equal 
	 *         or greater verbose level 1.
	 * 
	 *  \param format
	 */
	EXPORT_VISTA void VistaIOMessage (VistaIOStringConst format, ...);

	

	EXPORT_VISTA void VistaIOSetErrorHandler (VistaIOErrorHandler * fnc);
	/*! \brief Establish a caller-supplied routine as the handler for warning messages.
	 *
	 *  \param  fnc
	 */

	EXPORT_VISTA void VistaIOSetWarningHandler (VistaIOWarningHandler * fnc);

	
	/*! \brief Report a fatal error incurred by a system call.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VistaIOSystemError (VistaIOStringConst format, ...);

	/*! \brief Report non-fatal error incurred by a system call.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VistaIOSystemWarning (VistaIOStringConst format, ...);

	/*! \brief Convert an array of data elements from unpacked to packed form.
	 *
	 *  The elements to be packed are integers or floating point numbers, as
	 *  specified by repn. Each element's unpacked size is unpacked_elsize
	 *  bits, and its packed size is packed_elsize bits. There are nels of
	 *  them, beginning at unpacked. Packed_order specifies whether they are
	 *  to be packed from MSB to LSB (VMSBFirst), or vice versa (VLSBFirst).
	 *
	 *  To pack into a buffer already allocated:
	 *
	 *	length = length of buffer;
	 *	packed = address of buffer;
	 *	VistaIOPackData ( ..., & length, & packed, NULL);
	 *
	 *  and on return length will be set to the length of the packed data.
	 *
	 *  To pack into a buffer supplied by VistaIOPackData:
	 *
	 *	VistaIOPackData ( ..., & length, & packed, & alloced);
	 *
	 *  and on return length will be set to the length of the packed data,
	 *  packed will be set to point to it, and alloced will be TRUE if
	 *  the storage packed points to is a buffer that was allocated just for
	 *  the packed data. If alloced is FALSE, packed is pointing to the same
	 *  place as the unpacked data because the packed and unpacked forms are
	 *  identical.
	 *
	 *  These assumptions are made:
	 *    - packed_elsize is either 1 or a multiple of 8
	 *    - if packed_elsize is 1, then the unpacked data elements are VistaIOBits
	 *    - unpacked_elsize >= packed_elsize
	 *
	 *  \param  repn
	 *  \param  nels
	 *  \param  unpacked
	 *  \param  packed_order
	 *  \param  length
	 *  \param  packed
	 *  \param  alloced
	 *  \return  VistaIOBoolean
	 */
	VistaIOBoolean VistaIOPackData (VistaIORepnKind repn, size_t nels, VistaIOPointer unpacked, 
		VistaIOPackOrder packed_order, size_t *length, VistaIOPointer *packed, 
		VistaIOBoolean *alloced);

	/*! \brief Convert an array of data elements from packed to unpacked form.
	 *
	 *  The elements to be packed are integers or floating point numbers, as
	 *  specified by repn. Each element's unpacked size is unpacked_elsize
	 *  bits, and its packed size is packed_elsize bits. There are nels of
	 *  them, beginning at packed. Packed_order specifies whether they are
	 *  to be unpacked from MSB to LSB (VistaIOBigEndian), or vice versa (VistaIOLittleEndian).
	 *
	 *  To unpack into a buffer already allocated:
	 *
	 *	length = length of buffer;
	 *	unpacked = address of buffer;
	 *	VistaIOUnpackData ( ..., & length, & unpacked, NULL);
	 *
	 *  and on return length will be set to the length of the packed data.
	 *
	 *  To unpack into a buffer supplied by VistaIOUnpackData:
	 *
	 *	VistaIOUnpackData ( ..., & length, & unpacked, & alloced);
	 *
	 *  and on return length will be set to the length of the unpacked data,
	 *  unpacked will be set to point to it, and alloced will be TRUE if
	 *  the storage unpacked points to is a buffer that was allocated just for
	 *  the unpacked data. If alloced is FALSE, unpacked is pointing to the same
	 *  place as the packed data because the packed and unpacked forms are
	 *  identical.
	 *
	 *  These assumptions are made:
	 *    - packed_elsize is either 1 or a multiple of 8
	 *    - if packed_elsize is 1, then the unpacked data elements are VistaIOBits
	 *    - unpacked_elsize >= packed_elsize
	 *
	 *  \param  repn
	 *  \param  nels
	 *  \param  packed
	 *  \param  packed_order
	 *  \param  length
	 *  \param  unpacked
	 *  \param  alloced
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOUnpackData (VistaIORepnKind repn, size_t nels, VistaIOPointer packed, 
		VistaIOPackOrder packed_order, size_t *length, VistaIOPointer *unpacked, 
		VistaIOBoolean *alloced);

	/*! \brief Pack the low order bits of consecutive VistaIOBit data elements.
	 *  
	 *  unpacked and packed can point to the same place.
	 *
	 *  \param  nels
	 *  \param  packed_order
	 *  \param  unpacked
	 *  \param  packed
	 */
	void VistaIOPackBits (size_t nels, VistaIOPackOrder packed_order, VistaIOBit *unpacked, 
		char *packed);

	/*! \brief Unpack into the low order bits of consecutive VistaIOBit data elements.
	 *  
	 *  packed and unpacked can point to the same place.
	 *
	 *  \param  nels
	 *  \param  packed_order
	 *  \param  packed
	 *  \param  unpacked
	 */
	void VistaIOUnpackBits (size_t nels, VistaIOPackOrder packed_order, char *packed, 
		VistaIOBit *unpacked);

	/*! \brief Register some handlers for dealing with objects of a particular type.
	 *  
	 *  \param  name
	 *  \param  methods
	 *  \return Returns the VistaIORepnKind code assigned the new type.
	 */
	VistaIORepnKind VistaIORegisterType (VistaIOStringConst name, VistaIOTypeMethods *methods);

	/*! \brief Locate information about a named type.
	 *
	 *  \param  name
	 *  \return VistaIORepnKind
	 */
	VistaIORepnKind VistaIOLookupType (VistaIOStringConst name);

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
	VistaIOEdges VistaIOCreateEdges (int nrows, int ncolumns, int nedge_fields, int npoint_fields);

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
	VistaIOEdge VistaIOAddEdge (VistaIOEdges edges, VistaIOFloat *edge_fields, int npoints, 
		VistaIOFloat *points, VistaIOBooleanPromoted closed, VistaIOBooleanPromoted copy);

	/*! \brief Copy a VistaIOEdges object.
	 *
	 *  \param  src
	 *  \return VistaIOEdges
	 */
	VistaIOEdges VistaIOCopyEdges (VistaIOEdges src);

	/*! \brief Frees memory occupied by set of edges.
	 *
	 *  \param edges
	 */
	void VistaIODestroyEdges (VistaIOEdges edges);

	/*! \brief Read a Vista data file, extract the edge sets from it, 
	 *         and return a list of them.
	 *  
	 *  \param  file
	 *  \param  attributes
	 *  \param  edge_sets
	 */
	int VistaIOReadEdges (FILE *file, VistaIOAttrList *attributes, VistaIOEdges **edge_sets);

	/*! \brief Write a list of edge sets to a Vista data file.
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nedge_sets
	 *  \param  edge_sets
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOWriteEdges (FILE *file, VistaIOAttrList attributes, int nedge_sets, 
		VistaIOEdges *edge_sets);

	/*! \brief Default error handler.
	 *  
	 *  \param msg
	 */
	EXPORT_VISTA void VistaIODefaultError (VistaIOStringConst msg);

	/*! \brief Default warning handler.
	 *
	 *  \param msg
	 */
	EXPORT_VISTA void VistaIODefaultWarning (VistaIOStringConst msg);

	/*! \brief create a new volume list and return a ptr to it.
	 *  
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncols
	 *  \return Volumes
	 */
	Volumes VistaIOCreateVolumes (short nbands, short nrows, short ncols);

	/*! \brief copy a list a volumes
	 *  
	 *  \param  src
	 *  \return Volumes
	 */
	Volumes VistaIOCopyVolumes (Volumes src);

	/*! \brief destroy a set of volumes
	 *
	 *  \param volumes
	 */
	void VistaIODestroyVolumes (Volumes volumes);

	/*! \brief
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nvolumes
	 *  \param  volumes
	 *  \return VistaIOBoolean
	 */
	VistaIOBoolean VistaIOWriteVolumes (FILE *file, VistaIOAttrList attributes, int nvolumes, 
		Volumes *volumes);

	/*! \brief
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  volumes
	 *  \return int
	 */
	int VistaIOReadVolumes (FILE *file, VistaIOAttrList *attributes, Volumes **volumes);

	/*! \brief create a single volume and initialize it, i,e. allocate space for 
	 *         its hashtable.
	 *
	 *  \param  label
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncolumns
	 *  \param  nbuckets
	 *  \return Volume
	 */
	Volume VistaIOCreateVolume (short label, short nbands, short nrows, short ncolumns, 
		short nbuckets);

	/*! \brief copy src volume to dest volume
	 *
	 *  \param  src
	 *  \return Volume
	 */
	Volume VistaIOCopyVolume (Volume src);

	/*! \brief append a volume to the end of a volume list
	 *
	 *  \param  volumes
	 *  \param  vol
	 */
	void VistaIOAddVolume (Volumes volumes, Volume vol);

	/*! \brief Add a new track to a hashtable while keeping the bucket sorted,
	 *  
	 *  The track must have been created prior to the call.
	 * 
	 *  \param  v
	 *  \param  t
	 */
	void AddTrack (Volume v, VistaIOTrack t);

	/*! \brief Note the program name for use in error messages.
	 *
	 *  \param name
	 */
	EXPORT_VISTA void VistaIOSetProgramName (VistaIOStringConst name);
	
#ifdef __cplusplus
}
#endif

#endif	/* TYPES_H */
