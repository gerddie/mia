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

/*$Id: volumes.c 51 2004-02-26 12:53:22Z jaenicke $*/

/*! \file  volumes.c
 *  \brief Methods for the volume set (Volumes) data type.
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/*! \brief create a new volume list and return a ptr to it.
 *  
 *  \param  nbands
 *  \param  nrows
 *  \param  ncols
 *  \return Volumes
 */

Volumes VCreateVolumes (short nbands, short nrows, short ncols)
{
	Volumes volumes;

	/* allocate memory: */
	volumes = VMalloc (sizeof (VolumesRec));
	if (!volumes)
		return NULL;

	/* initialize structure: */
	volumes->attributes = VCreateAttrList ();
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

Volume VCreateVolume (short label, short nbands, short nrows, short ncolumns,
		      short nbuckets)
{
	Volume vol;
	int i;

	vol = VMalloc (sizeof (VolumeRec));
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
	vol->bucket = (VBucket) VMalloc (sizeof (VBucketRec) * vol->nbuckets);
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

Volume VCopyVolume (Volume src)
{
	Volume dest;
	VTrack s, t, s_prev;
	int i, j, ntracks;

	dest = VCreateVolume (src->label, src->nbands, src->nrows,
			      src->ncolumns, src->nbuckets);
	dest->ntracks = src->ntracks;

	for (i = 0; i < src->nbuckets; i++) {

		ntracks = src->bucket[i].ntracks;
		dest->bucket[i].ntracks = ntracks;
		dest->bucket[i].first = NULL;
		dest->bucket[i].last = NULL;

		if (ntracks == 0)
			continue;

		s = VMalloc (sizeof (VTrackRec));
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
			s->next = VMalloc (sizeof (VTrackRec));
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

void AddTrack (Volume v, VTrack t)
{
	int i;
	VTrack s, r;

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

void VAddVolume (Volumes volumes, Volume vol)
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

Volumes VCopyVolumes (Volumes src)
{
	Volumes dest;
	Volume v;

	dest = VCreateVolumes (src->nbands, src->nrows, src->ncolumns);
	if (!dest)
		return NULL;

	for (v = src->first; v != NULL; v = v->next) {
		VAddVolume (dest, VCopyVolume (v));
	}

	if (VolumesAttrList (src))
		VolumesAttrList (dest) =
			VCopyAttrList (VolumesAttrList (src));
	else
		VolumesAttrList (dest) = NULL;

	return dest;
}


/*! \brief destroy a single volume 
 *
 *  \param v
 */

void VDestroyVolume (Volume v)
{
	int i, n;
	VTrack t, s;

	for (i = 0; i < v->nbuckets; i++) {
		t = v->bucket[i].first;
		n = 0;
		while (t != NULL) {
			s = t;
			t = t->next;
			VFree (s);
			n++;
		}
		if (n > v->bucket[i].ntracks)
			VError ("VDestroyVolume: free error");
	}

	v->nbuckets = 0;
	v->ntracks = 0;
	v->next = NULL;
	VFree (v->bucket);
	v->bucket = NULL;
	VFree (v);
}


/*! \brief destroy a set of volumes
 *
 *  \param volumes
 */

void VDestroyVolumes (Volumes volumes)
{
	Volume v, w;

	v = volumes->first;

	while (v != NULL) {
		w = v;
		v = v->next;
		VDestroyVolume (w);
	}
	volumes->first = NULL;
	VDestroyAttrList (volumes->attributes);
	VFree (volumes);
}

/*! \brief
 *
 *  \param  file
 *  \param  attributes
 *  \param  volumes
 *  \return int
 */
 
int VReadVolumes (FILE * file, VAttrList * attributes, Volumes ** volumes)
{
	return VReadObjects (file, VolumesRepn, attributes,
			     (VPointer **) volumes);
}

/*! \brief
 *
 *  \param  file
 *  \param  attributes
 *  \param  nvolumes
 *  \param  volumes
 *  \return VBoolean
 */

VBoolean VWriteVolumes (FILE * file, VAttrList attributes, int nvolumes,
			Volumes * volumes)
{
	return VWriteObjects (file, VolumesRepn, attributes, nvolumes,
			      (VPointer *) volumes);
}
