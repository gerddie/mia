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

/*! \brief create a new volume list and return a ptr to it.
 *  
 *  \param  nbands
 *  \param  nrows
 *  \param  ncols
 *  \return Volumes
 */

Volumes VistaIOCreateVolumes (short nbands, short nrows, short ncols)
{
	Volumes volumes;

	/* allocate memory: */
	volumes = VistaIOMalloc (sizeof (VolumesRec));
	if (!volumes)
		return NULL;

	/* initialize structure: */
	volumes->attributes = VistaIOCreateAttrList ();
	volumes->first = NULL;
	volumes->nvolumes = 0;
	volumes->nbands = nbands;
	volumes->nrows = nrows;
	volumes->ncolumns = ncols;

	return volumes;
}

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
		      short nbuckets)
{
	Volume vol;
	int i;

	vol = VistaIOMalloc (sizeof (VolumeRec));
	vol->label = label;
	vol->nbands = nbands;
	vol->nrows = nrows;
	vol->ncolumns = ncolumns;
	vol->nbuckets = nbuckets;
	vol->ntracks = 0;
	vol->next = NULL;

	if (nbuckets > MAXHASHLEN)
		vol->nbuckets = MAXHASHLEN;
	else
		vol->nbuckets = (short)nbuckets;
	vol->bucket = (VistaIOBucket) VistaIOMalloc (sizeof (VistaIOBucketRec) * vol->nbuckets);
	if (!vol->bucket)
		return NULL;

	for (i = 0; i < vol->nbuckets; i++) {
		vol->bucket[i].ntracks = 0;
		vol->bucket[i].first = NULL;
		vol->bucket[i].last = NULL;
	}

	return vol;
}


/*! \brief copy src volume to dest volume
 *
 *  \param  src
 *  \return Volume
 */

Volume VistaIOCopyVolume (Volume src)
{
	Volume dest;
	VistaIOTrack s, t, s_prev;
	int i, j, ntracks;

	dest = VistaIOCreateVolume (src->label, src->nbands, src->nrows,
			      src->ncolumns, src->nbuckets);
	dest->ntracks = src->ntracks;

	for (i = 0; i < src->nbuckets; i++) {

		ntracks = src->bucket[i].ntracks;
		dest->bucket[i].ntracks = ntracks;
		dest->bucket[i].first = NULL;
		dest->bucket[i].last = NULL;

		if (ntracks == 0)
			continue;

		s = VistaIOMalloc (sizeof (VistaIOTrackRec));
		dest->bucket[i].first = s;
		s_prev = NULL;
		t = src->bucket[i].first;

		for (j = 0; j < ntracks - 1; j++) {
			s->band = t->band;
			s->row = t->row;
			s->col = t->col;
			s->length = t->length;

			s->previous = s_prev;
			s_prev = s;
			s->next = VistaIOMalloc (sizeof (VistaIOTrackRec));
			s = s->next;
			t = t->next;
		}
		s->band = t->band;
		s->row = t->row;
		s->col = t->col;
		s->length = t->length;
		s->previous = s_prev;
		s->next = NULL;
		dest->bucket[i].last = s;
	}
	return dest;
}



#define smaller(s,t)  \
     ((s)->band < (t)->band) \
  || (((s)->band == (t)->band) && ((s)->row < (t)->row)) \
  || (((s)->band == (t)->band) && ((s)->row == (t)->row) \
      && ((s)->col < (t)->col))

/*! \brief Add a new track to a hashtable while keeping the bucket sorted,
 *  
 *  The track must have been created prior to the call.
 * 
 *  \param  v
 *  \param  t
 */

void AddTrack (Volume v, VistaIOTrack t)
{
	int i;
	VistaIOTrack s, r;

	i = VolumeHash (v->nbands, t->band, t->row, v->nbuckets);

	if (v->bucket[i].ntracks < 1) {
		t->next = NULL;
		t->previous = NULL;
		v->bucket[i].first = t;
		v->bucket[i].last = t;
	} else {

		s = v->bucket[i].last;
		while ((s != NULL) && (smaller (t, s))) {
			s = s->previous;
		}

		if ((s == v->bucket[i].last)) {	/* append to the end */
			t->previous = s;
			t->next = NULL;
			v->bucket[i].last = t;
			s->next = t;
		}

		else if ((s == NULL)) {	/* prepend to start */
			r = v->bucket[i].first;
			t->previous = NULL;
			t->next = r;
			r->previous = t;
			v->bucket[i].first = t;
		}

		else {		/* insert somewhere into the middle */
			r = s->next;
			r->previous = t;
			t->previous = s;
			t->next = r;
			s->next = t;
		}
	}
	v->bucket[i].ntracks++;
	v->ntracks++;
}

/*! \brief append a volume to the end of a volume list
 *
 *  \param  volumes
 *  \param  vol
 */

void VistaIOAddVolume (Volumes volumes, Volume vol)
{
	Volume u = NULL, v = NULL;

	/* find end of volume list: */
	v = volumes->first;
	while (v != NULL) {
		u = v;
		v = v->next;
	}

	/* append new volume to end of list: */
	if (u != NULL)
		u->next = vol;
	else
		volumes->first = vol;
	volumes->nvolumes++;
	return;
}


/*! \brief copy a list a volumes
 *  
 *  \param  src
 *  \return Volumes
 */

Volumes VistaIOCopyVolumes (Volumes src)
{
	Volumes dest;
	Volume v;

	dest = VistaIOCreateVolumes (src->nbands, src->nrows, src->ncolumns);
	if (!dest)
		return NULL;

	for (v = src->first; v != NULL; v = v->next) {
		VistaIOAddVolume (dest, VistaIOCopyVolume (v));
	}

	if (VolumesAttrList (src))
		VolumesAttrList (dest) =
			VistaIOCopyAttrList (VolumesAttrList (src));
	else
		VolumesAttrList (dest) = NULL;

	return dest;
}


/*! \brief destroy a single volume 
 *
 *  \param v
 */

void VistaIODestroyVolume (Volume v)
{
	int i, n;
	VistaIOTrack t, s;

	for (i = 0; i < v->nbuckets; i++) {
		t = v->bucket[i].first;
		n = 0;
		while (t != NULL) {
			s = t;
			t = t->next;
			VistaIOFree (s);
			n++;
		}
		if (n > v->bucket[i].ntracks)
			VistaIOError ("VistaIODestroyVolume: free error");
	}

	v->nbuckets = 0;
	v->ntracks = 0;
	v->next = NULL;
	VistaIOFree (v->bucket);
	v->bucket = NULL;
	VistaIOFree (v);
}


/*! \brief destroy a set of volumes
 *
 *  \param volumes
 */

void VistaIODestroyVolumes (Volumes volumes)
{
	Volume v, w;

	v = volumes->first;

	while (v != NULL) {
		w = v;
		v = v->next;
		VistaIODestroyVolume (w);
	}
	volumes->first = NULL;
	VistaIODestroyAttrList (volumes->attributes);
	VistaIOFree (volumes);
}

/*! \brief
 *
 *  \param  file
 *  \param  attributes
 *  \param  volumes
 *  \return int
 */
 
int VistaIOReadVolumes (FILE * file, VistaIOAttrList * attributes, Volumes ** volumes)
{
	return VistaIOReadObjects (file, VolumesRepn, attributes,
			     (VistaIOPointer **) volumes);
}

/*! \brief
 *
 *  \param  file
 *  \param  attributes
 *  \param  nvolumes
 *  \param  volumes
 *  \return VistaIOBoolean
 */

VistaIOBoolean VistaIOWriteVolumes (FILE * file, VistaIOAttrList attributes, int nvolumes,
			Volumes * volumes)
{
	return VistaIOWriteObjects (file, VolumesRepn, attributes, nvolumes,
			      (VistaIOPointer *) volumes);
}
