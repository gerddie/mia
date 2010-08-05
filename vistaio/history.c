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

/* $Id: history.c 885 2006-02-27 10:13:57Z write1 $ */

/*! \file  history.c
 *  \brief basic support for an history object
 *  \remark This file is part of the lipsia and via library, too.
 *  \author Gabi Lohmann, lohmann@cbs.mpg.de, 2004
 */
#define _BSD_SOURCE 

/* From the Vista library: */
#include "vistaio/vistaio.h"

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#define snprintf _snprintf
#endif


#define MAXPARAMLENGTH 400

char*
VGetOptionValue (VOptionDescRec *option)
{
  char *ret;
  int n, i;
  char *vp;
  VDictEntry *dict;
  VLong ivalue;
  VDouble fvalue = 0.0;
  VStringConst svalue;

  ret = (char*)malloc(MAXPARAMLENGTH);


  if (option->number == 0) {
    n = ((VArgVector *) option->value)->number;
    vp = (char *) ((VArgVector *) option->value)->vector;
  } else {
    n = option->number;
    vp = (char *) option->value;

  }
  for (i = 0; i < n; i++, vp += VRepnSize (option->repn)) {

    switch (option->repn) {

    case VBitRepn:
      ivalue = * (VBit *) vp;
      goto PrintLong;

    case VUByteRepn:
      ivalue = * (VUByte *) vp;
      goto PrintLong;

    case VSByteRepn:
      ivalue = * (VSByte *) vp;
      goto PrintLong;

    case VShortRepn:
      ivalue = * (VShort *) vp;
      goto PrintLong;

    case VLongRepn:
      ivalue = * (VLong *) vp;
    PrintLong:
      snprintf (ret, MAXPARAMLENGTH,"%d", ivalue);
      break;

    case VFloatRepn:
      fvalue = * (VFloat *) vp;
      goto PrintDbl;

    case VDoubleRepn:
      fvalue = * (VDouble *) vp;
    PrintDbl:
      snprintf (ret, MAXPARAMLENGTH, "%g", fvalue);
      break;

    case VBooleanRepn:
	    snprintf (ret,MAXPARAMLENGTH, "%s", * (VBoolean *) vp ? "true" : "false");
	    break;
    case VStringRepn:
	    svalue = * (VString *) vp;
      if (! svalue)
	      svalue = "(none)";
      else if (option->dict &&
               (dict = VLookupDictValue (option->dict, VDoubleRepn,
                                         svalue)))
	      svalue = dict->keyword;
      snprintf (ret, MAXPARAMLENGTH, "%s", svalue);
      break;

    default:
      break;
    }
  }

  return ret;
}

char*
VGetHistory(int noptions,VOptionDescRec *options,char *name)
{
  int i,k;
  char *history;
  char *item;
  char *tok, *tname, *sversion = NULL, *sname;
  char cmd[]="; ";
  char *ignore[] = { "in", "out", "ref", NULL };

  tname = strdup(name);

  tok = strtok(tname, ":");
  sname = strdup(tok);
  if (tok != NULL) {
    tok = strtok(NULL, " ");
    if (tok != NULL) {
      tok = strtok(NULL, " ");
      sversion = strdup(tok);
    }
    else VWarning("History function can not grab program version");
  }
  else VWarning("History function can not grab program name");


  history = (char*) malloc(sizeof(char*) * MAXPARAMLENGTH);
  history[0]='\0';
  strncat(history, sname, strlen(sname));
  strncat(history, " ", 1);
  strncat(history, sversion, strlen(sversion));

  strncat(history, cmd, strlen(cmd));

  for (i = 0; i < noptions; i++, options++) {
    for (k=0; ignore[k] != NULL; k++) {
      if (strncmp(options->keyword, ignore[k], strlen(ignore[k])) == 0)
	goto out;
    }
    item = VGetOptionValue (options);
    strncat(history, "-", 1);
    strncat(history, options->keyword, strlen(options->keyword));
    strncat(history, " ", 1);
    strncat(history, item, strlen(item));
    if	(i+1<noptions)
      strncat(history, " ", 1);
  out:
    /* cannot happen */
    break;

  }

  return history;
}

VAttrList
VReadHistory(VAttrList *list)
{
  int i = 0;
  VStringConst temponame;
  VAttrListPosn posn;
  VAttrList history_list=NULL;
  char *temptok, history[]="history";
  VString str;

  for (VLastAttr((*list),&posn);VAttrExists(&posn);VPrevAttr(&posn)) {

    if (strncmp(VGetAttrName(&posn), history, strlen(history)) != 0 )
      continue;

    /* old history format by SH */
    if (VGetAttrRepn(&posn) == VStringRepn ) {
      if (history_list==NULL) history_list = VCreateAttrList();
      VGetAttrValue(&posn, NULL, VStringRepn, &str);
      temptok=strtok(str, " ");
      temponame = strdup(temptok);
      temptok = strtok(NULL, "\0");
      VPrependAttr(history_list, temponame, NULL, VStringRepn, temptok);
      i++;
    }
    /* new history format */
    if (VGetAttrRepn(&posn) == VAttrListRepn ) {
      if (i>0) VError("type mismatch while reading history");
      VGetAttrValue(&posn, NULL, VAttrListRepn, &history_list);
      break;
    }
  }

  /* Warning */
  if (history_list == NULL)
    VWarning("VReadHistory: No history items found");

  return history_list;
}

void
VPrependHistory(int noptions,VOptionDescRec *options,char *name,VAttrList *list)
{
  char *tok;
  char *newhistory;
  VStringConst oname;

  /* Generate the new history entry */
  if ((newhistory = VGetHistory(noptions,options,name)) == NULL)
    VError("Error while building history string\n");

  tok = strtok(newhistory, " ");
  oname = strdup(tok);
  tok = strtok(NULL, "\0");

  /* Prepend history list */
  if ((*list) == NULL) (*list) = VCreateAttrList();
  VPrependAttr( (*list) ,oname, NULL, VStringRepn, tok);
}

void
VHistory(int noptions,VOptionDescRec *options,char *name,VAttrList *in_list,VAttrList *out_list)
{
   VAttrList history_list=NULL;
  VAttrListPosn posn;
  /*VBoolean sw=FALSE;*/
  char history[]="history";
  /*VString str; */

  /* Read history from list */
  history_list = VReadHistory(in_list);

  /* Prepend new history entry */
  VPrependHistory(noptions,options,name,&history_list);

  /* DELETE ANY history attributes in dest */
  for (VLastAttr((*out_list),&posn);VAttrExists(&posn);VPrevAttr(&posn)) {
    if (strncmp(VGetAttrName(&posn), history, strlen(history)) == 0 )
      VDeleteAttr(&posn);
  }

  /* Prepend history in dest */
  VPrependAttr( (*out_list),history,NULL,VAttrListRepn,history_list);

}
