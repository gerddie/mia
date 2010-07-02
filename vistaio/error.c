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

/* $Id: error.c 461 2004-11-01 16:02:16Z wollny $ */

/*! \file  error.c
 *  \brief routines for reporting errors
 *  \author Arthur Pope, UBC Laboratory for Computational Intelligentce
 */

#include "vistaio/vistaio.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

/* Name of the program, for use in error messages: */
static VStringConst programName;

/* Routines to call to report errors: */
static VErrorHandler *errorHandler = VDefaultError;
static VWarningHandler *warningHandler = VDefaultWarning;

/* Maximum length of an error message: */
#define maxErrorMessageLength	500

int VERBOSE = 1; 

/* Later in this file: */
static void FormatMsg (char *, VStringConst, VStringConst, va_list *,
		       VStringConst);


/*! \brief Establish a caller-supplied routine as the handler for error messages.
 *
 *  \param fnc
 */

EXPORT_VISTA void VSetErrorHandler (VErrorHandler * fnc)
{
	errorHandler = fnc ? fnc : VDefaultError;
}


/*! \brief Establish a caller-supplied routine as the handler for warning messages.
 *
 *  \param  fnc
 */

EXPORT_VISTA void VSetWarningHandler (VWarningHandler * fnc)
{
	warningHandler = fnc ? fnc : VDefaultWarning;
}


/*! \brief Note the program name for use in error messages.
 *
 *  \param name
 */

EXPORT_VISTA void VSetProgramName (VStringConst name)
{
	programName = name;
}


/*! \brief Report a fatal program error.
 *
 *  \param format
 */

EXPORT_VISTA void VError (VStringConst format, ...)
{
	va_list args;
	char buf[maxErrorMessageLength + 3];

	va_start (args, format);
	FormatMsg (buf, "Fatal", format, &args, 0);
	va_end (args);
	(*errorHandler) (buf);
	fprintf (stderr, "%s: Fatal: Unexpected return from error handler.\n",
		 programName);
	exit (EXIT_FAILURE);
}


/*! \brief Report a non-fatal program error.
 *
 *  \param  format
 */

EXPORT_VISTA void VWarning (VStringConst format, ...)
{
	va_list args;
	char buf[maxErrorMessageLength + 3];

	if (VERBOSE < 1)
		return;
	va_start (args, format);
	FormatMsg (buf, "Warning", format, &args, 0);
	va_end (args);
	(*warningHandler) (buf);
}


/*! \brief Report a application specific messages, but only if verbose equal 
 *         or greater verbose level 1.
 * 
 *  \param format
 */

EXPORT_VISTA void VMessage (VStringConst format, ...)
{
	va_list args;
	char buf[maxErrorMessageLength + 3];

	if (VERBOSE < 2)
		return;

	va_start (args, format);
	FormatMsg (buf, "Message", format, &args, 0);
	va_end (args);
	(*warningHandler) (buf);
}

/*! \brief Report a fatal error incurred by a system call.
 *
 *  \param format
 */

EXPORT_VISTA void VSystemError (VStringConst format, ...)
{
	va_list args;
	char buf[maxErrorMessageLength + 3];

	va_start (args, format);
	FormatMsg (buf, "Fatal", format, &args, strerror (errno));
	va_end (args);
	(*errorHandler) (buf);
	fprintf (stderr, "%s: Fatal: Unexpected return from error handler.\n",
		 programName);
	exit (EXIT_FAILURE);
}



/*! \brief Report non-fatal error incurred by a system call.
 *
 *  \param format
 */

EXPORT_VISTA void VSystemWarning (VStringConst format, ...)
{
	va_list args;
	char buf[maxErrorMessageLength + 3];

	va_start (args, format);
	FormatMsg (buf, "Warning", format, &args, strerror (errno));
	va_end (args);
	(*warningHandler) (buf);
}


/*
 *  FormatMsg -- format an error message for output.
 */

static void FormatMsg (char *buf, VStringConst severity, VStringConst format,
		       va_list * args, VStringConst extra)
{
	if (programName)
		snprintf (buf, maxErrorMessageLength,"%s: %s: ", programName, severity);
	else
		snprintf (buf, maxErrorMessageLength, "%s: ", severity);
	vsnprintf (buf + strlen (buf), maxErrorMessageLength - strlen (buf), format, *args);
	if (extra)
		snprintf (buf + strlen (buf), maxErrorMessageLength - strlen (buf), ": %s", extra);
	strcat (buf, ".\n");
}


/*! \brief Default error handler.
 *  
 *  \param msg
 */

EXPORT_VISTA void VDefaultError (VStringConst msg)
{
	fputs (msg, stderr);
	exit (EXIT_FAILURE);
}


/*! \brief Default warning handler.
 *
 *  \param msg
 */

EXPORT_VISTA void VDefaultWarning (VStringConst msg)
{
	fputs (msg, stderr);
}
