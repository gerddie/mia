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

/* $Id: attr.c 889 2006-03-03 12:47:24Z write1 $ */

/*! \file  attr.c
 *  \brief routines for manipulating lists of attributes 
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vista.h"

/* Later in this file: */
static VStringConst Encode (VDictEntry * dict, VRepnKind repn, va_list * args);
static VAttrRec *NewAttr (VStringConst, VDictEntry *, VRepnKind, va_list *);
static void SetAttr (VAttrListPosn *, VDictEntry *, VRepnKind, va_list *);
static void FreeAttrValue (VStringConst, VAttrRec *);


/*! \brief Append a new attribute to a list.
 *
 *  The calling sequence is:
 *
 *	VAppendAttr (VAttrList list, VStringConst name,
 *		     VDictEntry *dict, VRepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param  list
 *  \param  name
 *  \param  dict
 *  \param  repn
 */

EXPORT_VISTA void VAppendAttr (VAttrList list, VStringConst name, VDictEntry * dict,
		  VRepnKind repn, ...)
{
	va_list args;
	VAttrRec *a;

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
 *  \return VAttrList
 */

EXPORT_VISTA VAttrList VCopyAttrList (VAttrList list)
{
	VAttrList new_list = VCreateAttrList ();
	size_t name_size, value_size;
	VAttrRec *old_a, *new_a;
	VBundle old_b, new_b;
	VTypeMethods *methods;

	/* For each node of the old list: */
	for (old_a = list->next; old_a; old_a = old_a->next) {

		/* Compute the amount of storage needed for a copy of the node: */
		name_size = strlen (old_a->name);
		value_size = (old_a->repn == VStringRepn) ?
			strlen ((VStringConst) old_a->value) + 1 : 0;

		/* Allocate that size and fill in the node's value: */
		new_a = VMalloc (sizeof (VAttrRec) + name_size + value_size);
		strcpy (new_a->name, old_a->name);
		switch (new_a->repn = old_a->repn) {

		case VAttrListRepn:
			new_a->value = VCopyAttrList (old_a->value);
			break;

		case VBundleRepn:
			old_b = old_a->value;
			new_b = VCreateBundle (old_b->type_name,
					       VCopyAttrList (old_b->list),
					       old_b->length, NULL);
			if (old_b->length > 0) {
				new_b->data = VMalloc (old_b->length);
				memcpy (new_b->data, old_b->data,
					old_b->length);
			}
			new_a->value = new_b;
			break;

		case VPointerRepn:
			new_a->value = old_a->value;
			break;

		case VStringRepn:
			new_a->value =
				(VPointer) (new_a->name + name_size + 1);
			strcpy (new_a->value, old_a->value);
			break;

		default:
		  if ((methods = VRepnMethods (new_a->repn)))
				new_a->value = (methods->copy) (old_a->value);
			else
				VError ("VCopyAttrList: %s attribute has invalid repn %d", old_a->name, old_a->repn);
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

EXPORT_VISTA VAttrList VCreateAttrList (void)
{
	VAttrList list;

	list = VNew (VAttrRec);
	list->next = list->prev = list->value = NULL;
	list->repn = VUnknownRepn;	/* not mistakable for an attribute */
	list->name[0] = 0;
	return list;
}


/*! \brief Create a bundle object.
 *
 *  \param  type_name
 *  \param  list
 *  \param  length
 *  \param  data
 *  \return VBundle
 */

EXPORT_VISTA VBundle VCreateBundle (VStringConst type_name, VAttrList list, size_t length,
		       VPointer data)
{
	VBundle b;

	b = VMalloc (sizeof (VBundleRec) + strlen (type_name));
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
 *  \return VBoolean
 */

EXPORT_VISTA VBoolean VDecodeAttrValue (VStringConst str, VDictEntry * dict,
			   VRepnKind repn, VPointer value)
{
	VLong i_value = 0;
	VDouble f_value = 0.0;
	char *cp = NULL, buf[20];

	/* If a dict is provided, see if str maps to any dict entry keyword,
	   substituting the associated value if found: */
	if (dict) {
		dict = VLookupDictKeyword (dict, str);

		/* If there's a dictionary entry, complete it: */
		if (dict && !dict->svalue) {
			str = NULL;
			dict->icached = dict->fcached = TRUE;
			sprintf (buf, "%ld", (long)dict->ivalue);
			dict->svalue = VNewString (buf);
			dict->fvalue = dict->ivalue;
		}
	}

	/* Convert to the internal representation: */
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VBooleanRepn:
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

	case VFloatRepn:
	case VDoubleRepn:
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

	case VStringRepn:
		if (dict)
			str = dict->svalue;
		break;

	default:
		VError ("VDecodeAttrValue: Can't decode to %s",
			VRepnName (repn));
	}
	if (cp && *cp)
		return FALSE;

	/* Store at *value: */
	switch (repn) {
	case VBitRepn:
		*(VBit *) value = i_value;
		break;
	case VUByteRepn:
		*(VUByte *) value = i_value;
		break;
	case VSByteRepn:
		*(VSByte *) value = i_value;
		break;
	case VShortRepn:
		*(VShort *) value = i_value;
		break;
	case VLongRepn:
		*(VLong *) value = i_value;
		break;
	case VFloatRepn:
		*(VFloat *) value = (VFloat)f_value;
		break;
	case VDoubleRepn:
		*(VDouble *) value = f_value;
		break;
	case VBooleanRepn:
		*(VBoolean *) value = i_value;
		break;
	case VStringRepn:
		*(VStringConst *) value = str;
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

EXPORT_VISTA void VDeleteAttr (VAttrListPosn * posn)
{
	VAttrRec *a = posn->ptr;

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

	VFree (a);
}


/*! \brief Discard a list of attributes.
 *
 *  \param  list
 */

EXPORT_VISTA void VDestroyAttrList (VAttrList list)
{
	VAttrRec *a, *a_next;

	if (!list) {
		VWarning ("VDestroyAttrList: Called with NULL list");
		return;
	}

	/* For each attribute in the list: */
	for (a = list->next; a; a = a_next) {
		a_next = a->next;

		/* Free any storage used for the attribute's value: */
		FreeAttrValue ("VDestroyAttrList", a);

		/* Free the attribute record itself: */
		VFree (a);
	}
	VFree (list);
}


/*! \brief Discard a bundle.
 *
 *  \param b
 */

EXPORT_VISTA void VDestroyBundle (VBundle b)
{
	VDestroyAttrList (b->list);
	if (b->length > 0)
		VFree (b->data);
	VFree (b);
}


/*! \brief Encode an attribute's value from internal representaiton to a string.
 *  
 *  This is just a stub for Encode, which is shared by VSetAttr.
 *  The calling sequence is:
 *
 *	VEncodeAttrValue (VDictEntry *dict, VRepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations. It returns
 *  a pointer to an encoded string, valid until the next VEncodeAttrValue
 *  call.
 *
 *  \param  dict
 *  \param  repn
 *  \return  VStringConst
 */

EXPORT_VISTA VStringConst VEncodeAttrValue (VDictEntry * dict, VRepnKind repn, ...)
{
	va_list args;
	VStringConst str;

	va_start (args, repn);
	str = Encode (dict, repn, &args);
	va_end (args);
	return str;
}


/*
 *  Encode
 *
 *  Encode an attribute's value from internal representation to a string.
 *  Does the actual work of encoding (cf VEncodeAttrValue).
 */

static VStringConst Encode (VDictEntry * dict, VRepnKind repn, va_list * args)
{
	VLong i_value = 0;
	VDouble f_value = 0.0;
	VString s_value = NULL;
	static char buf[40];

	/* Fetch the attribute value: */
	switch (repn) {
	case VBitRepn:
		i_value = va_arg (*args, VBitPromoted);
		break;
	case VUByteRepn:
		i_value = va_arg (*args, VUBytePromoted);
		break;
	case VSByteRepn:
		i_value = va_arg (*args, VSBytePromoted);
		break;
	case VShortRepn:
		i_value = va_arg (*args, VShortPromoted);
		break;
	case VLongRepn:
		i_value = va_arg (*args, VLongPromoted);
		break;
	case VFloatRepn:
		f_value = va_arg (*args, VFloatPromoted);
		break;
	case VDoubleRepn:
		f_value = va_arg (*args, VDoublePromoted);
		break;
	case VBooleanRepn:
		i_value = va_arg (*args, VBooleanPromoted);
		break;
	case VStringRepn:
		s_value = va_arg (*args, VString);
		break;

	default:
		VError ("VEncodeAttrValue: Can't encode from %s",
			VRepnName (repn));
	}

	/* If its numeric, convert it to a string: */
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VBooleanRepn:
		sprintf (s_value = buf, "%ld", (long)i_value);
		break;

	case VFloatRepn:
	case VDoubleRepn:
		sprintf (s_value = buf, "%.20g", (double)f_value);
		break;

	default:
		break;
	}

	/* If a dictionary was supplied, try to map the encoded value to
	   a keyword: */
	if (dict)
		switch (repn) {

		case VBitRepn:
		case VUByteRepn:
		case VSByteRepn:
		case VShortRepn:
		case VLongRepn:
		case VBooleanRepn:
			dict = VLookupDictValue (dict, VLongRepn, i_value);
			break;

		case VFloatRepn:
		case VDoubleRepn:
			dict = VLookupDictValue (dict, VDoubleRepn, f_value);
			break;

		case VStringRepn:
			dict = VLookupDictValue (dict, VStringRepn, s_value);
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
 *  \return VBoolean
 */

EXPORT_VISTA VBoolean VExtractAttr (VAttrList list, VStringConst name,
		       VDictEntry * dict, VRepnKind repn, VPointer value,
		       VBooleanPromoted required)
{
	VAttrListPosn posn;

	/* If the attribute is in the list... */
	if (VLookupAttr (list, name, &posn)) {

		if (value) {

			/* Get its value: */
			if (!VGetAttrValue (&posn, dict, repn, value)) {
				VWarning ("VExtractAttr: %s attribute has bad value", name);
				return FALSE;
			}

			/* Clone or hide the value if we're about to delete it: */
			if (repn == VStringRepn)
				*(VString *) value =
					VNewString (*(VString *) value);
		}

		/* Remove it from the list: */
		VDeleteAttr (&posn);
		return TRUE;
	}

	/* Otherwise complain if the attribute was a required one: */
	if (required)
		VWarning ("VExtractAttr: %s attribute missing", name);
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
 *  \return VGetAttrResult
 */

EXPORT_VISTA VGetAttrResult VGetAttr (VAttrList list, VStringConst name,
			 VDictEntry * dict, VRepnKind repn, VPointer value)
{
	VAttrListPosn posn;

	/* Look up the attribute name in the list: */
	if (!VLookupAttr (list, name, &posn))
		return VAttrMissing;

	/* Get its value in the specified representation: */
	return VGetAttrValue (&posn, dict, repn,
			      value) ? VAttrFound : VAttrBadValue;
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

EXPORT_VISTA VBoolean VGetAttrValue (VAttrListPosn * posn, VDictEntry * dict,
			VRepnKind repn, VPointer value)
{
	/* Convert it to the requested representation: */
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VFloatRepn:
	case VDoubleRepn:
	case VBooleanRepn:
	case VStringRepn:
		return (VGetAttrRepn (posn) == VStringRepn &&
			VDecodeAttrValue (posn->ptr->value, dict, repn,
					  value));

	default:
		if (VGetAttrRepn (posn) != repn)
			return FALSE;
		*(VPointer *) value = posn->ptr->value;
		return TRUE;
	}
}


/*! \brief Insert a new attribute into a list, before or after a specified position.
 *
 *  The calling sequence is:
 *
 *	VInsertAttr (VAttrListPosn *posn, VBoolean after, VStringConst name,
 *		     VDictEntry *dict, VRepnKind repn, xxx value)
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

EXPORT_VISTA void VInsertAttr (VAttrListPosn * posn, VBooleanPromoted after,
		  VStringConst name, VDictEntry * dict, VRepnKind repn, ...)
{
	va_list args;
	VAttrRec *a;

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
 *  \return VBoolean
 */

EXPORT_VISTA VBoolean VLookupAttr (VAttrList list, VStringConst name, VAttrListPosn * posn)
{
	for (VFirstAttr (list, posn); VAttrExists (posn); VNextAttr (posn))
		if (strcmp (VGetAttrName (posn), name) == 0)
			return TRUE;
	return FALSE;
}


/*! \brief Prepend a new attribute to a list.
 *
 *  The calling sequence is:
 *
 *	VPrependAttr (VAttrList list, VStringConst name,
 *		      VDictEntry *dict, VRepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param list
 *  \param name
 *  \param dict
 *  \param repn
 */

EXPORT_VISTA void VPrependAttr (VAttrList list, VStringConst name, VDictEntry * dict,
		   VRepnKind repn, ...)
{
	va_list args;
	VAttrRec *a;

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
 *	VSetAttr (VAttrList list, VStringConst name, VDictEntry *dict,
 *		  VRepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param list
 *  \param name
 *  \param dict
 *  \param repn 
 */

EXPORT_VISTA void VSetAttr (VAttrList list, VStringConst name, VDictEntry * dict,
	       VRepnKind repn, ...)
{
	va_list args;
	VAttrListPosn posn;
	VAttrRec *a;

	/* Locate any existing attribute of the specified name: */
	va_start (args, repn);
	if (VLookupAttr (list, name, &posn))
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
 *	VSetAttrValue (VAttrListPosn *posn, VDictEntry *dict,
 *		       VRepnKind repn, xxx value)
 *
 *  where xxx depends on the kind of representation, repn. An optional
 *  dictionary, dict, can specify value -> string translations.
 *
 *  \param posn
 *  \param dict
 *  \param repn
 */

EXPORT_VISTA void VSetAttrValue (VAttrListPosn * posn, VDictEntry * dict, VRepnKind repn,
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
 *  Local routine performing common functions for VSetAttr, VInsertAttr,
 *  VAppendAttr, VPrependAttr.
 */

static VAttrRec *NewAttr (VStringConst name, VDictEntry * dict,
			  VRepnKind repn, va_list * args)
{
	size_t new_value_size, name_size;
	VPointer value;
	VAttrRec *a;

	name_size = strlen (name);
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VFloatRepn:
	case VDoubleRepn:
	case VBooleanRepn:
	case VStringRepn:

		/* Determine the amount of storage needed to record the new value.
		   In some cases, this requires first encoding the new value as a
		   string. */
		if (repn == VStringRepn && !dict)
			value = (VPointer) va_arg (*args, VStringConst);
		else
			value = (VPointer) Encode (dict, repn, args);
		new_value_size = strlen (value) + 1;

		/* Allocate storage for the new attribute and copy in its value: */
		a = VMalloc (sizeof (VAttrRec) + name_size + new_value_size);
		a->repn = VStringRepn;
		a->value = (a->name + name_size + 1);
		strcpy (a->value, value);
		break;

	default:
		a = VMalloc (sizeof (VAttrRec) + name_size);
		a->repn = repn;
		a->value = va_arg (*args, VPointer);
	}
	strcpy (a->name, name);
	return a;
}


/*
 *  SetAttr
 *
 *  Local routine performing common functions for VSetAttr and VSetAttrValue.
 *  VInsertAttr, VAppendAttr, VPrependAttr.
 */

static void SetAttr (VAttrListPosn * posn, VDictEntry * dict, VRepnKind repn,
		     va_list * args)
{
	size_t old_value_size, new_value_size, name_size;
	VPointer value;
	VAttrRec *a = posn->ptr;

	/* Determine the amount of storage needed to record the new value. In some
	   cases, this requires first encoding the new value as a string. */
	name_size = strlen (a->name);
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VFloatRepn:
	case VDoubleRepn:
	case VBooleanRepn:
	case VStringRepn:
		if (repn == VStringRepn && !dict)
			value = (VPointer) va_arg (*args, VStringConst);
		else
			value = (VPointer) Encode (dict, repn, args);
		new_value_size = strlen (value) + 1;
		break;

	default:
		value = va_arg (*args, VPointer);
		new_value_size = 0;
	}

	/* Is enough storage allocated for it in the existing attribute node? */
	switch (a->repn) {

	case VStringRepn:
		old_value_size = strlen (a->value) + 1;
		break;

	default:
		old_value_size = 0;
	}
	if (old_value_size < new_value_size) {

		/* It exists, but it's too small: */
		a = VMalloc (sizeof (VAttrRec) + name_size + new_value_size);
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
		VFree (posn->ptr);
		posn->ptr = a;
	}

	/* Copy in the attribute's new value: */
	switch (repn) {

	case VBitRepn:
	case VUByteRepn:
	case VSByteRepn:
	case VShortRepn:
	case VLongRepn:
	case VFloatRepn:
	case VDoubleRepn:
	case VBooleanRepn:
	case VStringRepn:
		a->repn = VStringRepn;
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

static void FreeAttrValue (VStringConst routine, VAttrRec * a)
{
	VTypeMethods *methods;

	switch (a->repn) {

	case VAttrListRepn:
		VDestroyAttrList (a->value);
		break;

	case VBundleRepn:
		VDestroyBundle (a->value);
		break;

	case VPointerRepn:
	case VStringRepn:
		break;

	default:
		if (!(methods = VRepnMethods (a->repn)))
			VError ("%s: %s attribute has invalid repn %d",
				routine, a->name, a->repn);
		(methods->destroy) (a->value);
	}
}
