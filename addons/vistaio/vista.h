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

#define VMax(a,b)		((a) > (b) ? (a) : (b))
#define VMin(a,b)		((a) < (b) ? (a) : (b))
#define VOffset(type, field) 	((size_t) (((char *) & ((type) 0)->field) - (char *) 0))
#define VOffsetOf(type, field)	VOffset(type *, field)
#define VNumber(array)		((size_t) (sizeof (array) / sizeof ((array)[0])))
#define VZero(array, nels) 	((void) memset ((void *) array, 0, (size_t) (nels) * sizeof ((array)[0])))
#define VCopy(from, to, nels) 	((void) memcpy ((void *) (to), (void *) (from), \
		    			(size_t) (nels) * sizeof ((from)[0])))
#define VNew(type)		((type *) VMalloc (sizeof (type)))
#define VNewString(str) 	((VString) ((str) ? strcpy ((char *) VMalloc (strlen (str) + 1), str) : 0))
#define VFileHeader		"V-data"
#define VFileVersion		2
#define VFileDelimiter		"\f\n"
#define VMaxAttrNameLength	256
#define VRequiredOpt		(& V_RequiredOpt)
#define VOptionalOpt		(& V_OptionalOpt)
#define VAllBands		-1	/* all bands */
#define VAttrListEmpty(l)	((l) == NULL || (l)->next == NULL)
#define VFirstAttr(l,p)		((void) ((p)->list = (l), (p)->ptr = (l)->next))
#define VLastAttr(l,p)		((void) ((p)->list = (l), (p)->ptr = (l)->prev))
#define VAttrExists(p)		((p)->ptr != NULL)
#define VNextAttr(p)		((void) ((p)->ptr = (p)->ptr ? (p)->ptr->next : NULL))
#define VPrevAttr(p)		((void) ((p)->ptr = (p)->ptr ? (p)->ptr->prev : NULL))
#define VGetAttrName(p)		((p)->ptr->name)
#define VGetAttrRepn(p)		((p)->ptr->repn)
#define VListCount(vlist)	((vlist)->count)
#define VListCurr(vlist)	((vlist)->current->item)
#define VListGetCurr(vlist)	((vlist)->current)
#define VListSetCurr(vlist,cur) ((void)((vlist)->current = (cur)))
#define VImageNBands(image)	((image)->nbands)
#define VImageNRows(image)	((image)->nrows)
#define VImageNColumns(image)	((image)->ncolumns)
#define VImageNFrames(image)	((image)->nframes)
#define VImageNViewpoints(image) ((image)->nviewpoints)
#define VImageNColors(image)	((image)->ncolors)
#define VImageNComponents(image) ((image)->ncomponents)
#define VPixelRepn(image)	((image)->pixel_repn)
#define VImageData(image)	((image)->data)
#define VImageAttrList(image)	((image)->attributes)
#define VImageNPixels(image) 	((image)->nbands * (image)->nrows * (image)->ncolumns)
#define VPixelSize(image)	(VRepnSize ((image)->pixel_repn))
#define VPixelPrecision(image)  (VRepnPrecision ((image)->pixel_repn))
#define VPixelRepnName(image)	(VRepnName ((image)->pixel_repn))
#define VPixelMinValue(image)	(VRepnMinValue ((image)->pixel_repn))
#define VPixelMaxValue(image)	(VRepnMaxValue ((image)->pixel_repn))
#define VImageSize(image)	(VImageNPixels(image) * VPixelSize(image))
#define VPixelPtr(image, band, row, column) \
				((VPointer) ((char *) ((image)->band_index[band][row])+(column) * VPixelSize (image)))
#define VPixel(image, band, row, column, type) \
				(* ((type *) (image)->band_index[band][row] + (column)))
#define VPixelArray(image, type) ((type ***) (image)->band_index)
#define VBandIndex(image, frame, viewpoint, color, component) \
				(((((frame) * (image)->nviewpoints + (viewpoint)) * (image)->ncolors + \
					(color)) * (image)->ncomponents) + (component))
#define VSameImageRange(image1, image2)	\
				((image1)->nbands == (image2)->nbands && (image1)->nrows == (image2)->nrows && \
				(image1)->ncolumns == (image2)->ncolumns && (image1)->pixel_repn == (image2)->pixel_repn)
#define VSameImageSize(image1, image2) \
				((image1)->nbands == (image2)->nbands && (image1)->nrows == (image2)->nrows && \
				(image1)->ncolumns == (image2)->ncolumns)
#define VGraphNNodes(graph)	(graph->nnodes)
#define VGraphNFields(graph)	(graph->nfields)
#define VGraphNSize(graph)	(graph->size)
#define VGraphAttrList(graph)	(graph->attributes)
#define VGraphGetNode(graph, nid)	(graph->table[nid-1])
#define VGraphNodeIsFree(graph, nid)	(graph->table[nid-1] == 0)
#define VNodeRepn(graph)	(graph->node_repn)
#define VNodeSize(graph) 	(sizeof(VNodeBaseRec) + (graph->nfields * VRepnPrecision(graph->node_repn)) / 8)
#define VNodeTestVisit(node)	(((VNodeBase)node)->visited == TRUE)
#define VNodeSetVisit(node)	(((VNodeBase)node)->visited = TRUE)
#define VNodeClearVisit(node)	(((VNodeBase)node)->visited = FALSE)

#define VEdgesNRows(edges)	((edges)->nrows)
#define VEdgesNColumns(edges)	((edges)->ncolumns)
#define VEdgesAttrList(edges)	((edges)->attributes)
#define VNEdgeFields(edges)	((edges)->edge_fields)
#define VNPointFields(edges)	((edges)->npoints)
#define VNEdges(edges)		((edges)->nedges)
#define VFirstEdge(edges)	((edges)->first)
#define VNextEdge(edge)		((edge)->next)
#define VEdgeExists(edge)	((edge) != NULL)
#define VEdgeFields(edge)	((edge)->edge_fields)
#define VEdgeNPoints(edge)	((edge)->npoints)
#define VEdgeClosed(edge)	((edge)->closed)
#define VEdgePointArray(edge)	((edge)->point_index)

/* Following are old macro names which should no longer be used.
   They can be removed in a future version once all of the documentation
   is in place and has been announced. */
#define VEdgesCount(edges)	((edges)->nedges)
#define VEdgePoints(edge)	((edge)->point_index)
#define VEdgesEdgeFields(edges) ((edges)->nedge_fields)
#define VEdgesPointFields(edges) ((edges)->npoint_fields)
#define VEdgesRows(edges)	((edges)->nrows)
#define VEdgesColumns(edges)	((edges)->ncolumns)
#define VEdgePointCount(edge)	((edge)->npoints)

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
#define VFirstVolume(volumes) ((volumes)->first)
#define VNextVolume(volume) ((volume)->next)
#define VolumeExists(volume) ((volume) != NULL)

#define VTrackLength(track) ((track)->length)
#define VTrackExists(track) ((track) != NULL)
#define VFirstTrack(volume,i) ((volume)->bucket[(i)].first)
#define VNextTrack(track) ((track)->next)
#define VPreviousTrack(track) ((track)->previous)

/* Names of generic attributes: */
#define VCommentAttr		"comment"
#define VDataAttr		"data"
#define VHistoryAttr		"history"
#define VLengthAttr		"length"
#define VNameAttr		"name"
#define VNColumnsAttr		"ncolumns"
#define VNRowsAttr		"nrows"
#define VRepnAttr               "repn"

/* Image attribute type names: */
#define VColorInterpAttr	"color_interp"
#define VComponentInterpAttr	"component_interp"
#define VFrameInterpAttr	"frame_interp"
#define VNBandsAttr		"nbands"
#define VNColorsAttr		"ncolors"
#define VNComponentsAttr	"ncomponents"
#define VNFramesAttr		"nframes"
#define VNViewpointsAttr	"nviewpoints"
#define VPixelAspectRatioAttr	"pixel_aspect_ratio"
#define VViewpointInterpAttr	"viewpoint_interp"

/* Graph attribute type names: */
#define VGraphAttr		"Graph"
#define VNGraphNodesAttr	"nnodes"
#define VNGraphSizeAttr		"size"
#define VNNodeFieldsAttr	"nfields"
#define VNNodeWeightsAttr	"useWeights"

/* Edge type names: */
#define VEdgesAttr		"edges"
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
#define VBitConst(c)			(c)
#define VUByteConst(c)		(c)
#define VSByteConst(c)		(c)
#define VShortConst(c)		(c)
#define VLongConst(c)		(c ## l)
#define VFloatConst(c)		(c ## f)
#define VDoubleConst(c) 	(c)

/* (These definitions may be platform-specific.) */
typedef char VBit;		/*!< 0 or 1 */
typedef double VDouble;		/*!< >= 64-bit IEEE floating point */
typedef float VFloat;		/*!< >= 32-bit IEEE floating point */
typedef int VLong;		/*!< !! changed, G.L. 19.9.95 !! */
typedef signed char VSByte;	/*!< integer in [-128,127] */
typedef short VShort;		/*!< >= 16-bit signed integer */
typedef unsigned char VUByte;	/*!< integer in [0,255] */
typedef char VBoolean;		/*!< TRUE or FALSE */
typedef void *VPointer;		/*!< generic pointer */
typedef const void *VPointerConst;		/*!< generic pointer */
typedef const char *VStringConst;	/* null-terminated string constant */
typedef char *VString;		/*!< null-terminated string */
typedef int VBitPromoted;
typedef int VBooleanPromoted;
typedef double VDoublePromoted;
typedef double VFloatPromoted;
typedef long VLongPromoted;
typedef int VSBytePromoted;
typedef int VShortPromoted;
typedef unsigned int VUBytePromoted;
typedef struct V_ImageRec *VImage;
typedef int VBand;
typedef void VErrorHandler (VStringConst);
typedef void VWarningHandler (VStringConst);
typedef VPointer VCopyMethod (VPointer);
typedef void VDestroyMethod (VPointer);

extern VBoolean V_RequiredOpt, V_OptionalOpt;

/*! \brief Codes for referring to representations: */
typedef enum {
	VUnknownRepn,
	VBitRepn,		/*!< 1-bit integer, [0, 1] */
	VUByteRepn,		/*!< 8-bit integer, [0, 255] */
	VSByteRepn,		/*!< 8-bit integer, [-128, 127] */
	VShortRepn,		/*!< 16-bit integer, [-32768, 32767] */
	VLongRepn,		/*!< 32-bit integer, [-2**31, 2**31-1] */
	VFloatRepn,		/*!< 32-bit IEEE floating point */
	VDoubleRepn,		/*!< 64-bit IEEE floating point */
	VAttrListRepn,		/*!< attribute list */
	VBooleanRepn,		/*!< TRUE or FALSE */
	VBundleRepn,		/*!< object of named type */
	VListRepn,		/*!< list of opaque objects */
	VPointerRepn,		/*!< pointer to opaque object */
	VStringRepn,		/*!< null-terminated string */
	VEdgesRepn,		/*!< edge set */
	VImageRepn,		/*!< image */
	VGraphRepn,		/*!< graph */
	VolumesRepn,		/*!< volumes */
	VCPEListRepn,           /*!< list of critical points */ 
	VField3DRepn,           /*!< A 3D field of 3D Vectors */
	VField2DRepn,           /*!< A 3D field of 3D Vectors */
	VNRepnKinds		/*!< number of predefined types */
} VRepnKind;

/*! \brief  Values of band interpretation attributes: 
 *  
 *  Returns information about how an image's bands are to be interpreted
 */
typedef enum {
	VBandInterpNone,	/*!< no interpretation specified */
	VBandInterpOther,	/*!< unknown interpretation specified */
	VBandInterpStereoPair,
	VBandInterpRGB,
	VBandInterpComplex,
	VBandInterpGradient,
	VBandInterpIntensity,
	VBandInterpOrientation
} VBandInterp;

/*! \brief Dictionary entry: */
typedef struct {
	/* The following are initialized by the dictionary provider: */
	
	VStringConst keyword;	/*!< keyword string */
	VLong ivalue;		/*!< value, if an integer */
	VStringConst svalue;	/*!< value, if a string */

	/* The following are used only by code in VLookupDictValue: */
	VBoolean icached;	/*!< whether integer value cached */
	VBoolean fcached;	/*!< whether float value cached */
	VDouble fvalue;		/*!< cached floating-point value */
} VDictEntry;

/*! \brief Accepted command options are described by a table of these entries */
typedef struct {
	VStringConst keyword;	/*!< keyword signalling option */
	VRepnKind repn; 	/*!< type of value supplied */
	int number; 		/*!< number of values supplied*/
	VPointer value; 	/*!< location for storing value(s) */
	VBoolean *found; 	/*!< whether optionl arguments */
	VDictEntry *dict;	/*!< optional dictionary of value */
	VStringConst blurb;	/*!< online help blurb */
} VOptionDescRec;

/*! \brief If an option takes multiple values, they are represented by 
 *         a VArgVector: 
 */
typedef struct {
	int number;		/*!< number of arguments */
	VPointer vector;	/*!< vector of arguments */
} VArgVector;

/*! \brief Each attribute name/value pair is represented by: */
typedef struct V_AttrRec {
	struct V_AttrRec *next;	/*!< next in list */
	struct V_AttrRec *prev;	/*!< previous in list */
	VRepnKind repn;		/*!< rep'n of attribute value */
	VPointer value;		/*!< pointer to attribute value */
	char name[1];		/*!< beginning of name string */
} VAttrRec;

typedef VAttrRec *VAttrList;

/*! \brief Position within a list of attributes: */
typedef struct {
	VAttrList list;		/*!< the list */
	struct V_AttrRec *ptr;	/*!< position within the list */
} VAttrListPosn;

/*! \brief Result of trying to retrieve an attribute's value: */
typedef enum {
	VAttrFound,		/*!< successfully retrieved value */
	VAttrMissing,		/*!< didn't find attribute */
	VAttrBadValue		/*!< incompatible value */
} VGetAttrResult;

/*! \brief An object whose type is named but not registered: */
typedef struct {
	VAttrList list;		/*!< object's attribute list value */
	size_t length;		/*!< length of binary data */
	VPointer data;		/*!< pointer to binary data */
	char type_name[1];	/*!< beginning of object's type's name */
} VBundleRec, *VBundle;

typedef VPointer VDecodeMethod (VStringConst, VBundle);
typedef VAttrList VEncodeAttrMethod (VPointer, size_t *);
typedef VPointer VEncodeDataMethod (VPointer, VAttrList, size_t, VBoolean *);

/*! \brief Set of methods supporting an object type: */
typedef struct {
	VCopyMethod *copy;
	VDestroyMethod *destroy;
	VDecodeMethod *decode;
	VEncodeAttrMethod *encode_attr;
	VEncodeDataMethod *encode_data;
} VTypeMethods;

/*! \brief Information about a representation: */
typedef struct {
	VStringConst name;	/*!< name string */
	size_t size;		/*!< size, in bytes */
	int precision;		/*!< precision, in bits */
	VDouble min_value;	/*!< min and max representable values */
	VDouble max_value;
	VTypeMethods *methods;	/*!< associated methods */
} VRepnInfoRec;

/*! \brief List element: */
typedef struct V_Node *VNodePtrType;
/*! \brief List element: */
struct V_Node {
	VPointer item;		/*!< pointer to data item */
	VNodePtrType prev;	/*!< pointer to previous node */
	VNodePtrType next;	/*!< pointer to next node */
};

/*! \brief List head: */
typedef struct V_List {
	VNodePtrType current;	/*!< pointer to current node */
	VNodePtrType head;	/*!< pointer to head node */
	VNodePtrType tail;	/*!< pointer to tail node */
	int count;		/*!< number of nodes in VList */
} *VList;

/*! \brief Description of an image: */
typedef struct V_ImageRec {
	int nbands;		/*!< number of bands */
	int nrows;		/*!< number of rows */
	int ncolumns;		/*!< number of columns */
	VRepnKind pixel_repn;	/*!< representation of pixel values */
	unsigned long flags;	/*!< various flags */
	VAttrList attributes;	/*!< list of other image attributes */
	VPointer data;		/*!< array of image pixel values */
	VPointer *row_index;	/*!< ptr to first pixel of each row */
	VPointer **band_index;	/*!< ptr to first row of each band */
	int nframes;		/*!< number of motion frames */
	int nviewpoints;	/*!< number of camera viewpoints */
	int ncolors;		/*!< number of color channels */
	int ncomponents;	/*!< number of vector components */
} VImageRec;

/*! \brief Codes for flags: */
enum {
	VImageSingleAlloc = 0x01	/*!< one free() releases everything */
};

/*! \brief Description of a Graph 
 *  
 *  Vista  represents  a  graph as a list of connected nodes. Nodes and
 *  connections may have weights, connections may be uni-  or  bidirec-
 *  tional in a graph.  The actual representation of a node is achieved
 *  by subclassing from a VNodeBase structure, which implements the i/o
 *  and  bookkeeping part.  Subclasses of VNodeBase may implement f.ex.
 *  a vertex by adding x, y, and z coordinates or polygons by recording
 *  a  list  of vertex references.  Besides node data, a graph may also
 *  has an arbitrary list of attributes associated with it.
 *
 *  A graph in memory is referred to by the C pointer type  VGraph.  In
 *  data  files it is identified by the type name graph, and in memory,
 *  by the VRepnKind code VGraphRepn. Since it  is  a  standard  object
 *  type with built-in support in the Vista library, graphs can be read
 *  from data files, written to data files, and manipulated as part  of
 *  attribute   lists  by  routines  such  as  VReadFile(3Vi),  VWrite-
 *  File(3Vi), and VGetAttr(3Vi). 
 */
typedef struct V_GraphRec {
	int nnodes;		/*!< number of nodes */
	int nfields;		/*!< size of fields in a node´s private area */
	VRepnKind node_repn;	/*!< data representation in a node */
	VAttrList attributes;	/*!< list of other attributes */
	struct VNodestruct **table;	/*!< node table of Graph */
	int size;		/*!< number of places in table */
	int lastUsed;		/*!< last entry used in table */
	int iter;		/*!< iteration counter in sequential access */
	int useWeights;		/*!< TRUE iff weights are used */
} VGraphRec, *VGraph;

/*! \brief Description of the base of a node */
typedef struct VNodebaseStruct {
	unsigned int hops:31;	/*!< number of hops in this node */
	unsigned int visited:1;	/*!< true if seen before */
	VFloat weight;		/*!< weight of this node */
	struct VAdjstruct *head;
} VNodeBaseRec, *VNodeBase;

/*! \brief Description of a node */
typedef struct VNodestruct {
	VNodeBaseRec base;
	char data[1];		/*!< private data area of node starts here */
} VNodeRec, *VNode;

/*! \brief Information about adjacency of nodes */
typedef struct VAdjstruct {
	unsigned int id;	/*!< node reference */
	VFloat weight;		/*!< weight of this node */
	struct VAdjstruct *next;	/* list of adjacent nodes */
} VAdjRec, *VAdjacency;

/*! \brief General information about an edge set
 *
 *  A  Vista  edge  set  is typically used to represent two-dimensional
 *  connected edge points or lines that have been  extracted  from  im-
 *  ages,  but  it can also be used to represent any sets of vectors of
 *  floating point values.
 *
 *  An edge set in memory is referred to by the C pointer type  VEdges.
 *  In  data files it's identified by the type name edges, and in memo-
 *  ry, by the VRepnKind code VEdgesRepn. Since it is a standard object
 *  type  with  built-in support in the Vista library, edge sets can be
 *  read from data files, written to data  files,  and  manipulated  as
 *  part of attribute lists by routines such as VReadFile(3Vi), VWrite-
 *  File(3Vi), and VGetAttr(3Vi).
 *								     
 *  A single edge, a member of an edge set, is referred  to  by  the  C
 *  pointer type VEdge.
 */
typedef struct V_EdgesRec {
	int nrows;		/*!< number of rows */
	int ncolumns;		/*!< number of columns */
	VAttrList attributes;	/*!< list of other attributes */
	int nedge_fields;	/*!< number of fields in each edge record */
	int npoint_fields;	/*!< number of fields in each point record */
	int nedges;		/*!< number of edges */
	int npoints;		/*!< total number of points */
	struct VEdgeStruct *first;	/*!< first edge in linked list of edges */
	struct VEdgeStruct *last;	/*!< last edge in linked list of edges */
	VPointer free;		/*!< free this storage when destroying edges */
} VEdgesRec, *VEdges;

/*! \brief Topological information about an edge set. */
typedef struct VEdgeStruct {
	struct VEdgeStruct *next;	/*!< next edge in linked list of edges */
	VFloat *edge_fields;	/*!< vector of field entries for this edge */
	VBoolean closed;	/*!< indicates closed edge (a loop) */
	int npoints;		/*!< number of points in this edge */
	VFloat **point_index;	/*!< pointers to start of each point */
	VPointer free;		/*!< free this storage when destroying edges */
} VEdgeRec, *VEdge;

/*! \brief Description of a track */
typedef struct VTrackStruct {
	short band;
	short row;
	short col;
	short length;
	struct VTrackStruct *next;
	struct VTrackStruct *previous;
} *VTrack, VTrackRec;

/*! \brief Information needed to handel tracks */
typedef struct VBucketStruct {
	short ntracks;		/*!< number of tracks in one hashtable bucket */
	VTrack first;		/*!< ptr to first track in bucket             */
	VTrack last;		/*!< ptr to last track in bucket              */
} *VBucket, VBucketRec;

/*! \brief Description of a volume */
typedef struct VolumeStruct {
	short label;
	short nbands;
	short nrows;
	short ncolumns;
	short nbuckets;		/*!< length of hash table (number of buckets) */
	int ntracks;		/*!< total number of tracks in all buckets   */
	VBucket bucket;		/*!< ptrs to buckets      */
	struct VolumeStruct *next;
} VolumeRec, *Volume;

/*! \brief Description of a set of volume */
typedef struct V_VolumesRec {
	VAttrList attributes;
	short nvolumes;		/*!< number of volumes in list       */
	short nbands;
	short nrows;
	short ncolumns;
	Volume first;		/*!< ptr to first volume in list     */
} VolumesRec, *Volumes;

#define MAXHASHLEN 1024		/*!< max length of hash table */


/* A list of attributes is represented by a header node: */
typedef enum { VLsbFirst, VMsbFirst } VPackOrder;
typedef VBoolean VReadFileFilterProc (VBundle, VRepnKind);

/* Macros for accessing information about representations: */
#define VRepnSize(repn)			(VRepnInfo[repn].size)
#define VRepnPrecision(repn)	(VRepnInfo[repn].precision)
#define VRepnName(repn)		(VRepnInfo[repn].name)
#define VRepnMinValue(repn)	(VRepnInfo[repn].min_value)
#define VRepnMaxValue(repn)	(VRepnInfo[repn].max_value)
#define VRepnMethods(repn)	(VRepnInfo[repn].methods)
#define VIsIntegerRepn(repn)	((repn) >= VBitRepn && (repn) <= VLongRepn)
#define VIsFloatPtRepn(repn)	((repn) == VFloatRepn || (repn) == VDoubleRepn)
#define VRandomDouble()		((VDouble) drand48 ())
#define VRandomSeed(seed)	srand48 ((long) seed)
#define VRandomLong()			((VLong) mrand48 ())

/*  Declarations of library routines. */
#ifdef __cplusplus
extern "C" {
#endif

  EXPORT_VISTA extern VRepnInfoRec *VRepnInfo;
  extern EXPORT_VISTA VDictEntry VBooleanDict[];	/*!< boolean values */
  extern EXPORT_VISTA VDictEntry VNumericRepnDict[];	/*!< numeric representation kinds */
  extern EXPORT_VISTA VDictEntry VBandInterpDict[];

	typedef void (*VShowProgressFunc)(int pos, int length, void *data);
	
	extern int VERBOSE;
	
	/*! \brief a function to set a file io progress indicator
	 *  \param   show_read the callback used on read
	 *  \param   show_write the callback used on write
	 *  \param   data some data that is passed to the callback 
	 */
	EXPORT_VISTA void VSetProgressIndicator(VShowProgressFunc show_read, VShowProgressFunc show_write, void *data);

	/*! \brief A function to reaset the progress callback to the standart behavior (be quiet) */
	EXPORT_VISTA void VResetProgressIndicator(void);
	
	/*! \brief The function to create a VGraph. 
	 *
	 *  \param size number of nodes in the Graph
	 *  \param nfields number of fields in a node
	 *  \param repn type of fields in the node
	 *  \param use_weight whether the weight is used in the graph nodes
	 *  \returns a newly created Graph
	 */
	EXPORT_VISTA VGraph VCreateGraph (int size, int nfields , VRepnKind repn , int use_weight);
	
	/*! \brief Copy a VGraph object.
	 *
	 *  Note that no compaction is performed, since this would require
	 *  a recalculation of all indices,
	 *
	 *  \param  src
	 *  \return VGraph
	 */
	EXPORT_VISTA VGraph VCopyGraph (VGraph src);

	/*! \brief Frees memory occupied by a graph.
	 *
	 *  \param graph
	 */
	EXPORT_VISTA void VDestroyGraph (VGraph graph);

	/*! \brief Read a Vista data file, extract the graphs from it, and return 
	 *         a list of them.
	 *
	 *  \param  file
	 *  \param  attrs
	 *  \param  graphs
	 *  \return int
	 */
	EXPORT_VISTA int VReadGraphs (FILE *file, VAttrList *attrs, VGraph **graphs);

	/*! \brief Write a list of graphs to a Vista data file.
	 * 
	 *  \param  file
	 *  \param  attrs
	 *  \param  n
	 *  \param  graphs
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VWriteGraphs (FILE *file, VAttrList attrs, int n, VGraph *graphs);

	/*! \brief Find a node in a Vista graph structure.
	 *  
	 *  \param  graph
	 *  \param  node
	 *  \return Return reference to this node.
	 */
	EXPORT_VISTA int VGraphLookupNode (VGraph graph, VNode node);

	/*! \brief Add a node to a Vista graph structure.
	 *  
	 *  \param  graph
	 *  \param  node
	 *  \return Return reference to this node.
	 */
	EXPORT_VISTA int VGraphAddNode (VGraph graph, VNode node);

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
	EXPORT_VISTA int VGraphAddNodeAt (VGraph graph, VNode node, int position);

	/*! \brief Make a link between to nodes.
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return Return TRUE if successful.
	 */
	EXPORT_VISTA int VGraphLinkNodes (VGraph graph, int a, int b);

	/*! \brief unlinks two nodes.
	 *
	 *  Return TRUE if successful.
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return int
	 */
	EXPORT_VISTA int VGraphUnlinkNodes (VGraph graph, int a, int b);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return VPointer
	 */
	EXPORT_VISTA VPointer VGraphFirstNode (VGraph graph);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return VPointer
	 */
	EXPORT_VISTA VPointer VGraphNextNode (VGraph graph);

	/*! \brief
	 *
	 *  \param  graph
	 */	
	EXPORT_VISTA void VGraphClearVisit (VGraph graph);

	/*! \brief Visits all node in a graph connected to node i
	 *
	 *  \param  graph
	 *  \param  i
	 *  \return int
	 */
	EXPORT_VISTA int VGraphVisitNodesFrom (VGraph graph, int i);

	/*! \brief Grow private data area of each node to newfields.
	 *  
	 *  \param  graph
	 *  \param  newfields
	 *  \return Return TRUE if successful.
	 */
	EXPORT_VISTA int VGraphResizeFields (VGraph graph, int newfields);

	/*! \brief
	 *
	 *  \param  graph
	 *  \return Returns number of cycles in a graph
	 */
	EXPORT_VISTA int VGraphNCycles (VGraph graph);

	/*! \brief Visits all node in a graph connected to node i and toggles the hops
	 *  field note that the visit field must have been cleared before
	 *
	 *  \param  graph
	 *  \param  i
	 */
	EXPORT_VISTA void VGraphToggleNodesFrom (VGraph graph, int i);

	/*! \brief make bidrectional link between nodes a and b
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 */
	EXPORT_VISTA void VGraphLinkNodesBi (VGraph graph, VLong a, VLong b);

	/*! \brief remove bidrectional link between nodes a and b
	 * 
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 */
	EXPORT_VISTA void VGraphUnlinkNodesBi (VGraph graph, VLong a, VLong b);

	/*! \brief
	 *
	 *  \param  graph
	 *  \param  a
	 *  \param  b
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VGraphHasLink (VGraph graph, int a, int b);

	/*! \brief complex deletion: look at all connected structures of this node
	 *
	 *  \param  graph
	 *  \param  i
	 */
	EXPORT_VISTA void VDestroyNode (VGraph graph, int i);

	/*! \brief Destroys nodes from a graph in which the hops field is set
	 *
	 *  \param graph
	 *  \param i
	 */
	EXPORT_VISTA void VGraphDestroyNodesFrom (VGraph graph, int i);

	/*! \brief Clears the hops field in a graph
	 *  
	 *  \param  graph
	 */
	EXPORT_VISTA void VGraphClearHops (VGraph graph);

	/*! \brief Identify the files specified by command line arguments.
	 *
	 *  Files can be provided in any of three ways:
	 *   (1) as arguments to a switch, -keyword (e.g, -in file1 file2)
	 *   (2) as additional command line arguments, not associated with
	 *       any switch (e.g., vview file1 file2)
	 *   (3) by piping to/from stdin or stdout (e.g., vview < file1).
	 *
	 *  VIdentifyFiles collects file names from these three sources once
	 *  VParseCommand has been called to parse the command's switches.
	 *  It looks first for the keyword, then for unclaimed command line
	 *  arguments, and finally for a file or pipe attached to stdin or stdout.
	 *
	 *  \param  noptions
	 *  \param  options
	 *  \param  keyword
	 *  \param  argc
	 *  \param  argv
	 *  \param  fd
	 *  \return VBoolean
	 */
	VBoolean VIdentifyFiles (int noptions, VOptionDescRec options[], 
		VStringConst keyword, int *argc, char **argv, int fd);

	/*! \brief Parse command line arguments according to a table of option descriptors.
	 *  Unrecognized options are left in argv, and argc is adjusted to reflect
	 *  their number.
	 *  If an erroneous (as opposed to simply unrecognized) argument is
	 *  encountered, VParseCommand returns FALSE; otherwise, TRUE.
	 *
	 *  The -help option is recognized explicitly. If it is present, VParseCommand
	 *  returns indicating that all arguments were recognized, but that an error
	 *  occurred. This should force the caller to simply print usage information.
	 *
	 *  \param  noptions
	 *  \param  options
	 *  \param  argc
	 *  \param  argv
	 *  \return VBoolean
	 */
	VBoolean VParseCommand (int noptions, VOptionDescRec options[], int *argc, char **argv);

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
	void VParseFilterCmd (int noptions, VOptionDescRec opts[], int argc, 
		char **argv,  FILE **inp, FILE **outp);

	/*! \brief Print the settings of a set of command line options.
	 *
	 *  \param f
	 *  \param noptions
	 *  \param options
	 */
	void VPrintOptions (FILE *f, int noptions, VOptionDescRec options[]);

	/*! \brief Print the value of a specified option.
	 *
	 *  \param f
	 *  \param option
	 */
	int VPrintOptionValue (FILE *f, VOptionDescRec *option);

	/*! \brief Report the remaining command line arguments in argv as ones that could
	 *  not be recognized.
	 *
	 *  \param argc
	 *  \param argv
	 */
	void VReportBadArgs (int argc, char **argv);

	/*! \brief Print, to stderr, information about how to use a program based on
	 *  the contents of its command argument parsing table.
	 *
	 *  \param program
	 *  \param noptions
	 *  \param options
	 *  \param other_args
	 */
	void VReportUsage (VStringConst program, int noptions, 
		VOptionDescRec options[], VStringConst other_args);

	/*! \brief Print, to stderr, a summary of program options based on the contents of
	 *  a command argument parsing table
	 *
	 *  \param noptions
	 *  \param options
	 */
	void VReportValidOptions (int noptions, VOptionDescRec options[]);

	/*! \brief Allocates memory for a new image with specified properties.
	 *  
	 *  Returns a pointer to the image if successful, zero otherwise.
	 *  
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncolumns
	 *  \param  pixel_repn
	 *  \return VImage
	 */
	EXPORT_VISTA VImage VCreateImage (int nbands, int nrows, int ncolumns, VRepnKind pixel_repn);

	/*! \brief Create an image with the same properties as an existing one.
	 *
	 *  \param  src
	 *  \return VImage 
	 */
	EXPORT_VISTA VImage VCreateImageLike (VImage src);

	/*! \brief Frees memory occupied by an image.
	 *
	 *  \param image
	 *  \return VImage
	 */
	EXPORT_VISTA void VDestroyImage (VImage image);

	/*! \brief Fetch a pixel value, regardless of type, and return it as a Double.
	 *
	 *  \param  image
	 *  \param  band
	 *  \param  row
	 *  \param  column
	 *  \return VDouble
	 */
	EXPORT_VISTA VDouble VGetPixel (VImage image, int band, int row, int column);

	/*! \brief Set a pixel, regardless of type, and to a value passed as a Double.
	 *
	 *  \param image
	 *  \param band
	 *  \param row
	 *  \param column
	 *  \param value
	 */
	EXPORT_VISTA void VSetPixel (VImage image, int band, int row, int column, 
		VDoublePromoted value);

	/*! \brief Copy the pixels and attributes of one image to another.
	 *
	 *  Returns a pointer to the destination image if successful, zero otherwise.
	 *  The band parameter may be VAllBands, in which case all bands of pixel
	 *  values are copied, or a particular band number, in which case only a
	 *  single band is copied to a 1-band destination image.
	 *
	 *  \param  src
	 *  \param  dest 
	 *  \param  band 
	 *  \return Returns a pointer to the destination image if successful, zero 
	 *          otherwise
	 */
	EXPORT_VISTA VImage VCopyImage (VImage src, VImage dest, VBand band);

	/*! \brief Give a destination image the same attributes as a source image.
	 *  
	 *  However if the destination image doesn't have the same number of bands
	 *  as the source image, any band interpretation attributes are deleted.
	 *
	 *  \param  src
	 *  \param  dest
	 *  \return VImage
	 */
	EXPORT_VISTA VImage VCopyImageAttrs (VImage src, VImage dest);

	/*! \brief Copy the pixels of one image to another.
	 *
	 *  Returns a pointer to the destination image if successful, zero otherwise.
	 *  The band parameter may be VAllBands, in which case all bands of pixel
	 *  values are copied, or a particular band number, in which case only a
	 *  single band is copied to a 1-band destination image.
	 *
	 *  \param  src
	 *  \param  dest
	 *  \param  band
	 *  \return Returns a pointer to the destination image if successful, 
	 *          zero otherwise.
	 */
	EXPORT_VISTA VImage VCopyImagePixels (VImage src, VImage dest, VBand band);

	/*! \brief Copy a band of pixel data from one image to another.
	 *
	 *  Band src_band of image src is copied to band dest_band of image dest.
	 *  The destination image must exist, having the same pixel representation
	 *  and size as the source image. Either src_band or dst_band may be
	 *  VAllBands, provided they both describe the same number of bands.
	 *
	 *  \param  src
	 *  \param  src_band
	 *  \param  dest
	 *  \param  dest_band
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VCopyBand (VImage src, VBand src_band, VImage dest, VBand dest_band);

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
	 *  \return VImage
	 */
	EXPORT_VISTA VImage VCombineBands (int nels, VImage src_images[], VBand src_bands[], 
		VImage dest);

	/*! \brief A varargs version of VCombineBands. 
	 *
	 *  It is called by:
	 *
	 *	dest = VCombineBandsVa (dest, src_image1, src_band1, ...,
	 *				(VImage) NULL);
	 *  \param  dest
	 *  \return VImage
	 */
	EXPORT_VISTA VImage VCombineBandsVa (VImage dest, ...);

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
	 *  \return VImage
	 */
	EXPORT_VISTA VImage VSelectDestImage (VStringConst routine, VImage dest, int nbands, 
		int nrows, int ncolumns,	VRepnKind pixel_repn);

	/*! \brief Check a band specification and use it to determine the number and
	 *         address of a block of pixels.
	 *
	 *  \param  routine
	 *  \param  image
	 *  \param  band
	 *  \param  npixels
	 *  \param  first_pixel
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VSelectBand (VStringConst routine, VImage image, VBand band, 
		int *npixels, VPointer *first_pixel);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VBandInterp
	 */
	EXPORT_VISTA VBandInterp VImageFrameInterp (VImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VBandInterp
	 */ 
	EXPORT_VISTA VBandInterp VImageViewpointInterp (VImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VBandInterp
	 */
	EXPORT_VISTA VBandInterp VImageColorInterp (VImage image);

	/*! \brief Routine for accessing an image's band interpretation information.
	 * 
	 *  \param  image
	 *  \return VBandInterp
	 */
	EXPORT_VISTA VBandInterp VImageComponentInterp (VImage image);

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
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VSetBandInterp (VImage image, VBandInterp frame_interp, 
		int nframes, VBandInterp viewpoint_interp, int nviewpoints, 
		VBandInterp color_interp, int ncolors, VBandInterp component_interp, 
		int ncomponents);

	/*! \brief Read a Vista data file, extract the images from it, and return a 
	 *         list of them.
	 * 
	 *  \param  file
	 *  \param  attributes
	 *  \param  images
	 *  \return int
	 */
	EXPORT_VISTA int VReadImages (FILE *file, VAttrList *attributes, VImage **images);

	/*! \brief Write a list of images to a Vista data file.
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nimages
	 *  \param  images
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VWriteImages (FILE *file, VAttrList attributes, int nimages, 
		VImage images[]);

	/*! \brief Open an input or output file, with "-" representing stdin or stdout.
	 *  
	 *  \param  filename
	 *  \param  nofail
	 *  \return If nofail is TRUE, any failure is a fatal error.
	 */
	FILE *VOpenInputFile (VStringConst filename, VBoolean nofail);

	/*! \brief
	 *  
	 *  \param  filename
	 *  \param  nofail
	 *  \return FILE
	 */	
	FILE *VOpenOutputFile (VStringConst filename, VBoolean nofail);

	/*! \brief Read a Vista data file, extract object of a specified type, and 
	 *  return a vector of them plus a list of anything else found in the file.
	 *
	 *  \param  file
	 *  \param  repn
	 *  \param  attributes
	 *  \param  objects
	 *  \return int
	 */
	int VReadObjects (FILE *file, VRepnKind repn, VAttrList *attributes, VPointer **objects);

	/*! \brief Read a Vista data file, returning an attribute list of its contents.
	 *
	 *  \param  f
	 *  \param  filter
	 *  \return VAttrList
	 */
	EXPORT_VISTA VAttrList VReadFile (FILE * f, VReadFileFilterProc *filter);

	/*! \brief Write a list of objects, plus some other attributes, to a Vista data file.
	 *
	 *  \param  file
	 *  \param  repn
	 *  \param  attributes
	 *  \param  nobjects
	 *  \param  objects
	 *  \return VBoolean
	 */
	VBoolean VWriteObjects (FILE *file, VRepnKind repn, VAttrList attributes, 
		int nobjects, VPointer objects[]);

	/*! \brief VWriteFile
	 *  
	 *  \param  f
	 *  \param  list
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VWriteFile (FILE *f, VAttrList list);

	/*! \brief Make a new, empty list, and returns its reference.
	 *
	 *  \return VList
	 */
	VList VListCreate ();

	/*! \brief Return a pointer to the first item in vlist, 
	 *         and make the first item the current item.
	 *
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListFirst (VList vlist);

	/*! \brief Return a pointer to the last item in vlist,
	 *         and make the last item the current item.
	 *
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListLast (VList  vlist);

	/*! \brief Advance vlist's current item by one, return the
	 *         new current item. Return NULL if the new current
	 *         item is beyond the end of vlist.
	 *
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListNext (VList vlist);

	/*! \brief Back up vlist's current item by one, return the
	 *         new current item. Return NULL if the new current
	 *         item is before the beginning of vlist.
	 *
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListPrev (VList vlist);

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
	void VListAdd (VList vlist, VPointer item);

	/*! \brief Add item to vlist immediately before the current item, 
	 *
	 *  and make item the current item. If the current pointer is before 
	 *  the beginning of vlist, item is added at the beginning. If the current
	 *  pointer is beyond the end of vlist, item is added at the end.
	 *
	 *  \param  vlist
	 *  \param item
	 */	
	void VListInsert (VList vlist, VPointer item);

	/*! \brief Add item to the end of vlist, and make item the current item.
	 *
	 *  \param vlist
	 *  \param item
	 */
	EXPORT_VISTA void VListAppend (VList vlist, VPointer item);

	/*! \brief Add item to the beginning of vlist, and make
	 *         item the current item.
	 *
	 *  \param vlist
	 *  \param item 
	 */
	void VListPrepend (VList vlist, VPointer item);

	/*! \brief Return current item and take it out of vlist.
	 * 
	 *  Make the next item the current one.
	 *
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListRemove (VList vlist);

	/*! \brief Add vlist2 to the end of vlist1. 
	 *
	 *  The current pointer is set to the current pointer of vlist1.
	 *  vlist2 no longer exists after the operation.
	 *
	 *  \param  vlist1
	 *  \param  vlist2
	 */
	void VListConcat (VList vlist1, VList vlist2);

	/*! \brief Delete vlist. 
	 *
	 *  \param vlist
	 *  \param item_free A pointer to a routine that frees an item.
	 */
	void VListDestroy (VList vlist, void (*item_free)(VPointer));

	/*! \brief Return last item and take it out of vlist. 
	 *
	 *  Make the new last item the current one.
	 *  
	 *  \param  vlist
	 *  \return VPointer
	 */
	VPointer VListTrim (VList vlist);

	/*! \brief Searche vlist starting at the current item until the end is 
	 *         reached or a match is found.
	 *
	 *  \param  vlist
	 *  \param  comp
	 *  \param  comp_arg
	 *  \return VPointer
	 */
	VPointer VListSearch (VList vlist, int (*comp)(), VPointer comp_arg);

	/*! \brief Perform error checking on calloc() call.
	 *
	 *  \param  n
	 *  \param  size
	 *  \return VPointer
	 */
	VPointer VCalloc (size_t n, size_t size);

	/*! \brief Perform error checking on free() call.
	 *
	 *  \param p
	 */
	EXPORT_VISTA void VFree (VPointer p);

	/*! \brief Perform error checking on malloc() call.
	 *
	 *  \param  size
	 *  \return VPointer
	 */
	VPointer VMalloc (size_t size);

	/*! \brief Perform error checking on realloc() call.
	 *
	 *  \param  p
	 *  \param  size
	 *  \return VPointer
	 */
	VPointer VRealloc (VPointer p, size_t size);

	/*! \brief Append a new attribute to a list.
	 *
	 *  The calling sequence is:
	 *
	 *	VAppendAttr (VAttrList list, VStringConst name,
	 *		     VDictEntry *dict, VRepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  dict
	 *  \param  repn
	 */
	EXPORT_VISTA void VAppendAttr (VAttrList list, VStringConst name, VDictEntry *dict, 
		VRepnKind repn , ...);

	/*! \brief Make a copy of an attribute list.
	 * 
	 *  \param  list 
	 *  \return VAttrList
	 */
	EXPORT_VISTA VAttrList VCopyAttrList (VAttrList list);

	/*! \brief Create an attribute list. */
	EXPORT_VISTA VAttrList VCreateAttrList ();

	/*! \brief Create a bundle object.
	 *
	 *  \param  type_name
	 *  \param  list
	 *  \param  length
	 *  \param  data
	 *  \return VBundle
	 */
	EXPORT_VISTA VBundle VCreateBundle (VStringConst type_name, VAttrList list, size_t  length, 
		VPointer data);

	/*! \brief Decode an attribute's value from a string to internal representation.
	 *
	 *  \param  str
	 *  \param  dict
	 *  \param  repn
	 *  \param  value
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VDecodeAttrValue (VStringConst str, VDictEntry *dict,
					  VRepnKind repn, VPointer value);

	/*! \brief Delete an attribute identified by its position in an attribute list.
	 *  
	 *  The posn parameter is updated to point to the next in the list.
	 * 
	 *  \param posn
	 */
	EXPORT_VISTA void VDeleteAttr (VAttrListPosn *posn);

	/*! \brief Discard a list of attributes.
	 *
	 *  \param  list
	 */
	EXPORT_VISTA void VDestroyAttrList (VAttrList list);

	/*! \brief Discard a bundle.
	 *
	 *  \param b
	 */	
	EXPORT_VISTA void VDestroyBundle (VBundle b);

	/*! \brief Encode an attribute's value from internal representaiton to a string.
	 *  
	 *  This is just a stub for Encode, which is shared by VSetAttr.
	 *  The calling sequence is:
	 *
	 *	VEncodeAttrValue (VDictEntry *dict, VRepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations. It returns
	 *  a pointer to an encoded string, valid until the next VEncodeAttrValue
	 *  call.
	 *
	 *  \param  dict
	 *  \param  repn
	 *  \return  VStringConst
	 */
	EXPORT_VISTA VStringConst VEncodeAttrValue (VDictEntry *dict, VRepnKind repn, ...);

	/*! \brief Fetch an attribute value, removing it from its attribute list if found.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  dict
	 *  \param  repn 
	 *  \param  value
	 *  \param  required
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VExtractAttr (VAttrList list, VStringConst name, VDictEntry *dict,
		VRepnKind repn, VPointer value, VBooleanPromoted required);

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
	 *  \return VGetAttrResult
	 */
	EXPORT_VISTA VGetAttrResult VGetAttr (VAttrList list, VStringConst name, 
		VDictEntry *dict, VRepnKind repn, VPointer value);

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
	EXPORT_VISTA VBoolean VGetAttrValue (VAttrListPosn *posn, VDictEntry *dict,
		VRepnKind repn, VPointer value);

	/*! \brief Insert a new attribute into a list, before or after a specified position.
	 *
	 *  The calling sequence is:
	 *
	 *	VInsertAttr (VAttrListPosn *posn, VBoolean after, VStringConst name,
	 *		     VDictEntry *dict, VRepnKind repn, xxx value)
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
	EXPORT_VISTA void VInsertAttr (VAttrListPosn *posn, VBooleanPromoted after,
		VStringConst name, VDictEntry *dict, VRepnKind repn, ...);

	/*! \brief Lookup an attribute, by name, in a list of attributes.
	 *
	 *  \param  list
	 *  \param  name
	 *  \param  posn
	 *  \return VBoolean
	 */
	EXPORT_VISTA VBoolean VLookupAttr (VAttrList list, VStringConst name,
		VAttrListPosn *posn );

	/*! \brief Prepend a new attribute to a list.
	 *
	 *  The calling sequence is:
	 *
	 *	VPrependAttr (VAttrList list, VStringConst name,
	 *		      VDictEntry *dict, VRepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param list
	 *  \param name
	 *  \param dict
	 *  \param repn
	 */
	EXPORT_VISTA void VPrependAttr (VAttrList list, VStringConst name, VDictEntry *dict,
		VRepnKind repn, ...);

	/*! \brief Set an attribute value, where the attribute is specified by name.
	 *
	 *  The calling sequence is:
	 *
	 *	VSetAttr (VAttrList list, VStringConst name, VDictEntry *dict,
	 *		  VRepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param list
	 *  \param name
	 *  \param dict
	 *  \param repn 
	 */
	EXPORT_VISTA void VSetAttr (VAttrList list, VStringConst name, VDictEntry *dict, 
		VRepnKind repn, ...);

	/*! \brief Set an attribute value, where the attribute is specified by its
	 *  position in an attribute list.
	 *
	 *  The calling sequence is:
	 *
	 *	VSetAttrValue (VAttrListPosn *posn, VDictEntry *dict,
	 *		       VRepnKind repn, xxx value)
	 *
	 *  where xxx depends on the kind of representation, repn. An optional
	 *  dictionary, dict, can specify value -> string translations.
	 *
	 *  \param posn
	 *  \param dict
	 *  \param repn
	 */
	EXPORT_VISTA void VSetAttrValue (VAttrListPosn *posn, VDictEntry *dict, VRepnKind repn,  ...);

	/*! \brief Look up an entry in an attribute value dictionary, by keyword.
	 *  
	 *  (It's assumed that dictionaries are pretty small -- a linear search
	 *  is done.)
	 *
	 *  \param  dict
	 *  \param  keyword
	 *  \return VDictEntry
	 */
	EXPORT_VISTA VDictEntry *VLookupDictKeyword (VDictEntry *dict, VStringConst keyword);

	/*! \brief Look up an entry in an attribute dictionary, by value.
	 *
	 *  Calling sequence:
	 *
	 *	VLookupDictValue (VDictEntry *dict, VRepnKind repn, xxx value)
	 *
	 *  where xxx is a type that corresponds to repn.
	 *
	 *  \param  dict
	 *  \param  repn
	 *  \return VDictEntry
	 */
	EXPORT_VISTA VDictEntry *VLookupDictValue (VDictEntry *dict, VRepnKind repn, ...);

	/*! \brief Report a fatal program error.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VError (VStringConst format, ...);

	/*! \brief Report a non-fatal program error.
	 *
	 *  \param  format
	 */
	EXPORT_VISTA void VWarning (VStringConst format , ...);

	/*! \brief Report a application specific messages, but only if verbose equal 
	 *         or greater verbose level 1.
	 * 
	 *  \param format
	 */
	EXPORT_VISTA void VMessage (VStringConst format, ...);

	

	EXPORT_VISTA void VSetErrorHandler (VErrorHandler * fnc);
	/*! \brief Establish a caller-supplied routine as the handler for warning messages.
	 *
	 *  \param  fnc
	 */

	EXPORT_VISTA void VSetWarningHandler (VWarningHandler * fnc);

	
	/*! \brief Report a fatal error incurred by a system call.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VSystemError (VStringConst format, ...);

	/*! \brief Report non-fatal error incurred by a system call.
	 *
	 *  \param format
	 */
	EXPORT_VISTA void VSystemWarning (VStringConst format, ...);

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
	 *	VPackData ( ..., & length, & packed, NULL);
	 *
	 *  and on return length will be set to the length of the packed data.
	 *
	 *  To pack into a buffer supplied by VPackData:
	 *
	 *	VPackData ( ..., & length, & packed, & alloced);
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
	 *    - if packed_elsize is 1, then the unpacked data elements are VBits
	 *    - unpacked_elsize >= packed_elsize
	 *
	 *  \param  repn
	 *  \param  nels
	 *  \param  unpacked
	 *  \param  packed_order
	 *  \param  length
	 *  \param  packed
	 *  \param  alloced
	 *  \return  VBoolean
	 */
	VBoolean VPackData (VRepnKind repn, size_t nels, VPointer unpacked, 
		VPackOrder packed_order, size_t *length, VPointer *packed, 
		VBoolean *alloced);

	/*! \brief Convert an array of data elements from packed to unpacked form.
	 *
	 *  The elements to be packed are integers or floating point numbers, as
	 *  specified by repn. Each element's unpacked size is unpacked_elsize
	 *  bits, and its packed size is packed_elsize bits. There are nels of
	 *  them, beginning at packed. Packed_order specifies whether they are
	 *  to be unpacked from MSB to LSB (VBigEndian), or vice versa (VLittleEndian).
	 *
	 *  To unpack into a buffer already allocated:
	 *
	 *	length = length of buffer;
	 *	unpacked = address of buffer;
	 *	VUnpackData ( ..., & length, & unpacked, NULL);
	 *
	 *  and on return length will be set to the length of the packed data.
	 *
	 *  To unpack into a buffer supplied by VUnpackData:
	 *
	 *	VUnpackData ( ..., & length, & unpacked, & alloced);
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
	 *    - if packed_elsize is 1, then the unpacked data elements are VBits
	 *    - unpacked_elsize >= packed_elsize
	 *
	 *  \param  repn
	 *  \param  nels
	 *  \param  packed
	 *  \param  packed_order
	 *  \param  length
	 *  \param  unpacked
	 *  \param  alloced
	 *  \return VBoolean
	 */
	VBoolean VUnpackData (VRepnKind repn, size_t nels, VPointer packed, 
		VPackOrder packed_order, size_t *length, VPointer *unpacked, 
		VBoolean *alloced);

	/*! \brief Pack the low order bits of consecutive VBit data elements.
	 *  
	 *  unpacked and packed can point to the same place.
	 *
	 *  \param  nels
	 *  \param  packed_order
	 *  \param  unpacked
	 *  \param  packed
	 */
	void VPackBits (size_t nels, VPackOrder packed_order, VBit *unpacked, 
		char *packed);

	/*! \brief Unpack into the low order bits of consecutive VBit data elements.
	 *  
	 *  packed and unpacked can point to the same place.
	 *
	 *  \param  nels
	 *  \param  packed_order
	 *  \param  packed
	 *  \param  unpacked
	 */
	void VUnpackBits (size_t nels, VPackOrder packed_order, char *packed, 
		VBit *unpacked);

	/*! \brief Register some handlers for dealing with objects of a particular type.
	 *  
	 *  \param  name
	 *  \param  methods
	 *  \return Returns the VRepnKind code assigned the new type.
	 */
	VRepnKind VRegisterType (VStringConst name, VTypeMethods *methods);

	/*! \brief Locate information about a named type.
	 *
	 *  \param  name
	 *  \return VRepnKind
	 */
	VRepnKind VLookupType (VStringConst name);

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
	VEdges VCreateEdges (int nrows, int ncolumns, int nedge_fields, int npoint_fields);

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
	VEdge VAddEdge (VEdges edges, VFloat *edge_fields, int npoints, 
		VFloat *points, VBooleanPromoted closed, VBooleanPromoted copy);

	/*! \brief Copy a VEdges object.
	 *
	 *  \param  src
	 *  \return VEdges
	 */
	VEdges VCopyEdges (VEdges src);

	/*! \brief Frees memory occupied by set of edges.
	 *
	 *  \param edges
	 */
	void VDestroyEdges (VEdges edges);

	/*! \brief Read a Vista data file, extract the edge sets from it, 
	 *         and return a list of them.
	 *  
	 *  \param  file
	 *  \param  attributes
	 *  \param  edge_sets
	 */
	int VReadEdges (FILE *file, VAttrList *attributes, VEdges **edge_sets);

	/*! \brief Write a list of edge sets to a Vista data file.
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nedge_sets
	 *  \param  edge_sets
	 *  \return VBoolean
	 */
	VBoolean VWriteEdges (FILE *file, VAttrList attributes, int nedge_sets, 
		VEdges *edge_sets);

	/*! \brief Default error handler.
	 *  
	 *  \param msg
	 */
	EXPORT_VISTA void VDefaultError (VStringConst msg);

	/*! \brief Default warning handler.
	 *
	 *  \param msg
	 */
	EXPORT_VISTA void VDefaultWarning (VStringConst msg);

	/*! \brief create a new volume list and return a ptr to it.
	 *  
	 *  \param  nbands
	 *  \param  nrows
	 *  \param  ncols
	 *  \return Volumes
	 */
	Volumes VCreateVolumes (short nbands, short nrows, short ncols);

	/*! \brief copy a list a volumes
	 *  
	 *  \param  src
	 *  \return Volumes
	 */
	Volumes VCopyVolumes (Volumes src);

	/*! \brief destroy a set of volumes
	 *
	 *  \param volumes
	 */
	void VDestroyVolumes (Volumes volumes);

	/*! \brief
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  nvolumes
	 *  \param  volumes
	 *  \return VBoolean
	 */
	VBoolean VWriteVolumes (FILE *file, VAttrList attributes, int nvolumes, 
		Volumes *volumes);

	/*! \brief
	 *
	 *  \param  file
	 *  \param  attributes
	 *  \param  volumes
	 *  \return int
	 */
	int VReadVolumes (FILE *file, VAttrList *attributes, Volumes **volumes);

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
	Volume VCreateVolume (short label, short nbands, short nrows, short ncolumns, 
		short nbuckets);

	/*! \brief copy src volume to dest volume
	 *
	 *  \param  src
	 *  \return Volume
	 */
	Volume VCopyVolume (Volume src);

	/*! \brief append a volume to the end of a volume list
	 *
	 *  \param  volumes
	 *  \param  vol
	 */
	void VAddVolume (Volumes volumes, Volume vol);

	/*! \brief Add a new track to a hashtable while keeping the bucket sorted,
	 *  
	 *  The track must have been created prior to the call.
	 * 
	 *  \param  v
	 *  \param  t
	 */
	void AddTrack (Volume v, VTrack t);

	/*! \brief Note the program name for use in error messages.
	 *
	 *  \param name
	 */
	EXPORT_VISTA void VSetProgramName (VStringConst name);
	
#ifdef __cplusplus
}
#endif

#endif	/* TYPES_H */
