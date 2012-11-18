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

/* Later in this file: */
static VistaIONodePtrType MakeNode (VistaIOPointer item, VistaIONodePtrType prev,
			      VistaIONodePtrType next);


/*
 * MakeNode
 *
 * Make a node
 */

static VistaIONodePtrType MakeNode (VistaIOPointer item, VistaIONodePtrType prev,
			      VistaIONodePtrType next)
{
	VistaIONodePtrType result = VistaIOMalloc (sizeof (struct VistaIO_Node));

	result->item = item;
	result->prev = prev;
	result->next = next;

	return result;
}


/*! \brief Make a new, empty list, and returns its reference.
 *
 *  \return VistaIOList
 */

VistaIOList VistaIOListCreate (void)
{
	VistaIOList vlist = VistaIOMalloc (sizeof (struct VistaIO_List));
	VistaIONodePtrType dummy_head, dummy_tail;

	dummy_head = VistaIOMalloc (sizeof (struct VistaIO_Node));

	dummy_tail = VistaIOMalloc (sizeof (struct VistaIO_Node));

	dummy_head->item = NULL;
	dummy_head->prev = NULL;
	dummy_head->next = dummy_tail;

	dummy_tail->item = NULL;
	dummy_tail->prev = dummy_head;
	dummy_tail->next = NULL;

	vlist->head = dummy_head;
	vlist->tail = dummy_tail;
	vlist->current = dummy_head;
	vlist->count = 0;

	return vlist;
}


/*! \brief Return a pointer to the first item in vlist, 
 *         and make the first item the current item.
 *
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListFirst (VistaIOList vlist)
{
	if (vlist->count == 0)	/* empty vist, move beyond beginning */
		vlist->current = vlist->head;
	else			/* vlist not empty, move to beginning */
		vlist->current = vlist->head->next;

	return vlist->current->item;
}


/*! \brief Return a pointer to the last item in vlist,
 *         and make the last item the current item.
 *
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListLast (VistaIOList vlist)
{
	if (vlist->count == 0)	/* empty vlist, move beyond end */
		vlist->current = vlist->tail;
	else			/* vlist not empty, move to end */
		vlist->current = vlist->tail->prev;

	return vlist->current->item;
}


/*! \brief Advance vlist's current item by one, return the
 *         new current item. Return NULL if the new current
 *         item is beyond the end of vlist.
 *
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListNext (VistaIOList vlist)
{
	if (vlist->current == vlist->tail)
		/* already beyond end, no action */
		;
	else			/* move to next node */
		vlist->current = vlist->current->next;

	return vlist->current->item;
}


/*! \brief Back up vlist's current item by one, return the
 *         new current item. Return NULL if the new current
 *         item is before the beginning of vlist.
 *
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListPrev (VistaIOList vlist)
{
	if (vlist->current == vlist->head)
		/* already before beginning, no action */
		;
	else			/* move to previous node */
		vlist->current = vlist->current->prev;

	return vlist->current->item;
}


/*! \brief Add item to vlist immediately after the current item, 
 *
 *  and make item the current item. If the
 *  current pointer is before the beginning of vlist,
 *  item is added at the beginning. If the current
 *  pointer is beyond the end of vlist, item is
 *  added at the end.
 *
 *  \param  vlist
 *  \param  item
 */

void VistaIOListAdd (VistaIOList vlist, VistaIOPointer item)
{
	VistaIONodePtrType add_me;

	if (vlist->current == vlist->tail)
		/* current pointer beyond end, add to end */
		vlist->current = vlist->tail->prev;

	add_me = MakeNode (item, vlist->current, vlist->current->next);

	add_me->prev->next = add_me;
	add_me->next->prev = add_me;

	vlist->current = add_me;
	vlist->count++;
}


/*! \brief Add item to vlist immediately before the current item, 
 *
 *  and make item the current item. If the current pointer is before 
 *  the beginning of vlist, item is added at the beginning. If the current
 *  pointer is beyond the end of vlist, item is added at the end.
 *
 *  \param  vlist
 *  \param item
 */

void VistaIOListInsert (VistaIOList vlist, VistaIOPointer item)
{
	VistaIONodePtrType add_me;

	if (vlist->current == vlist->head)
		/* current pointer before beginning, add to beginning */
		vlist->current = vlist->head->next;

	add_me = MakeNode (item, vlist->current->prev, vlist->current);

	add_me->prev->next = add_me;
	add_me->next->prev = add_me;

	vlist->current = add_me;
	vlist->count++;
}


/*! \brief Add item to the end of vlist, and make item the current item.
 *
 *  \param vlist
 *  \param item
 */

void VistaIOListAppend (VistaIOList vlist, VistaIOPointer item)
{
	vlist->current = vlist->tail;	/* move beyond end */
	VistaIOListAdd (vlist, item);
}


/*! \brief Add item to the beginning of vlist, and make
 *         item the current item.
 *
 *  \param vlist
 *  \param item 
 */

void VistaIOListPrepend (VistaIOList vlist, VistaIOPointer item)
{
	vlist->current = vlist->head;	/* move before beginning */
	VistaIOListAdd (vlist, item);
}


/*! \brief Return current item and take it out of vlist.
 * 
 *  Make the next item the current one.
 *
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListRemove (VistaIOList vlist)
{
	VistaIOPointer return_me;
	VistaIONodePtrType free_me;

	return_me = vlist->current->item;

	if ((vlist->current == vlist->tail)
	    || (vlist->current == vlist->head))
		/* current pointer before beginning or beyond end, no action */
		;
	else {			/* free current node */

		vlist->current->prev->next = vlist->current->next;
		vlist->current->next->prev = vlist->current->prev;
		free_me = vlist->current;
		vlist->current = vlist->current->next;

		VistaIOFree (free_me);
		vlist->count--;
	}

	return return_me;
}


/*! \brief Add vlist2 to the end of vlist1. 
 *
 *  The current pointer is set to the current pointer of vlist1.
 *  vlist2 no longer exists after the operation.
 *
 *  \param  vlist1
 *  \param  vlist2
 */

void VistaIOListConcat (VistaIOList vlist1, VistaIOList vlist2)
{
	VistaIONodePtrType free_me, free_me_too;

	free_me = vlist1->tail;
	free_me_too = vlist2->head;

	vlist1->tail->prev->next = vlist2->head->next;
	vlist2->head->next->prev = vlist1->tail->prev;

	if (vlist1->current == vlist1->tail)
		/* current pointer of vlist1 points beyond end,
		   set it to first node of vlist2 */
		vlist1->current = vlist2->head->next;

	vlist1->tail = vlist2->tail;
	vlist1->count += vlist2->count;

	VistaIOFree (free_me);
	VistaIOFree (free_me_too);
	VistaIOFree (vlist2);
}


/*! \brief Delete vlist. 
 *
 *  \param vlist
 *  \param item_free A pointer to a routine that frees an item.
 */

void VistaIOListDestroy (VistaIOList vlist, void (*item_free) ())
{
	VistaIOPointer free_me;

	vlist->current = vlist->head->next;
	while (vlist->current != vlist->tail) {
		free_me = VistaIOListRemove (vlist);
		(*item_free) (free_me);
	}

	VistaIOFree (vlist->head);
	VistaIOFree (vlist->tail);
	VistaIOFree (vlist);
}


/*! \brief Return last item and take it out of vlist. 
 *
 *  Make the new last item the current one.
 *  
 *  \param  vlist
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListTrim (VistaIOList vlist)
{
	VistaIOPointer return_me;

	return_me = VistaIOListLast (vlist);
	VistaIOListRemove (vlist);
	VistaIOListLast (vlist);

	return return_me;
}


/*! \brief Searche vlist starting at the current item until the end is 
 *         reached or a match is found.
 *
 *  \param  vlist
 *  \param  comp
 *  \param  comp_arg
 *  \return VistaIOPointer
 */

VistaIOPointer VistaIOListSearch (VistaIOList vlist, int (*comp) (), VistaIOPointer comp_arg)
{
	if (vlist->current == vlist->head)
		/* before beginning, go to next node */
		vlist->current = vlist->current->next;

	while (vlist->current != vlist->tail) {
		if ((*comp) (vlist->current->item, comp_arg))
			/* a match is found */
			return (vlist->current->item);
		else
			vlist->current = vlist->current->next;
	}

	/* no match */
	return vlist->current->item;
}
