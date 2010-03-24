/*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
*
* The origional VISTA library is copyrighted of University of British Columbia.
* Copyright © 1993, 1994 University of British Columbia.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Lesser Public License as published by
* the Free Software Foundation; either version 2 of the License, or
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

/* $Id: alloc.c 51 2004-02-26 12:53:22Z jaenicke $ */

/*! \file  alloc.c
 *  \brief routines for allocating and freeing memory, with error checking
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/*! \brief Perform error checking on malloc() call.
 *
 *  \param  size
 *  \return VPointer
 */

VPointer VMalloc (size_t size)
{
	VPointer p;

	if (size == 0)
		return NULL;
	if (!(p = (VPointer) malloc (size)))
		VSystemError ("VMalloc: Memory allocation failure");
	return p;
}


/*! \brief Perform error checking on realloc() call.
 *
 *  \param  p
 *  \param  size
 *  \return VPointer
 */

VPointer VRealloc (VPointer p, size_t size)
{
	if (size == 0) {
		VFree (p);
		return NULL;
	}
	if (!p)
		return VMalloc (size);
	if (!(p = (VPointer) realloc (p, size)))
		VSystemError ("VRealloc: Memory allocation failure");
	return p;
}


/*! \brief Perform error checking on calloc() call.
 *
 *  \param  n
 *  \param  size
 *  \return VPointer
 */

VPointer VCalloc (size_t n, size_t size)
{
	VPointer p;

	if (n == 0 || size == 0)
		return NULL;
	if (!(p = (VPointer) calloc (n, size)))
		VSystemError ("VCalloc: Memory allocation failure");
	return p;
}


/*! \brief Perform error checking on free() call.
 *
 *  \param p
 */

EXPORT_VISTA void VFree (VPointer p)
{
	if (p)
		free ((char *)p);
}
