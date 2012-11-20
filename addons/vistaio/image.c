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
 *  Dictionaries for VistaIOImage attributes and their values.
 */

/* Keywords for representing band interpretation values: */
VistaIODictEntry VistaIOBandInterpDict[] = {
	{"complex", VistaIOBandInterpComplex}
	,
	{"gradient", VistaIOBandInterpGradient}
	,
	{"intensity", VistaIOBandInterpIntensity}
	,
	{"orientation", VistaIOBandInterpOrientation}
	,
	{"rgb", VistaIOBandInterpRGB}
	,
	{"stereo_pair", VistaIOBandInterpStereoPair}
	,
	{NULL}
};


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

EXPORT_VISTA VistaIOImage VistaIOCreateImage (int nbands, int nrows, int ncolumns,
		     VistaIORepnKind pixel_repn)
{
	size_t row_size = ncolumns * VistaIORepnSize (pixel_repn);
	size_t data_size = nbands * nrows * row_size;
	size_t row_index_size = nbands * nrows * sizeof (char *);
	size_t band_index_size = nbands * sizeof (char **);
	size_t pixel_size;
	char *p;
	VistaIOImage image;
	int band, row;

#define AlignUp(v, b) ((((v) + (b) - 1) / (b)) * (b))

	/* Check parameters: */
	if (nbands < 1) {
		VistaIOWarning ("VistaIOCreateImage: Invalid number of bands: %d",
			  (int)nbands);
		return NULL;
	}
	if (nrows < 1) {
		VistaIOWarning ("VistaIOCreateImage: Invalid number of rows: %d",
			  (int)nrows);
		return NULL;
	}
	if (ncolumns < 1) {
		VistaIOWarning ("VistaIOCreateImage: Invalid number of columns: %d",
			  (int)ncolumns);
		return NULL;
	}
	if (pixel_repn != VistaIOBitRepn && pixel_repn != VistaIOUByteRepn &&
	    pixel_repn != VistaIOSByteRepn && pixel_repn != VistaIOShortRepn &&
	    pixel_repn != VistaIOLongRepn && pixel_repn != VistaIOFloatRepn
	    && pixel_repn != VistaIODoubleRepn) {
		VistaIOWarning ("VistaIOCreateImage: Invalid pixel representation: %d",
			  (int)pixel_repn);
		return NULL;
	}

	/* Allocate memory for the VistaIOImage, its indices, and pixel values, while
	   padding enough to ensure pixel values are appropriately aligned: */
	pixel_size = VistaIORepnSize (pixel_repn);
	p = VistaIOMalloc (AlignUp (sizeof (VistaIOImageRec) + row_index_size +
			      band_index_size, pixel_size) + data_size);

	/* Initialize the VistaIOImage: */
	image = (VistaIOImage) p;
	image->nbands = nbands;
	image->nrows = nrows;
	image->ncolumns = ncolumns;
	image->flags = VistaIOImageSingleAlloc;
	image->pixel_repn = pixel_repn;
	image->attributes = VistaIOCreateAttrList ();
	image->band_index = (VistaIOPointer **) (p += sizeof (VistaIOImageRec));
	image->row_index = (VistaIOPointer *) (p += band_index_size);
	image->data =
		(VistaIOPointer) AlignUp ((long)p + row_index_size, pixel_size);
	image->nframes = nbands;
	image->nviewpoints = image->ncolors = image->ncomponents = 1;

	/* Initialize the indices: */
	for (band = 0; band < nbands; band++)
		image->band_index[band] = image->row_index + band * nrows;
	for (row = 0, p = image->data; row < nbands * nrows;
	     row++, p += row_size)
		image->row_index[row] = p;

	return image;

#undef AlignUp
}


/*! \brief Create an image with the same properties as an existing one.
 *
 *  \param  src
 *  \return VistaIOImage 
 */

EXPORT_VISTA VistaIOImage VistaIOCreateImageLike (VistaIOImage src)
{
	return VistaIOCopyImageAttrs (src, NULL);
}


/*! \brief Frees memory occupied by an image.
 *
 *  \param image
 *  \return VistaIOImage
 */

EXPORT_VISTA void VistaIODestroyImage (VistaIOImage image)
{
	if (!image)
		return;
	if (!(image->flags & VistaIOImageSingleAlloc)) {
		VistaIOFree (image->data);
		VistaIOFree ((VistaIOPointer) image->row_index);
		VistaIOFree ((VistaIOPointer) image->band_index);
	}
	VistaIODestroyAttrList (VistaIOImageAttrList (image));
	VistaIOFree ((VistaIOPointer) image);
}


/*! \brief Fetch a pixel value, regardless of type, and return it as a Double.
 *
 *  \param  image
 *  \param  band
 *  \param  row
 *  \param  column
 *  \return VistaIODouble
 */

VistaIODouble VistaIOGetPixel (VistaIOImage image, int band, int row, int column)
{
	VistaIOPointer p = VistaIOPixelPtr (image, band, row, column);

	switch (VistaIOPixelRepn (image)) {

	case VistaIOBitRepn:
		return (VistaIODouble) * (VistaIOBit *) p;

	case VistaIOUByteRepn:
		return (VistaIODouble) * (VistaIOUByte *) p;

	case VistaIOSByteRepn:
		return (VistaIODouble) * (VistaIOSByte *) p;

	case VistaIOShortRepn:
		return (VistaIODouble) * (VistaIOShort *) p;

	case VistaIOLongRepn:
		return (VistaIODouble) * (VistaIOLong *) p;

	case VistaIOFloatRepn:
		return (VistaIODouble) * (VistaIOFloat *) p;

	case VistaIODoubleRepn:
		return (VistaIODouble) * (VistaIODouble *) p;

	default:
		VistaIOError ("VistaIOGetPixel: %s images not supported",
			VistaIOPixelRepnName (image));
	}
	return 0.0;		/* to make lint happy */
}


/*! \brief Set a pixel, regardless of type, and to a value passed as a Double.
 *
 *  \param image
 *  \param band
 *  \param row
 *  \param column
 *  \param value
 */

void VistaIOSetPixel (VistaIOImage image, int band, int row, int column,
		VistaIODoublePromoted value)
{
	VistaIOPointer p = VistaIOPixelPtr (image, band, row, column);

	switch (VistaIOPixelRepn (image)) {

	case VistaIOBitRepn:
		*(VistaIOBit *) p = (VistaIOBit )value;
		break;

	case VistaIOUByteRepn:
		*(VistaIOUByte *) p = (VistaIOUByte )value;
		break;

	case VistaIOSByteRepn:
		*(VistaIOSByte *) p = (VistaIOSByte )value;
		break;

	case VistaIOShortRepn:
		*(VistaIOShort *) p = (VistaIOShort)value;
		break;

	case VistaIOLongRepn:
		*(VistaIOLong *) p = (VistaIOLong)value;
		break;

	case VistaIOFloatRepn:
		*(VistaIOFloat *) p = (VistaIOFloat)value;
		break;

	case VistaIODoubleRepn:
		*(VistaIODouble *) p = value;
		break;

	default:
		VistaIOError ("VistaIOSetPixel: %s images not supported",
			VistaIOPixelRepnName (image));
	}
}


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

EXPORT_VISTA VistaIOImage VistaIOCopyImage (VistaIOImage src, VistaIOImage dest, VistaIOBand band)
{
	VistaIOImage result;

	if (src == dest
	    && (band == VistaIOAllBands || (band == 0 && VistaIOImageNBands (src) == 1)))
		return src;

	if ((result = VistaIOCopyImagePixels (src, dest, band)) != 0)
		VistaIOCopyImageAttrs (src, result);
	return result;
}


/*! \brief Give a destination image the same attributes as a source image.
 *  
 *  However if the destination image doesn't have the same number of bands
 *  as the source image, any band interpretation attributes are deleted.
 *
 *  \param  src
 *  \param  dest
 *  \return VistaIOImage
 */

EXPORT_VISTA VistaIOImage VistaIOCopyImageAttrs (VistaIOImage src, VistaIOImage dest)
{
	VistaIOAttrList list;

	if (src == dest)
		return dest;
	if (!dest) {
		dest = VistaIOCreateImage (VistaIOImageNBands (src), VistaIOImageNRows (src),
				     VistaIOImageNColumns (src), VistaIOPixelRepn (src));
		if (!dest)
			return NULL;
	}

	/* Clone the source image's attribute list if it isn't empty: */
	if (!VistaIOAttrListEmpty (VistaIOImageAttrList (src))) {
		list = VistaIOImageAttrList (dest);
		VistaIOImageAttrList (dest) = VistaIOCopyAttrList (VistaIOImageAttrList (src));
	} else if (!VistaIOAttrListEmpty (VistaIOImageAttrList (dest))) {
		list = VistaIOImageAttrList (dest);
		VistaIOImageAttrList (dest) = VistaIOCreateAttrList ();
	} else
		list = NULL;
	if (list)
		VistaIODestroyAttrList (list);

	/* Preserve band interpretation attributes only if the source and
	   destination images have the same number of bands: */
	if (VistaIOImageNBands (src) > 1
	    && VistaIOImageNBands (dest) == VistaIOImageNBands (src)) {
		VistaIOImageNFrames (dest) = VistaIOImageNFrames (src);
		VistaIOImageNViewpoints (dest) = VistaIOImageNViewpoints (src);
		VistaIOImageNColors (dest) = VistaIOImageNColors (src);
		VistaIOImageNComponents (dest) = VistaIOImageNComponents (src);
	} else {
		VistaIOExtractAttr (VistaIOImageAttrList (dest), VistaIOFrameInterpAttr, NULL,
			      VistaIOBitRepn, NULL, FALSE);
		VistaIOExtractAttr (VistaIOImageAttrList (dest), VistaIOViewpointInterpAttr,
			      NULL, VistaIOBitRepn, NULL, FALSE);
		VistaIOExtractAttr (VistaIOImageAttrList (dest), VistaIOColorInterpAttr, NULL,
			      VistaIOBitRepn, NULL, FALSE);
		VistaIOExtractAttr (VistaIOImageAttrList (dest), VistaIOComponentInterpAttr,
			      NULL, VistaIOBitRepn, NULL, FALSE);
		VistaIOImageNComponents (dest) = VistaIOImageNColors (dest) =
			VistaIOImageNViewpoints (dest) = 1;
		VistaIOImageNFrames (dest) = VistaIOImageNBands (dest);
	}
	return dest;
}


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

EXPORT_VISTA VistaIOImage VistaIOCopyImagePixels (VistaIOImage src, VistaIOImage dest, VistaIOBand band)
{
	int npixels;
	VistaIOPointer src_pixels;
	VistaIOImage result;

	/* Locate the source and destination of the copy: */
	if (!VistaIOSelectBand
	    ("VistaIOCopyImagePixels", src, band, &npixels, &src_pixels))
		return NULL;
	result = VistaIOSelectDestImage ("VistaIOCopyImagePixels", dest,
				   band == VistaIOAllBands ? VistaIOImageNBands (src) : 1,
				   VistaIOImageNRows (src), VistaIOImageNColumns (src),
				   VistaIOPixelRepn (src));
	if (!result)
		return NULL;

	/* Copy pixel values from src to dest: */
	memcpy (VistaIOImageData (result), src_pixels, npixels * VistaIOPixelSize (src));

	return result;
}


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

EXPORT_VISTA VistaIOBoolean VistaIOCopyBand (VistaIOImage src, VistaIOBand src_band, VistaIOImage dest, VistaIOBand dest_band)
{
	int nbands, src_npixels, dest_npixels;
	VistaIOPointer src_pixels, dest_pixels;

	/* The destination image must exist: */
	if (!dest) {
		VistaIOWarning ("VistaIOCopyBand: No destination specified");
		return FALSE;
	}

	/* VistaIOAllBands not accepted for destination band: */
	if (dest_band < 0 || dest_band >= VistaIOImageNBands (dest)) {
		VistaIOWarning ("VistaIOCopyBand: Band %d referenced in image of %d bands", (int)dest_band, (int)VistaIOImageNBands (dest));
		return FALSE;
	}

	/* Ensure that the destination image has the appropriate dimensions
	   and pixel representation: */
	nbands = dest_band;
	if (src_band == VistaIOAllBands)
		nbands += VistaIOImageNBands (src) - 1;
	if (nbands < VistaIOImageNBands (dest))
		nbands = VistaIOImageNBands (dest);
	if (!VistaIOSelectDestImage ("VistaIOCopyBand", dest, nbands, VistaIOImageNRows (src),
			       VistaIOImageNColumns (src), VistaIOPixelRepn (src)))
		return FALSE;

	/* Locate the specified source and destination bands: */
	if (!VistaIOSelectBand
	    ("VistaIOCopyBand", src, src_band, &src_npixels, &src_pixels))
		return FALSE;
	if (!VistaIOSelectBand
	    ("VistaIOCopyBand", dest, dest_band, &dest_npixels, &dest_pixels))
		return FALSE;

	/* Copy from the source band to the destination band: */
	memcpy (dest_pixels, src_pixels, src_npixels * VistaIOPixelSize (src));

	return TRUE;
}


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
		      VistaIOImage dest)
{
	int n, i;
	VistaIOImage result, src = src_images[0];

	/* Count the number of bands needed in the destination image: */
	for (i = n = 0; i < nels; i++)
		n += (src_bands[i] ==
		      VistaIOAllBands) ? VistaIOImageNBands (src_images[i]) : 1;

	/* Check or allocate the destination image: */
	result = VistaIOSelectDestImage ("VistaIOCombineBands", dest, n,
				   VistaIOImageNRows (src), VistaIOImageNColumns (src),
				   VistaIOPixelRepn (src));
	if (!result)
		return NULL;

	/* Copy each source band into the destination image: */
	for (i = n = 0; i < nels; i++) {
		if (!VistaIOCopyBand (src_images[i], src_bands[i], result, n)) {
			if (result != dest)
				VistaIODestroyImage (result);
			return NULL;
		}
		n += (src_bands[i] ==
		      VistaIOAllBands) ? VistaIOImageNBands (src_images[i]) : 1;
	}
	return result;
}


/*! \brief A varargs version of VistaIOCombineBands. 
 *
 *  It is called by:
 *
 *	dest = VistaIOCombineBandsVa (dest, src_image1, src_band1, ...,
 *				(VistaIOImage) NULL);
 *  \param  dest
 *  \return VistaIOImage
 */

EXPORT_VISTA VistaIOImage VistaIOCombineBandsVa (VistaIOImage dest, ...)
{
	va_list args;
	VistaIOImage src, result;
	int nbands;
	VistaIOBand src_band, dest_band;

	/* Count the number of bands to be combined: */
	va_start (args, dest);
	for (nbands = 0; (src = va_arg (args, VistaIOImage)); nbands +=
	     (va_arg (args, VistaIOBand) == VistaIOAllBands) ? VistaIOImageNBands (src) : 1);
	va_end (args);

	/* Check or allocate the destination image: */
	va_start (args, dest);
	src = va_arg (args, VistaIOImage);
	va_end (args);
	result = VistaIOSelectDestImage ("VistaIOCombineBandsVa", dest, nbands,
				   VistaIOImageNRows (src), VistaIOImageNColumns (src),
				   VistaIOPixelRepn (src));
	if (!result)
		return NULL;

	/* Copy each source band into the destination image: */
	va_start (args, dest);
	for (dest_band = 0; (src = va_arg (args, VistaIOImage));) {
		src_band = va_arg (args, VistaIOBand);
		if (!VistaIOCopyBand (src, src_band, result, dest_band)) {
			if (result != dest)
				VistaIODestroyImage (result);
			return NULL;
		}
		dest_band += (src_band == VistaIOAllBands) ? VistaIOImageNBands (src) : 1;
	}
	va_end (args);
	return result;
}


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

/*  There are two ways to use this routine. If an operation is such that
 *  it can be carried out with a destination image that is the same as
 *  the source image, one follows this procedure:
 *
 *	VistaIOImage result;
 *
 *	result = VistaIOSelectDestImage (...);
 *	if (! result)
 *	    return NULL;
 *
 *	On successful completion:
 *      VistaIOCopyImageAttrs (src, result);
 *	return result;
 *
 *	On failure:
 *	if (result != dest)
 *	    VistaIODestroyImage (result);
 *	return NULL;
 *
 *  And if an operation *cannot* be carried out with a destination image
 *  that is the same as the source image, one does:
 *
 *	VistaIOImage result;
 *
 *	result = VistaIOSelectDestImage (...);	use or create dest image
 *	if (! result)
 *	    return NULL;
 *	if (src == result)
 *	    result = VistaIOCreateImage (...);	allocate a work image
 *
 *	On successful completion:
 *	if (src == dest) {
 *	    VistaIOCopyImagePixels (result, dest, VistaIOAllBands); move work to dest
 *	    VistaIODestroyImage (result);
 *	    return dest;
 *	} else {
 *	    VistaIOCopyImageAttrs (src, result);
 *	    return result;
 *      }
 *
 *	On failure:
 *	if (result != dest)
 *	    VistaIODestroyImage (result);
 *	return NULL;
 */

EXPORT_VISTA VistaIOImage VistaIOSelectDestImage (VistaIOStringConst routine, VistaIOImage dest,
			 int nbands, int nrows, int ncolumns,
			 VistaIORepnKind pixel_repn)
{
	/* If no destination image was specified, allocate one: */
	if (!dest)
		return VistaIOCreateImage (nbands, nrows, ncolumns, pixel_repn);

	/* Otherwise check that the destination provided has the appropriate
	   characteristics: */
	if (VistaIOImageNBands (dest) != nbands) {
		VistaIOWarning ("%s: Destination image has %d bands; %d expected",
			  routine, VistaIOImageNBands (dest), nbands);
		return NULL;
	}
	if (VistaIOImageNRows (dest) != nrows) {
		VistaIOWarning ("%s: Destination image has %d rows; %d expected",
			  routine, VistaIOImageNRows (dest), nrows);
		return NULL;
	}
	if (VistaIOImageNColumns (dest) != ncolumns) {
		VistaIOWarning ("%s: Destination image has %d columns; %d expected",
			  routine, VistaIOImageNColumns (dest), ncolumns);
		return NULL;
	}
	if (VistaIOPixelRepn (dest) != pixel_repn) {
		VistaIOWarning ("%s: Destination image has %s pixels; %s expected",
			  routine, VistaIOPixelRepnName (dest),
			  VistaIORepnName (pixel_repn));
		return NULL;
	}
	return dest;
}


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
		      int *npixels, VistaIOPointer * first_pixel)
{
	if (band == VistaIOAllBands) {
		if (npixels)
			*npixels = VistaIOImageNPixels (image);
		if (first_pixel)
			*first_pixel = VistaIOImageData (image);
	} else if (band >= 0 && band < VistaIOImageNBands (image)) {
		if (npixels)
			*npixels =
				VistaIOImageNRows (image) * VistaIOImageNColumns (image);
		if (first_pixel)
			*first_pixel = image->band_index[band][0];
	} else {
		VistaIOWarning ("%s: Band %d referenced in image of %d band(s)",
			  routine, band, VistaIOImageNBands (image));
		return FALSE;
	}
	return TRUE;
}


/*! \brief Routine for accessing an image's band interpretation information.
 * 
 *  \param  image
 *  \return VistaIOBandInterp
 */

/*
 *  VistaIOImageFrameInterp, VistaIOImageViewpointInterp,
 *  VistaIOImageColorInterp, VistaIOImageComponentInterp
 *
 *  Routines for accessing an image's band interpretation information.
 *
 *  Each routine returns a VistaIOBandInterpXxx constant describing how
 *  image bands are to be interpreted at a particular level of the band
 *  hierarchy.
 *
 *  If that level's dimension is 1 and there is no band interpretation
 *  attribute for the level, VistaIOBandInterpNone is returned.
 *
 *  If the dimension is >1 and there is no attribute, VistaIOBandInterpOther is
 *  returned.
 *
 *  If an image's band interpretation information is inconsistent
 *  (e.g., the color_interp attribute says RGB but ncolors is 2) then
 *  VistaIOWarning is called and VistaIOBandInterpOther is returned.
 */

EXPORT_VISTA VistaIOBandInterp VistaIOImageFrameInterp (VistaIOImage image)
{
	VistaIOLong interp;
	VistaIOGetAttrResult result;

	if (VistaIOImageNBands (image) !=
	    (VistaIOImageNFrames (image) * VistaIOImageNViewpoints (image) *
	     VistaIOImageNColors (image) * VistaIOImageNComponents (image)))
		VistaIOWarning ("VistaIOImageFrameInterp: No. bands (%d) conflicts with no. " "of frames, etc. (%d %d %d %d)", VistaIOImageNBands (image), VistaIOImageNFrames (image), VistaIOImageNViewpoints (image), VistaIOImageNColors (image), VistaIOImageNComponents (image));

	if (!VistaIOImageAttrList (image) ||
	    (result =
	     VistaIOGetAttr (VistaIOImageAttrList (image), VistaIOFrameInterpAttr,
		       VistaIOBandInterpDict, VistaIOLongRepn, &interp)) == VistaIOAttrMissing)
		return VistaIOImageNFrames (image) >
			1 ? VistaIOBandInterpOther : VistaIOBandInterpNone;

	if (result == VistaIOAttrBadValue)
		return VistaIOBandInterpOther;

	switch (interp) {

	}
	return VistaIOBandInterpOther;
}

/*! \brief Routine for accessing an image's band interpretation information.
 * 
 *  \param  image
 *  \return VistaIOBandInterp
 */ 

EXPORT_VISTA VistaIOBandInterp VistaIOImageViewpointInterp (VistaIOImage image)
{
	VistaIOLong interp;
	VistaIOGetAttrResult result;

	if (VistaIOImageNBands (image) !=
	    (VistaIOImageNFrames (image) * VistaIOImageNViewpoints (image) *
	     VistaIOImageNColors (image) * VistaIOImageNComponents (image)))
		VistaIOWarning ("VistaIOImageViewpointInterp: No. bands (%d) conflicts with no. " "of frames, etc. (%d %d %d %d)", VistaIOImageNBands (image), VistaIOImageNFrames (image), VistaIOImageNViewpoints (image), VistaIOImageNColors (image), VistaIOImageNComponents (image));

	if (!VistaIOImageAttrList (image) ||
	    (result =
	     VistaIOGetAttr (VistaIOImageAttrList (image), VistaIOViewpointInterpAttr,
		       VistaIOBandInterpDict, VistaIOLongRepn, &interp)) == VistaIOAttrMissing)
		return VistaIOImageNViewpoints (image) >
			1 ? VistaIOBandInterpOther : VistaIOBandInterpNone;

	if (result == VistaIOAttrBadValue)
		return VistaIOBandInterpOther;

	switch (interp) {

	case VistaIOBandInterpStereoPair:
		if (VistaIOImageNViewpoints (image) != 2) {
			VistaIOWarning ("VistaIOBandViewpointInterp: "
				  "Stereo-pair image has %d viewpoint dimension(s)",
				  VistaIOImageNViewpoints (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpStereoPair;
	}
	return VistaIOBandInterpOther;
}

/*! \brief Routine for accessing an image's band interpretation information.
 * 
 *  \param  image
 *  \return VistaIOBandInterp
 */

EXPORT_VISTA VistaIOBandInterp VistaIOImageColorInterp (VistaIOImage image)
{
	VistaIOLong interp;
	VistaIOGetAttrResult result;

	if (VistaIOImageNBands (image) !=
	    (VistaIOImageNFrames (image) * VistaIOImageNViewpoints (image) *
	     VistaIOImageNColors (image) * VistaIOImageNComponents (image)))
		VistaIOWarning ("VistaIOImageColorInterp: No. bands (%d) conflicts with no. " "of frames, etc. (%d %d %d %d)", VistaIOImageNBands (image), VistaIOImageNFrames (image), VistaIOImageNViewpoints (image), VistaIOImageNColors (image), VistaIOImageNComponents (image));

	if (!VistaIOImageAttrList (image) ||
	    (result =
	     VistaIOGetAttr (VistaIOImageAttrList (image), VistaIOColorInterpAttr,
		       VistaIOBandInterpDict, VistaIOLongRepn, &interp)) == VistaIOAttrMissing)
		return VistaIOImageNColors (image) >
			1 ? VistaIOBandInterpOther : VistaIOBandInterpNone;

	if (result == VistaIOAttrBadValue)
		return VistaIOBandInterpOther;

	switch (interp) {

	case VistaIOBandInterpRGB:
		if (VistaIOImageNColors (image) != 3) {
			VistaIOWarning ("VistaIOBandColorInterp: RGB image has %d color dimension(s)", VistaIOImageNColors (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpRGB;
	}
	return VistaIOBandInterpOther;
}

/*! \brief Routine for accessing an image's band interpretation information.
 * 
 *  \param  image
 *  \return VistaIOBandInterp
 */

EXPORT_VISTA VistaIOBandInterp VistaIOImageComponentInterp (VistaIOImage image)
{
	VistaIOLong interp;
	VistaIOGetAttrResult result;

	if (VistaIOImageNBands (image) !=
	    (VistaIOImageNFrames (image) * VistaIOImageNViewpoints (image) *
	     VistaIOImageNColors (image) * VistaIOImageNComponents (image)))
		VistaIOWarning ("VistaIOImageComponentInterp: No. bands (%d) conflicts with no. " "of frames, etc. (%d %d %d %d)", VistaIOImageNBands (image), VistaIOImageNFrames (image), VistaIOImageNViewpoints (image), VistaIOImageNColors (image), VistaIOImageNComponents (image));

	if (!VistaIOImageAttrList (image) ||
	    (result =
	     VistaIOGetAttr (VistaIOImageAttrList (image), VistaIOComponentInterpAttr,
		       VistaIOBandInterpDict, VistaIOLongRepn, &interp)) == VistaIOAttrMissing)
		return VistaIOImageNComponents (image) >
			1 ? VistaIOBandInterpOther : VistaIOBandInterpNone;

	if (result == VistaIOAttrBadValue)
		return VistaIOBandInterpOther;

	switch (interp) {

	case VistaIOBandInterpComplex:
		if (VistaIOImageNComponents (image) != 2) {
			VistaIOWarning ("VistaIOBandColorInterp: Complex image has %d component(s)", VistaIOImageNComponents (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpComplex;

	case VistaIOBandInterpGradient:
		if (VistaIOImageNComponents (image) > 3) {
			VistaIOWarning ("VistaIOBandColorInterp: Gradient image has %d component(s)", VistaIOImageNComponents (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpGradient;

	case VistaIOBandInterpIntensity:
		if (VistaIOImageNComponents (image) > 1) {
			VistaIOWarning ("VistaIOBandColorInterp: Intensity image has %d component(s)", VistaIOImageNComponents (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpIntensity;

	case VistaIOBandInterpOrientation:
		if (VistaIOImageNComponents (image) > 1) {
			VistaIOWarning ("VistaIOBandColorInterp: "
				  "Orientation image has %d component(s)",
				  VistaIOImageNComponents (image));
			return VistaIOBandInterpOther;
		}
		return VistaIOBandInterpOrientation;
	}
	return VistaIOBandInterpOther;
}


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

EXPORT_VISTA VistaIOBoolean VistaIOSetBandInterp (VistaIOImage image,
			 VistaIOBandInterp frame_interp, int nframes,
			 VistaIOBandInterp viewpoint_interp, int nviewpoints,
			 VistaIOBandInterp color_interp, int ncolors,
			 VistaIOBandInterp component_interp, int ncomponents)
{
	VistaIOBoolean result = TRUE;
	VistaIOString str;

	if (VistaIOImageNBands (image) !=
	    nframes * nviewpoints * ncolors * ncomponents) {
		VistaIOWarning ("VistaIOSetBandInterp: No. bands (%d) conflicts with no. "
			  "of frames, etc. (%d %d %d %d)",
			  VistaIOImageNBands (image), nframes, nviewpoints, ncolors,
			  ncomponents);
		result = FALSE;
	}

	if (frame_interp == VistaIOBandInterpNone)
		result &=
			VistaIOExtractAttr (VistaIOImageAttrList (image),
				      VistaIOFrameInterpAttr, NULL, VistaIOStringRepn,
				      &str, FALSE);
	else
		VistaIOSetAttr (VistaIOImageAttrList (image), VistaIOFrameInterpAttr,
			  VistaIOBandInterpDict, VistaIOLongRepn, (VistaIOLong) frame_interp);
	VistaIOImageNFrames (image) = nframes;

	if (viewpoint_interp == VistaIOBandInterpNone)
		result &=
			VistaIOExtractAttr (VistaIOImageAttrList (image),
				      VistaIOViewpointInterpAttr, NULL, VistaIOStringRepn,
				      &str, FALSE);
	else
		VistaIOSetAttr (VistaIOImageAttrList (image), VistaIOViewpointInterpAttr,
			  VistaIOBandInterpDict, VistaIOLongRepn,
			  (VistaIOLong) viewpoint_interp);
	VistaIOImageNViewpoints (image) = nviewpoints;

	if (color_interp == VistaIOBandInterpNone)
		result &=
			VistaIOExtractAttr (VistaIOImageAttrList (image),
				      VistaIOColorInterpAttr, NULL, VistaIOStringRepn,
				      &str, FALSE);
	else
		VistaIOSetAttr (VistaIOImageAttrList (image), VistaIOColorInterpAttr,
			  VistaIOBandInterpDict, VistaIOLongRepn, (VistaIOLong) color_interp);
	VistaIOImageNColors (image) = ncolors;

	if (component_interp == VistaIOBandInterpNone)
		result &=
			VistaIOExtractAttr (VistaIOImageAttrList (image),
				      VistaIOComponentInterpAttr, NULL, VistaIOStringRepn,
				      &str, FALSE);
	else
		VistaIOSetAttr (VistaIOImageAttrList (image), VistaIOComponentInterpAttr,
			  VistaIOBandInterpDict, VistaIOLongRepn,
			  (VistaIOLong) component_interp);
	VistaIOImageNComponents (image) = ncomponents;

	return result;
}


/*! \brief Read a Vista data file, extract the images from it, and return a 
 *         list of them.
 * 
 *  \param  file
 *  \param  attributes
 *  \param  images
 *  \return int
 */

EXPORT_VISTA int VistaIOReadImages (FILE * file, VistaIOAttrList * attributes, VistaIOImage ** images)
{
	return VistaIOReadObjects (file, VistaIOImageRepn, attributes,
			     (VistaIOPointer **) images);
}


/*! \brief Write a list of images to a Vista data file.
 *
 *  \param  file
 *  \param  attributes
 *  \param  nimages
 *  \param  images
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIOWriteImages (FILE * file, VistaIOAttrList attributes, int nimages,
		       VistaIOImage images[])
{
	return VistaIOWriteObjects (file, VistaIOImageRepn, attributes, nimages,
			      (VistaIOPointer *) images);
}
