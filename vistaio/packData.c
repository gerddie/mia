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

/*$Id: packData.c 52 2004-03-02 15:53:19Z tittge $ */

/*! \file  packData.c
 *  \brief routines for packing and unpacking arrays of data elements
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/* Later in this file: */
static VPackOrder MachineByteOrder (void);
static void SwapBytes (size_t, size_t, char *);


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

VBoolean VPackData (VRepnKind repn,
		    size_t nels, VPointer unpacked, VPackOrder packed_order,
		    size_t * length, VPointer * packed, VBoolean * alloced)
{
	VPackOrder unpacked_order;
	size_t unpacked_elsize = VRepnSize (repn) * CHAR_BIT;
	size_t packed_elsize = VRepnPrecision (repn);
	size_t packed_length = (nels * packed_elsize + 7) / 8;

	/* If space for the packed data was supplied, ensure there's
	   enough of it: */
	if (!alloced && packed_length > *length) {
		VWarning ("VPackData: Insufficient space for packed data");
		return FALSE;
	}
	*length = packed_length;

	/* Determine the present machine's internal byte order: */
	unpacked_order = MachineByteOrder ();

	/* If the desired byte order matches that of the present machine's, and
	   the unpacked and packed data element sizes are identical,
	   just return the unpacked data: */
	if (unpacked_order == packed_order
	    && unpacked_elsize == packed_elsize) {
		if (alloced) {
			*packed = unpacked;
			*alloced = FALSE;
		} else if (unpacked != packed)
			memcpy (*packed, unpacked, packed_length);
		return TRUE;
	}

	/* Allocate a buffer for the packed data if none was provided: */
	if (alloced) {
		*packed = VMalloc (packed_length);
		*alloced = TRUE;
	}

	/* Pack data elements into the buffer: */
	if (unpacked_elsize == packed_elsize) {

		/* If the packed and unpacked are the same size, do a straight copy: */
		if (unpacked != *packed)
			memcpy (*packed, unpacked, packed_length);

		/* Swap bytes if necessary: */
		if (packed_order != unpacked_order && packed_elsize > 8)
			SwapBytes (nels, packed_elsize / 8, (char *)*packed);

	} else if (packed_elsize == 1) {

		/* If the elements are VBits, this packs them: */
		VPackBits (nels, packed_order, (VBit *) unpacked,
			   (char *)*packed);

	} else
		/* Packing multi-byte integers or floats is currently not supported: */
		VError ("VPackData: Packing %s from %d to %d bits is not supported", VRepnName (repn), unpacked_elsize, packed_elsize);

	return TRUE;
}


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

VBoolean VUnpackData (VRepnKind repn,
		      size_t nels, VPointer packed, VPackOrder packed_order,
		      size_t * length, VPointer * unpacked,
		      VBoolean * alloced)
{
	VPackOrder unpacked_order;
	size_t unpacked_elsize = VRepnSize (repn) * CHAR_BIT;
	size_t packed_elsize = VRepnPrecision (repn);
	size_t unpacked_length = nels * VRepnSize (repn);

	/* If a space for the unpacked data was supplied, ensure there's
	   enough of it: */
	if (!alloced && unpacked_length > *length) {
		VWarning ("VUnpackData: Insufficient space for unpacked data");
		return FALSE;
	}
	*length = unpacked_length;

	/* Determine the present machine's internal byte order: */
	unpacked_order = MachineByteOrder ();

	/* If the desired byte order matches that of the present machine's, and
	   the unpacked and packed data element sizes are identical,
	   just return the packed data: */
	if (unpacked_order == packed_order
	    && unpacked_elsize == packed_elsize) {
		if (alloced) {
			*unpacked = packed;
			*alloced = FALSE;
		} else if (packed != *unpacked)
			memcpy (*unpacked, packed, unpacked_length);
		return TRUE;
	}

	/* Unpack data elements into the buffer: */
	if (packed_elsize == unpacked_elsize) {

		/* If the packed and unpacked are the same size, do a straight copy: */
		if (packed != *unpacked)
			memcpy (*unpacked, packed, unpacked_length);

		/* Swap bytes if necessary: */
		if (packed_order != unpacked_order && packed_elsize > 8)
			SwapBytes (nels, packed_elsize / 8,
				   (char *)*unpacked);

	} else if (packed_elsize == 1) {

		/* If the elements are VBits, this unpacks them: */
		VUnpackBits (nels, packed_order, (char *)packed,
			     (char *)*unpacked);

	} else
		/* Unpacking multi-byte integers or floats is currently not
		   supported: */
		VError ("VUnpackData: "
			"Unpacking %s from %d to %d bits is not supported",
			VRepnName (repn), packed_elsize, unpacked_elsize);

	return TRUE;
}


/*
 *  MachineByteOrder
 *
 *  Returns VLittleEndian if the first byte of a word is a low-order byte, 
 *  VBigEndian if it's a high-order byte.
 */

static VPackOrder MachineByteOrder ()
{
	union {
		short s;
		char c[sizeof (short)];
	} u;

	u.s = 1;
	if (u.c[0] == 1)
		return VLsbFirst;
	if (u.c[sizeof (short) - 1] != 1)
		VError ("VPackImage or VUnpackImage: Byte order not recognized");
	return VMsbFirst;
}


/*! \brief Pack the low order bits of consecutive VBit data elements.
 *  
 *  unpacked and packed can point to the same place.
 *
 *  \param  nels
 *  \param  packed_order
 *  \param  unpacked
 *  \param  packed
 */

void VPackBits (size_t nels, VPackOrder packed_order, VBit * unpacked,
		char *packed)
{
	int bit;
	char byte;

	if (packed_order == VLsbFirst)
		while (nels > 0) {
			byte = 0;
			for (bit = 0; bit < 8 && nels > 0; nels--, bit++)
				if (*unpacked++)
					byte |= (1 << bit);
			*packed++ = byte;
	} else
		while (nels > 0) {
			byte = 0;
			for (bit = 7; bit >= 0 && nels > 0; nels--, bit--)
				if (*unpacked++)
					byte |= (1 << bit);
			*packed++ = byte;
		}
}


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
		  VBit * unpacked)
{
	int bit;
	char byte;

	/* Compute the position of the first bit to be unpacked, which is the
	   last bit of the vector: */
	bit = (nels + 7) % 8;
	if (packed_order == VMsbFirst)
		bit = 7 - bit;

	/* Unpack bits from last to first. For each byte to be unpacked: */
	packed += (nels + 7) / 8;
	unpacked += nels;
	if (packed_order == VLsbFirst)
		while (nels > 0) {
			byte = *--packed;

			/* For each bit to be unpacked within that byte: */
			for (; bit >= 0 && nels > 0; nels--, bit--) {

				/* Unpack a bit: */
				*--unpacked = (byte >> bit) & 1;
			}
			bit = 7;
	} else
		while (nels > 0) {
			byte = *--packed;

			/* For each bit to be unpacked within that byte: */
			for (; bit < 8 && nels > 0; nels--, bit++) {

				/* Unpack a bit: */
				*--unpacked = (byte >> bit) & 1;
			}
			bit = 0;
		}
}


/*! \brief Reverses the byte order of nels elements, each of elsize bytes, 
 *         stored at data.
 *  
 *  \param  nels
 *  \param  elsize
 *  \param  data
 */

static void SwapBytes (size_t nels, size_t elsize, char *data)
{
	size_t i;
	char *pl, *pu, byte;

	for (i = 0; i < nels; i++, data += elsize)
		for (pl = data, pu = data + elsize - 1; pl < pu; pl++, pu--) {
			byte = *pl;
			*pl = *pu;
			*pu = byte;
		}
}
