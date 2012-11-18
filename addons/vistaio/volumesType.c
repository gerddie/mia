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
static VistaIODecodeMethod VolumesDecodeMethod;
static VistaIOEncodeAttrMethod VolumesEncodeAttrMethod;
static VistaIOEncodeDataMethod VolumesEncodeDataMethod;

/* Used in Type.c to register this type: */
VistaIOTypeMethods VolumesMethods = {
	(VistaIOCopyMethod *) VistaIOCopyVolumes,	/* copy a Volumes */
	(VistaIODestroyMethod *) VistaIODestroyVolumes,	/* destroy a Volumes */
	VolumesDecodeMethod,	/* decode a Volumes's value */
	VolumesEncodeAttrMethod,	/* encode a Volumes's attr list */
	VolumesEncodeDataMethod	/* encode a Volumes's binary data */
};



/*! \brief VolumesDecodeMethod
 *
 *  The "decode" method registered for the "Volumes" type.
 *  Convert an attribute list plus binary data to a Volumes object.
 *
 *  \param  name
 *  \param  b
 *  \return VistaIOPointer
 */

static VistaIOPointer VolumesDecodeMethod (VistaIOStringConst name, VistaIOBundle b)
{
	Volumes volumes;
	VistaIOAttrList list;
	int nels, i, j;
	size_t length;
	VistaIOShort hashlen, nbands, nrows, ncols;
	short nt1, nt2;
	short label;
	VistaIOShort nvolumes;
	VistaIOLong ntracks;
	short *p;
	VistaIOTrack t;
	VistaIOPointer data;
	Volume vol;

#define Extract(name, dict, locn, required)	\
  VistaIOExtractAttr (b->list, name, dict, VistaIOShortRepn, & locn, required)

	/* Extract the required attribute values for Volumes. */
	if (!Extract (VolNVolumesAttr, NULL, nvolumes, TRUE) ||
	    !Extract (VolNBandsAttr, NULL, nbands, TRUE) ||
	    !Extract (VolNRowsAttr, NULL, nrows, TRUE) ||
	    !Extract (VolNColumnsAttr, NULL, ncols, TRUE))
		return NULL;
	if (nvolumes <= 0) {
		VistaIOWarning ("VolumesReadDataMethod: Bad Volumes file attributes");
		return NULL;
	}

	/* Create the Volumes data structure. */
	volumes = VistaIOCreateVolumes (nbands, nrows, ncols);
	if (!volumes)
		return NULL;

	/* Give it whatever attributes remain: */
	list = VolumesAttrList (volumes);
	VolumesAttrList (volumes) = b->list;
	b->list = list;

	/* check amount of binary data: */
	nels = b->length / (VistaIORepnPrecision (VistaIOShortRepn) / 8);
	length = nels * (VistaIORepnPrecision (VistaIOShortRepn) / 8);

	/* Allocate storage for the Volumes binary data: */
	data = VistaIOMalloc (nels * sizeof (VistaIOShort));

	/* Unpack the binary data: */
	if (!VistaIOUnpackData
	    (VistaIOShortRepn, nels, b->data, VistaIOMsbFirst, &length, &data, NULL))
		VistaIOError ("error unpacking data");

	p = (VistaIOShort *) data;
	for (i = 0; i < nvolumes; i++) {
		nt1 = *p++;
		nt2 = *p++;
		ntracks =
			(long)nt1 + (long)nt2 -
			(long)VistaIORepnMinValue (VistaIOShortRepn);
		hashlen = *p++;
		nbands = *p++;
		label = *p++;
		vol = VistaIOCreateVolume (label, nbands, nrows, ncols, hashlen);
		for (j = 0; j < ntracks; j++) {
			t = VistaIOMalloc (sizeof (VistaIOTrackRec));
			t->band = *p++;
			t->row = *p++;
			t->col = *p++;
			t->length = *p++;
			AddTrack (vol, t);
		}
		VistaIOAddVolume (volumes, vol);
	}
	return volumes;

#undef Extract
}



/*! \brief VolumesEncodeAttrMethod
 *
 *  The "encode_attrs" method registered for the "Volumes" type.
 *  Encode an attribute list value for a Volumes object.
 *
 *  \param  value
 *  \param  lengthp
 *  \return VistaIOAttrList
 */

static VistaIOAttrList VolumesEncodeAttrMethod (VistaIOPointer value, size_t * lengthp)
{
	Volumes volumes = value;
	Volume v;
	VistaIOAttrList list;
	VistaIOLong ntracks;

	/* Temporarily prepend several attributes to the edge set's list: */
	if ((list = VolumesAttrList (volumes)) == NULL)
		list = VolumesAttrList (volumes) = VistaIOCreateAttrList ();
	VistaIOPrependAttr (list, VolNVolumesAttr, NULL, VistaIOShortRepn,
		      (VistaIOShort) volumes->nvolumes);
	VistaIOPrependAttr (list, VolNBandsAttr, NULL, VistaIOShortRepn,
		      (VistaIOShort) volumes->nbands);
	VistaIOPrependAttr (list, VolNRowsAttr, NULL, VistaIOShortRepn,
		      (VistaIOShort) volumes->nrows);
	VistaIOPrependAttr (list, VolNColumnsAttr, NULL, VistaIOShortRepn,
		      (VistaIOShort) volumes->ncolumns);

	/* Compute the file space needed for the Volumes's binary data: */

	ntracks = 0;
	for (v = volumes->first; v != NULL; v = v->next)
		ntracks += v->ntracks;

	/*
	   *lengthp = (VistaIORepnPrecision (VistaIOShortRepn) / 8)
	   * ((ntracks * 4) + (volumes->nvolumes * 2))
	   + (2 * VistaIORepnPrecision (VistaIOShortRepn) / 8) * volumes->nvolumes;
	 */

	*lengthp =
		(VistaIORepnPrecision (VistaIOShortRepn) / 8) * (4 * ntracks +
						     5 * volumes->nvolumes);

	return list;
}


/*! \briefVolumesEncodeDataMethod
 *
 *  The "encode_data" method registered for the "Volumes" type.
 *
 *  \param  value
 *  \param  list
 *  \param  length
 *  \param  free_itp
 *  \return VistaIOPointer
 */

static VistaIOPointer
VolumesEncodeDataMethod (VistaIOPointer value, VistaIOAttrList list, size_t length,
			 VistaIOBoolean * free_itp)
{
	Volumes volumes = value;
	Volume v;
	size_t len;
	VistaIOPointer ptr, p;
	VistaIOShort idata[4], hashlen, nbands, ntracks, label;
	long nt1, nt2;
	int i;
	VistaIOTrack t;

	/* Allocate a buffer for the encoded data: */
	p = ptr = VistaIOMalloc (length);

	/* Pack each edge: */
	for (v = volumes->first; v != NULL; v = v->next) {

		/* Pack the number of tracks in volume: */
		nt2 = 0;
		nt1 = v->ntracks + (long)VistaIORepnMinValue (VistaIOShortRepn);
		if (nt1 > (long)VistaIORepnMaxValue (VistaIOShortRepn)) {
			nt1 = (long)VistaIORepnMaxValue (VistaIOShortRepn) - 1;
			nt2 = v->ntracks - nt1 +
				(long)VistaIORepnMinValue (VistaIOShortRepn);
		}
		if (nt2 > (VistaIOLong) VistaIORepnMaxValue (VistaIOShortRepn)) {
			VistaIOError ("too many tracks in volume: ntracks:%d  %d %d", nt1, nt2, v->ntracks);
		}

		ntracks = (VistaIOShort) nt1;
		len = VistaIORepnPrecision (VistaIOShortRepn) / 8;
		if (!VistaIOPackData
		    (VistaIOShortRepn, 1, &ntracks, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		ntracks = (VistaIOShort) nt2;
		len = VistaIORepnPrecision (VistaIOShortRepn) / 8;
		if (!VistaIOPackData
		    (VistaIOShortRepn, 1, &ntracks, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		/* Pack the hashtable length: */
		hashlen = (VistaIOShort) v->nbuckets;
		len = VistaIORepnPrecision (VistaIOShortRepn) / 8;
		if (!VistaIOPackData
		    (VistaIOShortRepn, 1, &hashlen, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		/* Pack the number of bands (used in hash function): */
		nbands = (VistaIOShort) v->nbands;
		len = VistaIORepnPrecision (VistaIOShortRepn) / 8;
		if (!VistaIOPackData
		    (VistaIOShortRepn, 1, &nbands, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		/* Pack label id */
		label = (VistaIOShort) v->label;
		len = VistaIORepnPrecision (VistaIOShortRepn) / 8;
		if (!VistaIOPackData
		    (VistaIOShortRepn, 1, &label, VistaIOMsbFirst, &len, &p, NULL))
			return NULL;
		p = (char *)p + len;
		length -= len;

		for (i = 0; i < hashlen; i++) {
			for (t = v->bucket[i].first; t != NULL; t = t->next) {
				idata[0] = t->band;
				idata[1] = t->row;
				idata[2] = t->col;
				idata[3] = t->length;
				len = 4 * VistaIORepnPrecision (VistaIOShortRepn) / 8;
				if (!VistaIOPackData
				    (VistaIOShortRepn, 4, &idata[0], VistaIOMsbFirst,
				     &len, &p, NULL))
					return NULL;
				p = (char *)p + len;
				length -= len;
			}
		}
	}

	*free_itp = TRUE;
	return ptr;
}
