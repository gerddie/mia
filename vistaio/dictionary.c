/*
* Copyrigh (C) 2004 Max-Planck-Institute of Cognitive Neurosience
*
* The origional VISTA library is copyrighted of University of British Columbia.
* Copyright © 1993, 1994 University of British Columbia.
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

/* $Id: dictionary.c 51 2004-02-26 12:53:22Z jaenicke $ */

/*! \file  dictionary.c
 *  \brief contains dictionaries that convert symbols used in V data files to values used internally
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/*
 *  Dictionaries for generic attributes and their values.
 */

/* Keywords for representing TRUE or FALSE: */
EXPORT_VISTA VDictEntry VBooleanDict[] = {
	{"false", FALSE}
	,
	{"true", TRUE}
	,
	{"no", FALSE}
	,
	{"yes", TRUE}
	,
	{"off", FALSE}
	,
	{"on", TRUE}
	,
	{NULL}
};

/* Keywords for representing kinds of numeric representation: */
EXPORT_VISTA VDictEntry VNumericRepnDict[] = {
	{"bit", VBitRepn}
	,
	{"double", VDoubleRepn}
	,
	{"float", VFloatRepn}
	,
	{"long", VLongRepn}
	,
	{"sbyte", VSByteRepn}
	,
	{"short", VShortRepn}
	,
	{"ubyte", VUByteRepn}
	,
	{NULL}
};


/*! \brief Look up an entry in an attribute value dictionary, by keyword.
 *  
 *  (It's assumed that dictionaries are pretty small -- a linear search
 *  is done.)
 *
 *  \param  dict
 *  \param  keyword
 *  \return VDictEntry
 */

VDictEntry *VLookupDictKeyword (VDictEntry * dict, VStringConst keyword)
{
	if (dict)
		for (; dict->keyword; dict++)
			if (strcmp (keyword, dict->keyword) == 0)
				return dict;
	return NULL;
}


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

VDictEntry *VLookupDictValue (VDictEntry * dict, VRepnKind repn, ...)
{
	va_list args;
	VLong i_value = 0;
	VDouble f_value = 0.0;
	VString s_value = NULL;
	VBoolean i_valid;

	/* Unravel the arguments passed: */
	if (!dict)
		return NULL;
	va_start (args, repn);
	switch (repn) {
	case VBitRepn:
		i_value = va_arg (args, VBitPromoted);
		break;
	case VUByteRepn:
		i_value = va_arg (args, VUBytePromoted);
		break;
	case VSByteRepn:
		i_value = va_arg (args, VSBytePromoted);
		break;
	case VShortRepn:
		i_value = va_arg (args, VShortPromoted);
		break;
	case VLongRepn:
		i_value = va_arg (args, VLongPromoted);
		break;
	case VFloatRepn:
		f_value = va_arg (args, VFloatPromoted);
		break;
	case VDoubleRepn:
		f_value = va_arg (args, VDoublePromoted);
		break;
	case VBooleanRepn:
		i_value = va_arg (args, VBooleanPromoted);
		break;
	case VStringRepn:
		s_value = va_arg (args, VString);
		break;
	default:
		VError ("VLookupDictValue: Can't lookup %s value",
			VRepnName (repn));
	}
	va_end (args);

	/* Search the dictionary by value: */
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VBooleanRepn:
		for (; dict->keyword; dict++) {

			/* Is the entry's value only stored as a string? */
			if (dict->svalue && !dict->icached) {

				/* Yes -- try to convert the string to an integer, and
				   cache that value: */
				if (!VDecodeAttrValue
				    (dict->svalue, NULL, VLongRepn,
				     &dict->ivalue))
					break;
				dict->icached = TRUE;
			}

			/* Test against the integer value stored in the entry: */
			if (i_value == dict->ivalue)
				return dict;
		}
		break;

	case VFloatRepn:
	case VDoubleRepn:
		for (; dict->keyword; dict++) {

			/* Does the entry include a cached floating point value? */
			if (!dict->fcached) {

				/* No -- obtain it from an integer or string value: */
				if (dict->svalue) {
					if (!VDecodeAttrValue
					    (dict->svalue, NULL, VDoubleRepn,
					     &dict->fvalue))
						break;
				} else
					dict->fvalue = dict->ivalue;
				dict->fcached = TRUE;
			}

			/* Test against the cached float value now stored in the entry: */
			if (f_value == dict->fvalue)
				return dict;
		}
		break;

	case VStringRepn:

		/* In case we're searching a dictionary with only integer values
		   stored, try to convert the supplied string value to an integer: */
		i_valid =
			VDecodeAttrValue (s_value, NULL, VLongRepn, &i_value);

		for (; dict->keyword; dict++) {

			/* If the entry includes a string value, compare with it: */
			if (dict->svalue) {
				if (strcmp (s_value, dict->svalue) == 0)
					return dict;
			}

			/* Otherwise, compare with its integer value: */
			else if (i_valid && i_value == dict->ivalue)
				return dict;
		}
		break;

	default:
		break;
	}
	return NULL;
}
