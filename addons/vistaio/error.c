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

#ifdef WIN32
#define snprintf _snprintf
#endif

/* Name of the program, for use in error messages: */
static VistaIOStringConst programName;

/* Routines to call to report errors: */
static VistaIOErrorHandler *errorHandler = VistaIODefaultError;
static VistaIOWarningHandler *warningHandler = VistaIODefaultWarning;

/* Maximum length of an error message: */
#define maxErrorMessageLength	500

int VERBOSE = 1; 

/* Later in this file: */
static void FormatMsg (char *, VistaIOStringConst, VistaIOStringConst, va_list *,
		       VistaIOStringConst);


/*! \brief Establish a caller-supplied routine as the handler for error messages.
 *
 *  \param fnc
 */

EXPORT_VISTA void VistaIOSetErrorHandler (VistaIOErrorHandler * fnc)
{
	errorHandler = fnc ? fnc : VistaIODefaultError;
}


/*! \brief Establish a caller-supplied routine as the handler for warning messages.
 *
 *  \param  fnc
 */

EXPORT_VISTA void VistaIOSetWarningHandler (VistaIOWarningHandler * fnc)
{
	warningHandler = fnc ? fnc : VistaIODefaultWarning;
}


/*! \brief Note the program name for use in error messages.
 *
 *  \param name
 */

EXPORT_VISTA void VistaIOSetProgramName (VistaIOStringConst name)
{
	programName = name;
}


/*! \brief Report a fatal program error.
 *
 *  \param format
 */

EXPORT_VISTA void VistaIOError (VistaIOStringConst format, ...)
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

EXPORT_VISTA void VistaIOWarning (VistaIOStringConst format, ...)
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

EXPORT_VISTA void VistaIOMessage (VistaIOStringConst format, ...)
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

EXPORT_VISTA void VistaIOSystemError (VistaIOStringConst format, ...)
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

EXPORT_VISTA void VistaIOSystemWarning (VistaIOStringConst format, ...)
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

static void FormatMsg (char *buf, VistaIOStringConst severity, VistaIOStringConst format,
		       va_list * args, VistaIOStringConst extra)
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

EXPORT_VISTA void VistaIODefaultError (VistaIOStringConst msg)
{
	fputs (msg, stderr);
	exit (EXIT_FAILURE);
}


/*! \brief Default warning handler.
 *
 *  \param msg
 */

EXPORT_VISTA void VistaIODefaultWarning (VistaIOStringConst msg)
{
	fputs (msg, stderr);
}
