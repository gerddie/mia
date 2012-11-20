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

#include "vista.h"

/* Later in this file: */
static VistaIOStringConst Encode (VistaIODictEntry * dict, VistaIORepnKind repn, va_list * args);
static VistaIOAttrRec *NewAttr (VistaIOStringConst, VistaIODictEntry *, VistaIORepnKind, va_list *);
static void SetAttr (VistaIOAttrListPosn *, VistaIODictEntry *, VistaIORepnKind, va_list *);
static void FreeAttrValue (VistaIOStringConst, VistaIOAttrRec *);


/*! \brief Append a new attribute to a list.
 *
 *  The calling sequence is:
 *
 *	VistaIOAppendAttr (VistaIOAttrList list, VistaIOStringConst name,
 *		     VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param  list
 *  \param  name
 *  \param  dict
 *  \param  repn
 */

EXPORT_VISTA void VistaIOAppendAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry * dict,
		  VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOAttrRec *a;

	/* Create the new attribute node: */
	va_start (args, repn);
	a = NewAttr (name, dict, repn, &args);
	va_end (args);

	/* Append it: */
	a->next = NULL;
	if (((a->prev) = (list->prev)))
		a->prev->next = a;
	else
		list->next = a;
	list->prev = a;
}


/*! \brief Make a copy of an attribute list.
 * 
 *  \param  list 
 *  \return VistaIOAttrList
 */

EXPORT_VISTA VistaIOAttrList VistaIOCopyAttrList (VistaIOAttrList list)
{
	VistaIOAttrList new_list = VistaIOCreateAttrList ();
	size_t name_size, value_size;
	VistaIOAttrRec *old_a, *new_a;
	VistaIOBundle old_b, new_b;
	VistaIOTypeMethods *methods;

	/* For each node of the old list: */
	for (old_a = list->next; old_a; old_a = old_a->next) {

		/* Compute the amount of storage needed for a copy of the node: */
		name_size = strlen (old_a->name);
		value_size = (old_a->repn == VistaIOStringRepn) ?
			strlen ((VistaIOStringConst) old_a->value) + 1 : 0;

		/* Allocate that size and fill in the node's value: */
		new_a = VistaIOMalloc (sizeof (VistaIOAttrRec) + name_size + value_size);
		strcpy (new_a->name, old_a->name);
		switch (new_a->repn = old_a->repn) {

		case VistaIOAttrListRepn:
			new_a->value = VistaIOCopyAttrList (old_a->value);
			break;

		case VistaIOBundleRepn:
			old_b = old_a->value;
			new_b = VistaIOCreateBundle (old_b->type_name,
					       VistaIOCopyAttrList (old_b->list),
					       old_b->length, NULL);
			if (old_b->length > 0) {
				new_b->data = VistaIOMalloc (old_b->length);
				memcpy (new_b->data, old_b->data,
					old_b->length);
			}
			new_a->value = new_b;
			break;

		case VistaIOPointerRepn:
			new_a->value = old_a->value;
			break;

		case VistaIOStringRepn:
			new_a->value =
				(VistaIOPointer) (new_a->name + name_size + 1);
			strcpy (new_a->value, old_a->value);
			break;

		default:
		  if ((methods = VistaIORepnMethods (new_a->repn)))
				new_a->value = (methods->copy) (old_a->value);
			else
				VistaIOError ("VistaIOCopyAttrList: %s attribute has invalid repn %d", old_a->name, old_a->repn);
		}

		/* Append it to the new list: */
		new_a->next = NULL;
		if (((new_a->prev) = (new_list->prev)))
			new_a->prev->next = new_a;
		if (!new_list->next)
			new_list->next = new_a;
		new_list->prev = new_a;
	}
	return new_list;
}


/*! \brief Create an attribute list.
 */

EXPORT_VISTA VistaIOAttrList VistaIOCreateAttrList (void)
{
	VistaIOAttrList list;

	list = VistaIONew (VistaIOAttrRec);
	list->next = list->prev = list->value = NULL;
	list->repn = VistaIOUnknownRepn;	/* not mistakable for an attribute */
	list->name[0] = 0;
	return list;
}


/*! \brief Create a bundle object.
 *
 *  \param  type_name
 *  \param  list
 *  \param  length
 *  \param  data
 *  \return VistaIOBundle
 */

EXPORT_VISTA VistaIOBundle VistaIOCreateBundle (VistaIOStringConst type_name, VistaIOAttrList list, size_t length,
		       VistaIOPointer data)
{
	VistaIOBundle b;

	b = VistaIOMalloc (sizeof (VistaIOBundleRec) + strlen (type_name));
	strcpy (b->type_name, type_name);
	b->list = list;
	b->length = length;
	b->data = data;
	return b;
}

/*! \brief Decode an attribute's value from a string to internal representation.
 *
 *  \param  str
 *  \param  dict
 *  \param  repn
 *  \param  value
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIODecodeAttrValue (VistaIOStringConst str, VistaIODictEntry * dict,
			   VistaIORepnKind repn, VistaIOPointer value)
{
	VistaIOLong i_value = 0;
	VistaIODouble f_value = 0.0;
	char *cp = NULL, buf[20];

	/* If a dict is provided, see if str maps to any dict entry keyword,
	   substituting the associated value if found: */
	if (dict) {
		dict = VistaIOLookupDictKeyword (dict, str);

		/* If there's a dictionary entry, complete it: */
		if (dict && !dict->svalue) {
			str = NULL;
			dict->icached = dict->fcached = TRUE;
			sprintf (buf, "%ld", (long)dict->ivalue);
			dict->svalue = VistaIONewString (buf);
			dict->fvalue = dict->ivalue;
		}
	}

	/* Convert to the internal representation: */
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOBooleanRepn:
		if (dict) {
			if (dict->icached)
				i_value = dict->ivalue;
			else {
				dict->ivalue = i_value =
					strtol (dict->svalue, &cp, 0);
				dict->icached = TRUE;
			}
		} else
			i_value = strtol (str, &cp, 0);
		break;

	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
		if (dict) {
			if (dict->fcached)
				f_value = dict->fvalue;
			else {
				dict->fvalue = f_value =
					strtod (dict->svalue, &cp);
				dict->fcached = TRUE;
			}
		} else
			f_value = strtod (str, &cp);
		break;

	case VistaIOStringRepn:
		if (dict)
			str = dict->svalue;
		break;

	default:
		VistaIOError ("VistaIODecodeAttrValue: Can't decode to %s",
			VistaIORepnName (repn));
	}
	if (cp && *cp)
		return FALSE;

	/* Store at *value: */
	switch (repn) {
	case VistaIOBitRepn:
		*(VistaIOBit *) value = i_value;
		break;
	case VistaIOUByteRepn:
		*(VistaIOUByte *) value = i_value;
		break;
	case VistaIOSByteRepn:
		*(VistaIOSByte *) value = i_value;
		break;
	case VistaIOShortRepn:
		*(VistaIOShort *) value = i_value;
		break;
	case VistaIOLongRepn:
		*(VistaIOLong *) value = i_value;
		break;
	case VistaIOFloatRepn:
		*(VistaIOFloat *) value = (VistaIOFloat)f_value;
		break;
	case VistaIODoubleRepn:
		*(VistaIODouble *) value = f_value;
		break;
	case VistaIOBooleanRepn:
		*(VistaIOBoolean *) value = i_value;
		break;
	case VistaIOStringRepn:
		*(VistaIOStringConst *) value = str;
		break;

	default:
		break;
	}

	return TRUE;
}


/*! \brief Delete an attribute identified by its position in an attribute list.
 *  
 *  The posn parameter is updated to point to the next in the list.
 * 
 *  \param posn
 */

EXPORT_VISTA void VistaIODeleteAttr (VistaIOAttrListPosn * posn)
{
	VistaIOAttrRec *a = posn->ptr;

	/* Remove it from the list: */
	if (a->next)
		a->next->prev = a->prev;
	if (a->prev)
		a->prev->next = a->next;
	if (posn->list->next == a)
		posn->list->next = a->next;
	if (posn->list->prev == a)
		posn->list->prev = a->prev;

	/* Make posn point to the next attribute, or nothing: */
	posn->ptr = a->next;

	VistaIOFree (a);
}


/*! \brief Discard a list of attributes.
 *
 *  \param  list
 */

EXPORT_VISTA void VistaIODestroyAttrList (VistaIOAttrList list)
{
	VistaIOAttrRec *a, *a_next;

	if (!list) {
		VistaIOWarning ("VistaIODestroyAttrList: Called with NULL list");
		return;
	}

	/* For each attribute in the list: */
	for (a = list->next; a; a = a_next) {
		a_next = a->next;

		/* Free any storage used for the attribute's value: */
		FreeAttrValue ("VistaIODestroyAttrList", a);

		/* Free the attribute record itself: */
		VistaIOFree (a);
	}
	VistaIOFree (list);
}


/*! \brief Discard a bundle.
 *
 *  \param b
 */

EXPORT_VISTA void VistaIODestroyBundle (VistaIOBundle b)
{
	VistaIODestroyAttrList (b->list);
	if (b->length > 0)
		VistaIOFree (b->data);
	VistaIOFree (b);
}


/*! \brief Encode an attribute's value from internal representaiton to a string.
 *  
 *  This is just a stub for Encode, which is shared by VistaIOSetAttr.
 *  The calling sequence is:
 *
 *	VistaIOEncodeAttrValue (VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations. It returns
 *  a pointer to an encoded string, valid until the next VistaIOEncodeAttrValue
 *  call.
 *
 *  \param  dict
 *  \param  repn
 *  \return  VistaIOStringConst
 */

EXPORT_VISTA VistaIOStringConst VistaIOEncodeAttrValue (VistaIODictEntry * dict, VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOStringConst str;

	va_start (args, repn);
	str = Encode (dict, repn, &args);
	va_end (args);
	return str;
}


/*
 *  Encode
 *
 *  Encode an attribute's value from internal representation to a string.
 *  Does the actual work of encoding (cf VistaIOEncodeAttrValue).
 */

static VistaIOStringConst Encode (VistaIODictEntry * dict, VistaIORepnKind repn, va_list * args)
{
	VistaIOLong i_value = 0;
	VistaIODouble f_value = 0.0;
	VistaIOString s_value = NULL;
	static char buf[40];

	/* Fetch the attribute value: */
	switch (repn) {
	case VistaIOBitRepn:
		i_value = va_arg (*args, VistaIOBitPromoted);
		break;
	case VistaIOUByteRepn:
		i_value = va_arg (*args, VistaIOUBytePromoted);
		break;
	case VistaIOSByteRepn:
		i_value = va_arg (*args, VistaIOSBytePromoted);
		break;
	case VistaIOShortRepn:
		i_value = va_arg (*args, VistaIOShortPromoted);
		break;
	case VistaIOLongRepn:
		i_value = va_arg (*args, VistaIOLongPromoted);
		break;
	case VistaIOFloatRepn:
		f_value = va_arg (*args, VistaIOFloatPromoted);
		break;
	case VistaIODoubleRepn:
		f_value = va_arg (*args, VistaIODoublePromoted);
		break;
	case VistaIOBooleanRepn:
		i_value = va_arg (*args, VistaIOBooleanPromoted);
		break;
	case VistaIOStringRepn:
		s_value = va_arg (*args, VistaIOString);
		break;

	default:
		VistaIOError ("VistaIOEncodeAttrValue: Can't encode from %s",
			VistaIORepnName (repn));
	}

	/* If its numeric, convert it to a string: */
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOBooleanRepn:
		sprintf (s_value = buf, "%ld", (long)i_value);
		break;

	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
		sprintf (s_value = buf, "%.20g", (double)f_value);
		break;

	default:
		break;
	}

	/* If a dictionary was supplied, try to map the encoded value to
	   a keyword: */
	if (dict)
		switch (repn) {

		case VistaIOBitRepn:
		case VistaIOUByteRepn:
		case VistaIOSByteRepn:
		case VistaIOShortRepn:
		case VistaIOLongRepn:
		case VistaIOBooleanRepn:
			dict = VistaIOLookupDictValue (dict, VistaIOLongRepn, i_value);
			break;

		case VistaIOFloatRepn:
		case VistaIODoubleRepn:
			dict = VistaIOLookupDictValue (dict, VistaIODoubleRepn, f_value);
			break;

		case VistaIOStringRepn:
			dict = VistaIOLookupDictValue (dict, VistaIOStringRepn, s_value);
			break;

		default:
			break;
		}
	return dict ? dict->keyword : s_value;
}


/*! \brief Fetch an attribute value, removing it from its attribute list if found.
 *
 *  \param  list
 *  \param  name
 *  \param  dict
 *  \param  repn 
 *  \param  value
 *  \param  required
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIOExtractAttr (VistaIOAttrList list, VistaIOStringConst name,
		       VistaIODictEntry * dict, VistaIORepnKind repn, VistaIOPointer value,
		       VistaIOBooleanPromoted required)
{
	VistaIOAttrListPosn posn;

	/* If the attribute is in the list... */
	if (VistaIOLookupAttr (list, name, &posn)) {

		if (value) {

			/* Get its value: */
			if (!VistaIOGetAttrValue (&posn, dict, repn, value)) {
				VistaIOWarning ("VistaIOExtractAttr: %s attribute has bad value", name);
				return FALSE;
			}

			/* Clone or hide the value if we're about to delete it: */
			if (repn == VistaIOStringRepn)
				*(VistaIOString *) value =
					VistaIONewString (*(VistaIOString *) value);
		}

		/* Remove it from the list: */
		VistaIODeleteAttr (&posn);
		return TRUE;
	}

	/* Otherwise complain if the attribute was a required one: */
	if (required)
		VistaIOWarning ("VistaIOExtractAttr: %s attribute missing", name);
	return !required;
}


/*! \brief Fetch an attribute value, given its name.
 *
 *  The value parameter should point to storage sufficient to hold a value
 *  of the kind described by repn. An optional dictionary, dict, can
 *  specify string -> value translations.
 *
 *  \param  list
 *  \param  name
 *  \param  dict
 *  \param  repn
 *  \param  value
 *  \return VistaIOGetAttrResult
 */

EXPORT_VISTA VistaIOGetAttrResult VistaIOGetAttr (VistaIOAttrList list, VistaIOStringConst name,
			 VistaIODictEntry * dict, VistaIORepnKind repn, VistaIOPointer value)
{
	VistaIOAttrListPosn posn;

	/* Look up the attribute name in the list: */
	if (!VistaIOLookupAttr (list, name, &posn))
		return VistaIOAttrMissing;

	/* Get its value in the specified representation: */
	return VistaIOGetAttrValue (&posn, dict, repn,
			      value) ? VistaIOAttrFound : VistaIOAttrBadValue;
}


/*! \brief Fetch an attribute value given its position within an attribute list.
 *
 *  The value parameter should point to storage sufficient to hold a value
 *  of the kind described by repn. An optional dictionary, dict, can
 *  specify string -> value translations. 
 *
 *  \param  posn
 *  \param  dict
 *  \param  repn
 *  \param  value
 *  \return Returns TRUE if successful, FALSE
 *          if the requested representation cannot be provided.
 */

EXPORT_VISTA VistaIOBoolean VistaIOGetAttrValue (VistaIOAttrListPosn * posn, VistaIODictEntry * dict,
			VistaIORepnKind repn, VistaIOPointer value)
{
	/* Convert it to the requested representation: */
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
	case VistaIOBooleanRepn:
	case VistaIOStringRepn:
		return (VistaIOGetAttrRepn (posn) == VistaIOStringRepn &&
			VistaIODecodeAttrValue (posn->ptr->value, dict, repn,
					  value));

	default:
		if (VistaIOGetAttrRepn (posn) != repn)
			return FALSE;
		*(VistaIOPointer *) value = posn->ptr->value;
		return TRUE;
	}
}


/*! \brief Insert a new attribute into a list, before or after a specified position.
 *
 *  The calling sequence is:
 *
 *	VistaIOInsertAttr (VistaIOAttrListPosn *posn, VistaIOBoolean after, VistaIOStringConst name,
 *		     VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations. If after is
 *  TRUE the new attribute is inserted after posn; otherwise, before it.
 *
 *  \param posn
 *  \param after
 *  \param name
 *  \param dict
 *  \param repn
 */

EXPORT_VISTA void VistaIOInsertAttr (VistaIOAttrListPosn * posn, VistaIOBooleanPromoted after,
		  VistaIOStringConst name, VistaIODictEntry * dict, VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOAttrRec *a;

	/* Create the new attribute node: */
	va_start (args, repn);
	a = NewAttr (name, dict, repn, &args);
	va_end (args);

	/* Insert it at the specified position: */
	if (!posn->ptr) {	/* the pointer points nowhere */
		if (((a->next) = (posn->list->next)))
			a->next->prev = a;
		a->prev = 0;
		posn->list->next = a;
		if (!posn->list->prev)
			posn->list->prev = a;
	} else if (after) {
		if (((a->next) = (posn->ptr->next)))
			a->next->prev = a;
		else
			posn->list->prev = a;
		a->prev = posn->ptr;
		a->prev->next = a;
		if (posn->list->prev == a->prev)
			posn->list->prev = a;
	} else {
		a->next = posn->ptr;
		if (((a->prev) = (posn->ptr->prev)))
			a->prev->next = a;
		else
			posn->list->next = a;
		a->next->prev = a;
		if (((posn->list->next) == (a->next)))
			posn->list->next = a;
	}
}


/*! \brief Lookup an attribute, by name, in a list of attributes.
 *
 *  \param  list
 *  \param  name
 *  \param  posn
 *  \return VistaIOBoolean
 */

EXPORT_VISTA VistaIOBoolean VistaIOLookupAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIOAttrListPosn * posn)
{
	for (VistaIOFirstAttr (list, posn); VistaIOAttrExists (posn); VistaIONextAttr (posn))
		if (strcmp (VistaIOGetAttrName (posn), name) == 0)
			return TRUE;
	return FALSE;
}


/*! \brief Prepend a new attribute to a list.
 *
 *  The calling sequence is:
 *
 *	VistaIOPrependAttr (VistaIOAttrList list, VistaIOStringConst name,
 *		      VistaIODictEntry *dict, VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param list
 *  \param name
 *  \param dict
 *  \param repn
 */

EXPORT_VISTA void VistaIOPrependAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry * dict,
		   VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOAttrRec *a;

	/* Create the new attribute node: */
	va_start (args, repn);
	a = NewAttr (name, dict, repn, &args);
	va_end (args);

	/* Prepend it: */
	if ((a->next = list->next))
		a->next->prev = a;
	else
		list->prev = a;
	a->prev = NULL;
	list->next = a;
}


/*! \brief Set an attribute value, where the attribute is specified by name.
 *
 *  The calling sequence is:
 *
 *	VistaIOSetAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry *dict,
 *		  VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param list
 *  \param name
 *  \param dict
 *  \param repn 
 */

EXPORT_VISTA void VistaIOSetAttr (VistaIOAttrList list, VistaIOStringConst name, VistaIODictEntry * dict,
	       VistaIORepnKind repn, ...)
{
	va_list args;
	VistaIOAttrListPosn posn;
	VistaIOAttrRec *a;

	/* Locate any existing attribute of the specified name: */
	va_start (args, repn);
	if (VistaIOLookupAttr (list, name, &posn))
		SetAttr (&posn, dict, repn, &args);
	else {

		/* None exists -- append a new attribute of that name: */
		a = NewAttr (name, dict, repn, &args);
		a->next = NULL;
		if ((a->prev = list->prev))
			a->prev->next = a;
		else
			list->next = a;
		list->prev = a;
	}
	va_end (args);
}


/*! \brief Set an attribute value, where the attribute is specified by its
 *  position in an attribute list.
 *
 *  The calling sequence is:
 *
 *	VistaIOSetAttrValue (VistaIOAttrListPosn *posn, VistaIODictEntry *dict,
 *		       VistaIORepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param posn
 *  \param dict
 *  \param repn
 */

EXPORT_VISTA void VistaIOSetAttrValue (VistaIOAttrListPosn * posn, VistaIODictEntry * dict, VistaIORepnKind repn,
		    ...)
{
	va_list args;

	/* Locate any existing attribute of the specified name: */
	va_start (args, repn);
	SetAttr (posn, dict, repn, &args);
	va_end (args);
}


/*
 *  NewAttr
 *
 *  Local routine performing common functions for VistaIOSetAttr, VistaIOInsertAttr,
 *  VistaIOAppendAttr, VistaIOPrependAttr.
 */

static VistaIOAttrRec *NewAttr (VistaIOStringConst name, VistaIODictEntry * dict,
			  VistaIORepnKind repn, va_list * args)
{
	size_t new_value_size, name_size;
	VistaIOPointer value;
	VistaIOAttrRec *a;

	name_size = strlen (name);
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
	case VistaIOBooleanRepn:
	case VistaIOStringRepn:

		/* Determine the amount of storage needed to record the new value.
		   In some cases, this requires first encoding the new value as a
		   string. */
		if (repn == VistaIOStringRepn && !dict)
			value = (VistaIOPointer) va_arg (*args, VistaIOStringConst);
		else
			value = (VistaIOPointer) Encode (dict, repn, args);
		new_value_size = strlen (value) + 1;

		/* Allocate storage for the new attribute and copy in its value: */
		a = VistaIOMalloc (sizeof (VistaIOAttrRec) + name_size + new_value_size);
		a->repn = VistaIOStringRepn;
		a->value = (a->name + name_size + 1);
		strcpy (a->value, value);
		break;

	default:
		a = VistaIOMalloc (sizeof (VistaIOAttrRec) + name_size);
		a->repn = repn;
		a->value = va_arg (*args, VistaIOPointer);
	}
	strcpy (a->name, name);
	return a;
}


/*
 *  SetAttr
 *
 *  Local routine performing common functions for VistaIOSetAttr and VistaIOSetAttrValue.
 *  VistaIOInsertAttr, VistaIOAppendAttr, VistaIOPrependAttr.
 */

static void SetAttr (VistaIOAttrListPosn * posn, VistaIODictEntry * dict, VistaIORepnKind repn,
		     va_list * args)
{
	size_t old_value_size, new_value_size, name_size;
	VistaIOPointer value;
	VistaIOAttrRec *a = posn->ptr;

	/* Determine the amount of storage needed to record the new value. In some
	   cases, this requires first encoding the new value as a string. */
	name_size = strlen (a->name);
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
	case VistaIOBooleanRepn:
	case VistaIOStringRepn:
		if (repn == VistaIOStringRepn && !dict)
			value = (VistaIOPointer) va_arg (*args, VistaIOStringConst);
		else
			value = (VistaIOPointer) Encode (dict, repn, args);
		new_value_size = strlen (value) + 1;
		break;

	default:
		value = va_arg (*args, VistaIOPointer);
		new_value_size = 0;
	}

	/* Is enough storage allocated for it in the existing attribute node? */
	switch (a->repn) {

	case VistaIOStringRepn:
		old_value_size = strlen (a->value) + 1;
		break;

	default:
		old_value_size = 0;
	}
	if (old_value_size < new_value_size) {

		/* It exists, but it's too small: */
		a = VistaIOMalloc (sizeof (VistaIOAttrRec) + name_size + new_value_size);
		a->next = posn->ptr->next;
		a->prev = posn->ptr->prev;
		if (a->next)
			a->next->prev = a;
		else
			posn->list->prev = a;
		if (a->prev)
			a->prev->next = a;
		else
			posn->list->next = a;
		strcpy (a->name, posn->ptr->name);
		VistaIOFree (posn->ptr);
		posn->ptr = a;
	}

	/* Copy in the attribute's new value: */
	switch (repn) {

	case VistaIOBitRepn:
	case VistaIOUByteRepn:
	case VistaIOSByteRepn:
	case VistaIOShortRepn:
	case VistaIOLongRepn:
	case VistaIOFloatRepn:
	case VistaIODoubleRepn:
	case VistaIOBooleanRepn:
	case VistaIOStringRepn:
		a->repn = VistaIOStringRepn;
		a->value = a->name + name_size + 1;
		strcpy (a->value, value);
		break;

	default:
		a->repn = repn;
		a->value = value;
	}
}


/*
 *  FreeAttrValue
 *
 *  Free storage occupied by an attribute's value.
 */

static void FreeAttrValue (VistaIOStringConst routine, VistaIOAttrRec * a)
{
	VistaIOTypeMethods *methods;

	switch (a->repn) {

	case VistaIOAttrListRepn:
		VistaIODestroyAttrList (a->value);
		break;

	case VistaIOBundleRepn:
		VistaIODestroyBundle (a->value);
		break;

	case VistaIOPointerRepn:
	case VistaIOStringRepn:
		break;

	default:
		if (!(methods = VistaIORepnMethods (a->repn)))
			VistaIOError ("%s: %s attribute has invalid repn %d",
				routine, a->name, a->repn);
		(methods->destroy) (a->value);
	}
}
