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

/*$Id: list.c 52 2004-03-02 15:53:19Z tittge $ */

/*! \file  list.c
 *  \brief routines for manipulating VList instances
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

/* Later in this file: */
static VNodePtrType MakeNode (VPointer item, VNodePtrType prev,
			      VNodePtrType next);


/*
 * MakeNode
 *
 * Make a node
 */

static VNodePtrType MakeNode (VPointer item, VNodePtrType prev,
			      VNodePtrType next)
{
	VNodePtrType result = VMalloc (sizeof (struct V_Node));

	result->item = item;
	result->prev = prev;
	result->next = next;

	return result;
}


/*! \brief Make a new, empty list, and returns its reference.
 *
 *  \return VList
 */

VList VListCreate (void)
{
	VList vlist = VMalloc (sizeof (struct V_List));
	VNodePtrType dummy_head, dummy_tail;

	dummy_head = VMalloc (sizeof (struct V_Node));

	dummy_tail = VMalloc (sizeof (struct V_Node));

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
 *  \return VPointer
 */

VPointer VListFirst (VList vlist)
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
 *  \return VPointer
 */

VPointer VListLast (VList vlist)
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
 *  \return VPointer
 */

VPointer VListNext (VList vlist)
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
 *  \return VPointer
 */

VPointer VListPrev (VList vlist)
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

void VListAdd (VList vlist, VPointer item)
{
	VNodePtrType add_me;

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

void VListInsert (VList vlist, VPointer item)
{
	VNodePtrType add_me;

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

void VListAppend (VList vlist, VPointer item)
{
	vlist->current = vlist->tail;	/* move beyond end */
	VListAdd (vlist, item);
}


/*! \brief Add item to the beginning of vlist, and make
 *         item the current item.
 *
 *  \param vlist
 *  \param item 
 */

void VListPrepend (VList vlist, VPointer item)
{
	vlist->current = vlist->head;	/* move before beginning */
	VListAdd (vlist, item);
}


/*! \brief Return current item and take it out of vlist.
 * 
 *  Make the next item the current one.
 *
 *  \param  vlist
 *  \return VPointer
 */

VPointer VListRemove (VList vlist)
{
	VPointer return_me;
	VNodePtrType free_me;

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

		VFree (free_me);
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

void VListConcat (VList vlist1, VList vlist2)
{
	VNodePtrType free_me, free_me_too;

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

	VFree (free_me);
	VFree (free_me_too);
	VFree (vlist2);
}


/*! \brief Delete vlist. 
 *
 *  \param vlist
 *  \param item_free A pointer to a routine that frees an item.
 */

void VListDestroy (VList vlist, void (*item_free) ())
{
	VPointer free_me;

	vlist->current = vlist->head->next;
	while (vlist->current != vlist->tail) {
		free_me = VListRemove (vlist);
		(*item_free) (free_me);
	}

	VFree (vlist->head);
	VFree (vlist->tail);
	VFree (vlist);
}


/*! \brief Return last item and take it out of vlist. 
 *
 *  Make the new last item the current one.
 *  
 *  \param  vlist
 *  \return VPointer
 */

VPointer VListTrim (VList vlist)
{
	VPointer return_me;

	return_me = VListLast (vlist);
	VListRemove (vlist);
	VListLast (vlist);

	return return_me;
}


/*! \brief Searche vlist starting at the current item until the end is 
 *         reached or a match is found.
 *
 *  \param  vlist
 *  \param  comp
 *  \param  comp_arg
 *  \return VPointer
 */

VPointer VListSearch (VList vlist, int (*comp) (), VPointer comp_arg)
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
