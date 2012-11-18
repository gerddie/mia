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
 *  Dictionaries for generic attributes and their values.
 */

/* Keywords for representing TRUE or FALSE: */
EXPORT_VISTA VistaIODictEntry VistaIOBooleanDict[] = {
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
EXPORT_VISTA VistaIODictEntry VistaIONumericRepnDict[] = {
	{"bit", VistaIOBitRepn}
	,
	{"double", VistaIODoubleRepn}
	,
	{"float", VistaIOFloatRepn}
	,
	{"long", VistaIOLongRepn}
	,
	{"sbyte", VSByteRepn}
	,
	{"short", VistaIOShortRepn}
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
 *  \return VistaIODictEntry
 */

VistaIODictEntry *VistaIOLookupDictKeyword (VistaIODictEntry * dict, VistaIOStringConst keyword)
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
 *	VistaIOLookupDictValue (VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
 *
 *  where xxx is a type that corresponds to repn.
 *
 *  \param  dict
 *  \param  repn
 *  \return VistaIODictEntry
 */

VistaIODictEntry *VistaIOLookupDictValue (VistaIODictEntry * dict, VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOLong i_value = 0;
	VistaIODouble f_value = 0.0;
	VistaIOString s_value = NULL;
	VistaIOBoolean i_valid;

	/* Unravel the arguments passed: */
	if (!dict)
		return NULL;
	va_start (args, repn);
	switch (repn) {
	case VistaIOBitRepn:
		i_value = va_arg (args, VistaIOBitPromoted);
		break;
	case VUByteRepn:
		i_value = va_arg (args, VUBytePromoted);
		break;
	case VSByteRepn:
		i_value = va_arg (args, VSBytePromoted);
		break;
	case VistaIOShortRepn:
		i_value = va_arg (args, VistaIOShortPromoted);
		break;
	case VistaIOLongRepn:
		i_value = va_arg (args, VistaIOLongPromoted);
		break;
	case VistaIOFloatRepn:
		f_value = va_arg (args, VistaIOFloatPromoted);
		break;
	case VistaIODoubleRepn:
		f_value = va_arg (args, VistaIODoublePromoted);
		break;
	case VistaIOBooleanRepn:
		i_value = va_arg (args, VistaIOBooleanPromoted);
		break;
	case VistaIOStringRepn:
		s_value = va_arg (args, VistaIOString);
		break;
	default:
		VistaIOError ("VistaIOLookupDictValue: Can't lookup %s value",
			VistaIORepnName (repn));
	}
	va_end (args);

	/* Search the dictionary by value: */
	switch (repn) {

	case VistaIOBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOBooleanRepn:
		for (; dict->keyword; dict++) {

			/* Is the entry's value only stored as a string? */
			if (dict->svalue && !dict->icached) {

				/* Yes -- try to convert the string to an integer, and
				   cache that value: */
				if (!VistaIODecodeAttrValue
				    (dict->svalue, NULL, VistaIOLongRepn,
				     &dict->ivalue))
					break;
				dict->icached = TRUE;
			}

			/* Test against the integer value stored in the entry: */
			if (i_value == dict->ivalue)
				return dict;
		}
		break;

	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
		for (; dict->keyword; dict++) {

			/* Does the entry include a cached floating point value? */
			if (!dict->fcached) {

				/* No -- obtain it from an integer or string value: */
				if (dict->svalue) {
					if (!VistaIODecodeAttrValue
					    (dict->svalue, NULL, VistaIODoubleRepn,
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

	case VistaIOStringRepn:

		/* In case we're searching a dictionary with only integer values
		   stored, try to convert the supplied string value to an integer: */
		i_valid =
			VistaIODecodeAttrValue (s_value, NULL, VistaIOLongRepn, &i_value);

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
