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
static VistaIOCopyMethod VistaIOImageCopyMethod;
static VistaIODecodeMethod VistaIOImageDecodeMethod;
static VistaIOEncodeAttrMethod VistaIOImageEncodeAttrMethod;
static VistaIOEncodeDataMethod VistaIOImageEncodeDataMethod;

/* Used in Type.c to register this type: */
VistaIOTypeMethods VistaIOImageMethods = {
	VistaIOImageCopyMethod,	/* copy a VistaIOImage */
	(VistaIODestroyMethod *) VistaIODestroyImage,	/* destroy a VistaIOImage */
	VistaIOImageDecodeMethod,	/* decode a VistaIOImage's value */
	VistaIOImageEncodeAttrMethod,	/* encode a VistaIOImage's attr list */
	VistaIOImageEncodeDataMethod	/* encode a VistaIOImage's binary data */
};


/*
 *  VistaIOImageCopyMethod
 *
 *  The "copy" method registered for the "image" type.
 *  Copy a VistaIOImage object.
 */

static VistaIOPointer VistaIOImageCopyMethod (VistaIOPointer value)
{
	return VistaIOCopyImage ((VistaIOImage) value, NULL, VistaIOAllBands);
}


/*
 *  VistaIOImageDecodeMethod
 *
 *  The "decode" method registered for the "image" type.
 *  Convert an attribute list plus binary data to a VistaIOImage object.
 */

static VistaIOPointer VistaIOImageDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	VistaIOImage image;
	VistaIOLong nbands, nrows, ncolumns, pixel_repn;
	VistaIOLong nframes, nviewpoints, ncolors, ncomponents;
	VistaIOAttrList list;
	size_t length;

#define Extract(name, dict, locn, required)	\
	VistaIOExtractAttr (b->list, name, dict, VistaIOLongRepn, & locn, required)

	/* Extract the number of bands, rows, columns, pixel repn, etc.: */
	nbands = nframes = nviewpoints = ncolors = ncomponents = 1;	/* defaults */
	if (!Extract (VistaIONBandsAttr, NULL, nbands, FALSE) ||
	    !Extract (VistaIONRowsAttr, NULL, nrows, TRUE) ||
	    !Extract (VistaIONColumnsAttr, NULL, ncolumns, TRUE) ||
	    !Extract (VistaIORepnAttr, VistaIONumericRepnDict, pixel_repn, TRUE) ||
	    !Extract (VistaIONFramesAttr, NULL, nframes, FALSE) ||
	    !Extract (VistaIONViewpointsAttr, NULL, nviewpoints, FALSE) ||
	    !Extract (VistaIONColorsAttr, NULL, ncolors, FALSE) ||
	    !Extract (VistaIONComponentsAttr, NULL, ncomponents, FALSE))
		return NULL;

	/* Ensure that nbands == nframes * nviewpoints * ncolors * ncomponents.
	   For backwards compatibility, set ncomponents to nbands if nbands != 1
	   but nframes == nviewpoints == ncolors == ncomponents == 1. */
	if (nbands != nframes * nviewpoints * ncolors * ncomponents) {
		if (nbands != 1 && nframes == 1 && nviewpoints == 1 &&
		    ncolors == 1 && ncomponents == 1)
			ncomponents = nbands;
		else {
			VistaIOWarning ("VistaIOImageDecodeMethod: %s image has inconsistent nbands", name);
			return NULL;
		}
	}

	/* Create an image with the specified properties: */
	if (!(image = VistaIOCreateImage ((int)nbands, (int)nrows, (int)ncolumns,
				    (VistaIORepnKind) pixel_repn)))
		return NULL;
	VistaIOImageNFrames (image) = nframes;
	VistaIOImageNViewpoints (image) = nviewpoints;
	VistaIOImageNColors (image) = ncolors;
	VistaIOImageNComponents (image) = ncomponents;

	/* Give it whatever attributes remain: */
	list = VistaIOImageAttrList (image);
	VistaIOImageAttrList (image) = b->list;
	b->list = list;

	/* Check that the expected amount of binary data was read: */
	length = VistaIOImageNPixels (image);
	if (VistaIOPixelRepn (image) == VistaIOBitRepn)
		length = (length + 7) / 8;
	else
		length *= VistaIOPixelPrecision (image) / 8;
	if (length != b->length) {
		VistaIOWarning ("VistaIOImageDecodeMethod: %s image has wrong data length", name);
	      Fail:VistaIODestroyImage (image);
		return NULL;
	}

	/* Unpack the binary pixel data: */
	length = VistaIOImageSize (image);
	if (!VistaIOUnpackData (VistaIOPixelRepn (image), VistaIOImageNPixels (image),
			  b->data, VistaIOMsbFirst, &length, &VistaIOImageData (image),
			  NULL))
		goto Fail;
	return image;

#undef Extract
}


/*
 *  VistaIOImageEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "image" type.
 *  Encode an attribute list value for a VistaIOImage object.
 */

static VistaIOAttrList VistaIOImageEncodeAttrMethod (VistaIOPointer value, size_t * lengthp)
{
	VistaIOImage image = value;
	VistaIOAttrList list;
	size_t length;

#define OptionallyPrepend(value, name)				\
	if (value != 1)							\
	    VistaIOPrependAttr (list, name, NULL, VistaIOLongRepn, (VistaIOLong) value)

	/* Temporarily prepend several attributes to the image's attribute list: */
	if ((list = VistaIOImageAttrList (image)) == NULL)
		list = VistaIOImageAttrList (image) = VistaIOCreateAttrList ();
	VistaIOPrependAttr (list, VistaIORepnAttr, VistaIONumericRepnDict, VistaIOLongRepn,
		      (VistaIOLong) VistaIOPixelRepn (image));
	VistaIOPrependAttr (list, VistaIONColumnsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) VistaIOImageNColumns (image));
	VistaIOPrependAttr (list, VistaIONRowsAttr, NULL, VistaIOLongRepn,
		      (VistaIOLong) VistaIOImageNRows (image));
	OptionallyPrepend (VistaIOImageNComponents (image), VistaIONComponentsAttr);
	OptionallyPrepend (VistaIOImageNColors (image), VistaIONColorsAttr);
	OptionallyPrepend (VistaIOImageNViewpoints (image), VistaIONViewpointsAttr);
	OptionallyPrepend (VistaIOImageNFrames (image), VistaIONFramesAttr);
	OptionallyPrepend (VistaIOImageNBands (image), VistaIONBandsAttr);

	/* Compute the file space needed for the image's binary data: */
	length = VistaIOImageNPixels (image);
	if (VistaIOPixelRepn (image) == VistaIOBitRepn)
		length = (length + 7) / 8;
	else
		length *= VistaIOPixelPrecision (image) / 8;
	*lengthp = length;

	return list;

#undef OptionallyPrepend
}


/*
 *  VistaIOImageEncodeDataMethod
 *
 *  The "encode_data" method registered for the "image" type.
 *  Encode the pixel values for a VistaIOImage object.
 */

static VistaIOPointer VistaIOImageEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list,
					size_t length, VistaIOBoolean * free_itp)
{
	VistaIOImage image = value;
	VistaIOAttrListPosn posn;
	size_t len;
	VistaIOPointer ptr;

	/* Remove the attributes prepended by the VistaIOImageEncodeAttrsMethod: */
	for (VistaIOFirstAttr (list, &posn);
	     strcmp (VistaIOGetAttrName (&posn), VistaIORepnAttr) != 0;
	     VistaIODeleteAttr (&posn));
	VistaIODeleteAttr (&posn);

	/* Pack and return pixel data: */
	if (!VistaIOPackData (VistaIOPixelRepn (image), VistaIOImageNPixels (image),
			VistaIOImageData (image), VistaIOMsbFirst, &len, &ptr, free_itp))
		return NULL;
	if (len != length)
		VistaIOError ("VistaIOImageEncodeDataMethod: Encoded data has unexpected length");
	return ptr;
}
