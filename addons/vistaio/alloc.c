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
