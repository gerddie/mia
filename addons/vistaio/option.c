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

#include <sys/stat.h>
#include "vistaio/vistaio.h"

#ifdef STAT_MACROS_BROKEN
#warning "The sys/stat.h macros are broken; you will experience problems while piping files"
#endif

/* Later in this file: */
static int ParseArgValues (int *, int, char **, VistaIOOptionDescRec *);

/* The following provide distinguished values for the found field of a
   VistaIOOptionDescRec: */
VistaIOBoolean VistaIO_RequiredOpt;
VistaIOBoolean VistaIO_OptionalOpt;

VistaIOShort VERBOSE = 2;		
VistaIOBoolean v_found = 0;		/* mpi-hack: v_found */

/*! \brief Identify the files specified by command line arguments.
 *
 *  Files can be provided in any of three ways:
 *   (1) as arguments to a switch, -keyword (e.g, -in file1 file2)
 *   (2) as additional command line arguments, not associated with
 *       any switch (e.g., vview file1 file2)
 *   (3) by piping to/from stdin or stdout (e.g., vview < file1).
 *
 *  VistaIOIdentifyFiles collects file names from these three sources once
 *  VistaIOParseCommand has been called to parse the command's switches.
 *  It looks first for the keyword, then for unclaimed command line
 *  arguments, and finally for a file or pipe attached to stdin or stdout.
 *
 *  \param  noptions
 *  \param  options
 *  \param  keyword
 *  \param  argc
 *  \param  argv
 *  \param  fd
 *  \return VistaIOBoolean
 */

VistaIOBoolean VistaIOIdentifyFiles (int noptions, VistaIOOptionDescRec options[],
			 VistaIOStringConst keyword, int *argc, char **argv, int fd)
{
	VistaIOOptionDescRec *opt;
	VistaIOArgVector *vec;
	VistaIOBoolean switch_found = FALSE;
	VistaIOStringConst *values;
	struct stat stat_buf;
	int i, j, n;

	/* A -in or -out switch on the command line may specify one or more
	   files. Look for the switch's entry in the option table supplied: */
	for (i = 0, opt = options; i < noptions; i++, opt++)
		if (strcmp (keyword, opt->keyword) == 0)
			break;
	if (i == noptions)
		VistaIOError ("VistaIOIdentifyFiles: Option -%s not defined in option table", keyword);

	/* The option table entry must specify a location for storing filenames: */
	if (!opt->value)
		VistaIOError ("VistaIOIdentifyFiles: No value storage for option -%s",
			keyword);
	vec = ((VistaIOArgVector *) opt->value);

	/* Whether or not the option was present on the command line should have
	   been recorded in a "found" flag: */
	if (opt->found && opt->found != VistaIORequiredOpt
	    && opt->found != VistaIOOptionalOpt)
		switch_found = *(opt->found);
	else
		VistaIOError ("VistaIOIdentifyFiles: No \"found\" flag for option -%s",
			keyword);

	/* If a -in or -out switch was specified, it supplies the files' names: */
	if (switch_found)
		return TRUE;

	/* If no switch was specified but some extra arguments were supplied
	   with the command, those arguments should be the files' names: */
	if (*argc > 1) {

		/* If any number are permitted, allocate storage for all of them: */
		if (opt->number == 0) {
			vec->vector =
				VistaIOMalloc ((*argc - 1) * sizeof (VistaIOStringConst));
			values = (VistaIOStringConst *) vec->vector;
		} else
			values = (VistaIOStringConst *) opt->value;

		/* Accept arguments as filenames provided they don't begin with
		   a single -: */
		for (i = j = 1, n = 0; i < *argc; i++)
			if ((argv[i][0] == '-' && argv[i][1] != '-') ||
			    (opt->number && n == opt->number))
				argv[j++] = argv[i];	/* not a filename */
			else if (argv[i][0] == '-' && argv[i][1] == '-')
				values[n++] = argv[i] + 1;	/* filename with -- prefix */
			else
				values[n++] = argv[i];	/* filename */

		/* Ensure that a sufficient number were supplied: */
		if (n) {
			if (opt->number) {
				if (n != opt->number) {
					if (opt->number == 1) {
						goto WrongNumber1;
					} else {
						goto WrongNumberN;
					}
				}
			} else
				vec->number = n;
			*argc = j;
			return TRUE;
		}
	}

	/* That leaves only an open file descriptor as the file. In that case
	   there can be at most one such file: */
	if (opt->number > 1) {
	      WrongNumberN:
		fprintf (stderr, "%s: %d files must be specified by -%s or "
			 "extra command arguments.\n", argv[0], opt->number,
			 keyword);
		return FALSE;
	}

	/* If a particular file descriptor is specified by fd, ensure that it
	   is connected to a pipe or file (not the terminal): */
	if (fd >= 0) {
		if (fstat (fd, &stat_buf) == -1)
			VistaIOSystemError ("Failed to fstat() fd %d", fd);
		if (!S_ISREG (stat_buf.st_mode)
#ifdef S_ISSOCK
/* The ISSOCK macro is only available in POSIX_STDC;
   beware of problems with piping files if this line is omited */
		    && !S_ISSOCK (stat_buf.st_mode)
#endif
		    && !S_ISLNK (stat_buf.st_mode)
		    && !S_ISFIFO (stat_buf.st_mode)) {
		      WrongNumber1:
			fprintf (stderr,
				 "%s: No file specified by -%s, extra command "
				 "argument, or %c.\n", argv[0], keyword,
				 "<>"[fd]);
			return FALSE;
		}
	}

	/* Record the file as being "-": */
	if (opt->number == 0) {
		vec->number = 1;
		vec->vector = VistaIOMalloc (sizeof (VistaIOStringConst));
		((VistaIOStringConst *) vec->vector)[0] = "-";
	} else
		*((VistaIOStringConst *) opt->value) = "-";
	return TRUE;
}


/*
 *  VistaIOParseCommand
 *
 *  Parse command line arguments according to a table of option descriptors.
 *  Unrecognized options are left in argv, and argc is adjusted to reflect
 *  their number.
 *  If an erroneous (as opposed to simply unrecognized) argument is
 *  encountered, VistaIOParseCommand returns FALSE; otherwise, TRUE.
 *
 *  The -help option is recognized explicitly. If it is present, VistaIOParseCommand
 *  returns indicating that all arguments were recognized, but that an error
 *  occurred. This should force the caller to simply print usage information.
 */

VistaIOBoolean VistaIOParseCommand (int noptions, VistaIOOptionDescRec options[], int *argc,
			char **argv)
{
	int arg, nvalues, i, j;
	char *cp;
	VistaIOBoolean *opts_seen, result = TRUE, missing_val = FALSE;
	VistaIOOptionDescRec *opt, *opt_t;

	/* Note the program's name: */
	VistaIOSetProgramName (argv[0]);

	/* Allocate storage for a set of flags indicating which
	   arguments have been seen: */
	opts_seen = VistaIOCalloc (noptions, sizeof (VistaIOBoolean));

	/* Initialize any "found" flags to false, and the number field of any
	   VistaIOArgVector values to zero: */
	for (opt = options + noptions - 1; opt >= options; opt--) {
		if (opt->found)
			*opt->found = FALSE;
		if (opt->number == 0 && opt->value)
			((VistaIOArgVector *) opt->value)->number = 0;
	}

	/* For each argument supplied with the command: */
	for (arg = 1; arg < *argc;) {
		cp = argv[arg++];

		/* If it doesn't start with - it can't be an option: */
		if (cp[0] != '-' || cp[1] == 0)
			continue;

		/* Check for -help: */
		if (strcmp (cp + 1, "help") == 0) {

			/* If found, return FALSE to force printing of usage info: */
			*argc = 1;
			return FALSE;
		}

		/* Look up the argument in the list of options: */
		i = strlen (cp + 1);
		opt = NULL;
		for (opt_t = options + noptions - 1; opt_t >= options;
		     opt_t--) {
			if (strncmp (cp + 1, opt_t->keyword, i) != 0)
				continue;	/* not this one */
			if (i == strlen (opt_t->keyword)) {
				opt = opt_t;
				break;	/* an exact match */
			}
			if (opt)
				goto NextArg;	/* already matched another prefix */
			opt = opt_t;	/* note a prefix match */
		}

		/* If the argument isn't recognized, skip it: */
		if (!opt)
			goto NextArg;	/* not recognized */

		/* Remove it from the list of command arguments: */
		argv[arg - 1] = 0;

		/* Ensure that the option has not already been seen: */
		if (opts_seen[opt - options]) {
			fprintf (stderr,
				 "%s: Duplicate -%s option; ignoring all but last.\n",
				 argv[0], opt->keyword);

			/* If it has been seen, delete its previous value: */
			if (opt->number == 0) {
				VistaIOFree (((VistaIOArgVector *) opt->value)->vector);
				((VistaIOArgVector *) opt->value)->number = 0;
			}
		} else
			opts_seen[opt - options] = TRUE;

		/* Swallow any value(s) that follow: */
		switch (opt->repn) {

		case VistaIOBitRepn:
		case VistaIOUByteRepn:
		case VistaIOSByteRepn:
		case VistaIOShortRepn:
		case VistaIOLongRepn:
		case VistaIOFloatRepn:
		case VistaIODoubleRepn:
		case VistaIOBooleanRepn:
		case VistaIOStringRepn:
			nvalues = ParseArgValues (&arg, *argc, argv, opt);
			break;

		default:
			VistaIOError ("Parsing of command options with %s values "
				"is not implemented", VistaIORepnName (opt->repn));
			nvalues = 0;	/* to quiet lint */
		}

		/* Ensure that the expected number of arguments was found: */

		/* 
		   ** mpi-hack: that means the parameter can have a variable value list specified by
		   ** opt->number == 0, but it needs minimum one value 
		 */

		if (opt->number == 0 && nvalues == 0)
			missing_val = TRUE;

		if ( (opt->number  && (nvalues != opt->number)) || missing_val ) {

			/* Either we encountered an argument we couldn't parse, or
			   we used up all arguments before finding the expected number
			   of them: */
			fprintf (stderr, "%s: Option -%s ", argv[0],
				 opt->keyword);
			if (arg < *argc)
				fprintf (stderr, "has incorrect value %s.\n",
					 argv[arg]);
			else if (opt->number > 1)
				fprintf (stderr,
					 "requires %d values; found only %d.\n",
					 opt->number, nvalues);
			else
				fprintf (stderr, "requires a value.\n");
			result = FALSE;
			break;
		}

		if (opt->number == 0)
			((VistaIOArgVector *) opt->value)->number = nvalues;

		/* Note that a value was successfully obtained for this option: */
		if (opt->found)
			*(opt->found) = TRUE;

	      NextArg:;
	}

	/* Ensure that each mandatory option was seen: */
	for (i = 0; i < noptions; i++)
		if (options[i].found == VistaIORequiredOpt && !opts_seen[i]) {
			fprintf (stderr,
				 "%s: Option -%s must be specified.\n",
				 argv[0], options[i].keyword);
			result = FALSE;
		}
	VistaIOFree ((VistaIOPointer) opts_seen);

	/* Squeeze together the remaining arguments in argv: */
	for (i = j = 1; i < *argc; i++)
		if (argv[i])
			argv[j++] = argv[i];
	*argc = j;

	return result;
}


/*
 *  ParseArgValues
 *
 *  Parse any arguments supplying values for an option.
 */

static int ParseArgValues (int *arg, int argc, char **argv,
			   VistaIOOptionDescRec * opt)
{
	VistaIOArgVector *vec = (VistaIOArgVector *) opt->value;
	VistaIOPointer values;
	int nvalues = 0;
	VistaIODictEntry *dict = opt->dict;
	char *cp;

	/* Locate the place we're to store the argument values: */
	if (opt->number == 0) {
		/* If a variable number of arguments is expected, allocate storage
		   to hold them: */
		vec->vector = values =
			VistaIOMalloc ((argc - *arg) * VistaIORepnSize (opt->repn));
	} else
		values = opt->value;

	/* If no dictionary is specified for a boolean-valued option, use the
	   default one of true, false, yes, no... */
	if (opt->repn == VistaIOBooleanRepn && !dict)
		dict = VistaIOBooleanDict;

	/* Parse argument values until we've reached the required number,
	   we've run out of entered arguments, or we encounter one that
	   is ill-formed: */
	while ((opt->number == 0 || nvalues < opt->number) && (*arg < argc)) {
		cp = argv[*arg];

		/* Special treatment for string-valued options: */
		if (opt->repn == VistaIOStringRepn) {

			/* An argument of the form -string is not interpreted as a
			   string value: */
			if (cp[0] == '-' && cp[1] != 0 && cp[1] != '-')
				break;

			/* An argument of the form --string is interpreted as string: */
			if (cp[0] == '-' && cp[1] == '-' && cp[2] != 0)
				cp += 2;
		}

		/* Convert the argument to the specified internal form: */
		if (!VistaIODecodeAttrValue (cp, dict, opt->repn, values))
			break;
		nvalues++;
		values = (VistaIOPointer) ((char *)values + VistaIORepnSize (opt->repn));
		argv[(*arg)++] = NULL;
	}

	/* Special treatment of boolean-valued options: if the option has just
	   one value associated with it then treat -option <other options> like
	   -option true <other options>: */
	if (opt->repn == VistaIOBooleanRepn && opt->number == 1 && nvalues == 0) {
		*(VistaIOBoolean *) opt->value = TRUE;
		nvalues = 1;
	}

	return nvalues;
}


/*
 *  VistaIOParseFilterCmd
 *
 *  Does all of the standard command line parsing and file location needed
 *  by Vista modules with a maximum of one input and output file.
 *  This routine prints the appropriate error messages and exits if
 *  necessary, so there is no need to check for a return value.
 *
 *  "inp" and "outp" are pointers to file variables.  The files can be
 *  specified in command line arguments according to Vista conventions,
 *  or else default to stdin and stdout respectively.  If either pointer 
 *  is NULL, then it is assumed that the file is not needed.
 *
 *  The standard "-in" and "-out" command line options are added by this
 *  routine so that they do not need to be specified in options arguments.
 */

VistaIODictEntry VerboseDict[] = {
	{"err", 0}
	,
	{"warn", 1}
	,
	{"mesg", 2}
	,
	{NULL}
};

void VistaIOParseFilterCmd (int noptions, VistaIOOptionDescRec opts[],
		      int argc, char **argv, FILE ** inp, FILE ** outp)
{
	static VistaIOStringConst in_file = NULL, out_file = NULL;
	static VistaIOBoolean in_found = 0, out_found = 0;
	static VistaIOShort v = 0;

	/* mpi-hack: add the option verbose to the option array */
	static VistaIOOptionDescRec io_opts[] = {
		{"in", VistaIOStringRepn, 1, &in_file, &in_found, NULL,
		 "Input file"},
		{"out", VistaIOStringRepn, 1, &out_file, &out_found, NULL,
		 "Output file"},
		{"verbose", VistaIOShortRepn, 1, (VistaIOPointer) & v, &v_found,
		 VerboseDict, "Verbosity level"}
	};
	int i, n;
	VistaIOOptionDescRec options[100];

	/* Check that number of options will not overflow the options array. */
	if (noptions >= 98) {
		VistaIOWarning ("VistaIOParseFilterCmd: Too many options allowed");
		noptions = 98;
	}

	/* Copy options into a new list and add the "in" and "out" options. */
	n = 0;
	if (inp)
		options[n++] = io_opts[0];
	if (outp)
		options[n++] = io_opts[1];
	/* mpi-hack: add the option verbose to the option list */

	options[n++] = io_opts[2];

	for (i = 0; i < noptions; i++, n++)
		options[n] = opts[i];

	/* Parse command line arguments and identify the input and output files: */
	if (!VistaIOParseCommand (n, options, &argc, argv) ||
	    (inp && !VistaIOIdentifyFiles (n, options, "in", &argc, argv, 0)) ||
	    (outp && !VistaIOIdentifyFiles (n, options, "out", &argc, argv, 1))) {
		goto Usage;
	}

	VERBOSE = (v < MIN_VERBOSE_LVL ? MIN_VERBOSE_LVL :
		   (v > MAX_VERBOSE_LVL ? MAX_VERBOSE_LVL : v));

	/* Any remaining unparsed arguments are erroneous: */
	if (argc > 1) {
		VistaIOReportBadArgs (argc, argv);
	      Usage:VistaIOReportUsage (argv[0], n, options,
			      inp ? (outp ? "[infile] [outfile]" : "[infile]")
			      : (outp ? "[outfile]" : NULL));
		exit (EXIT_FAILURE);
	}

	/* Open the input and output files: */
	if (inp)
		*inp = VistaIOOpenInputFile (in_file, TRUE);
	if (outp)
		*outp = VistaIOOpenOutputFile (out_file, TRUE);
}


/*
 *  VistaIOPrintOptions
 *
 *  Print the settings of a set of command line options.
 */

void VistaIOPrintOptions (FILE * f, int noptions, VistaIOOptionDescRec options[])
{
	int i;

#define commentColumn 40

	for (i = 0; i < noptions; i++, options++) {
		fputs ("  ", f);
		if (options->blurb)
			fprintf (f, "%s ", options->blurb);
		fprintf (f, "(%s): ", options->keyword);
		VistaIOPrintOptionValue (f, options);
		fputc ('\n', f);
	}

#undef commentColumn
}


/*
 *  VistaIOPrintOptionValue
 *
 *  Print the value of a specified option.
 */

int VistaIOPrintOptionValue (FILE * f, VistaIOOptionDescRec * option)
{
	int n, i, col = 0;
	char *vp;
	VistaIODictEntry *dict;
	VistaIOLong ivalue;
	VistaIODouble fvalue = 0.0;
	VistaIOStringConst svalue;

	if (option->number == 0) {
		n = ((VistaIOArgVector *) option->value)->number;
		vp = (char *)((VistaIOArgVector *) option->value)->vector;
	} else {
		n = option->number;
		vp = (char *)option->value;
	}
	for (i = 0; i < n; i++, vp += VistaIORepnSize (option->repn)) {
		if (i > 0)
			fputc (' ', f);
		switch (option->repn) {

		case VistaIOBitRepn:
			ivalue = *(VistaIOBit *) vp;
			goto PrintLong;

		case VistaIOUByteRepn:
			ivalue = *(VistaIOUByte *) vp;
			goto PrintLong;

		case VistaIOSByteRepn:
			ivalue = *(VistaIOSByte *) vp;
			goto PrintLong;

		case VistaIOShortRepn:
			ivalue = *(VistaIOShort *) vp;
			goto PrintLong;

		case VistaIOLongRepn:
			ivalue = *(VistaIOLong *) vp;

		PrintLong:if (option->dict &&
			      (dict =
			       VistaIOLookupDictValue (option->dict, VistaIOLongRepn,
						 ivalue)))
				col += fprintf (f, "%s", dict->keyword);
			else
				col += fprintf (f, "%d", ivalue);
			break;
			
		case VistaIOFloatRepn:
			fvalue = *(VistaIOFloat *) vp;
			goto PrintDbl;
			
		case VistaIODoubleRepn:
			fvalue = *(VistaIODouble *) vp;

		PrintDbl:if (option->dict &&
			     (dict =
			      VistaIOLookupDictValue (option->dict, VistaIODoubleRepn,
						fvalue)))
				col += fprintf (f, "%s", dict->keyword);
			else
				col += fprintf (f, "%g", fvalue);
			break;
			
		case VistaIOBooleanRepn:
			col += fprintf (f, "%s",
					*(VistaIOBoolean *) vp ? "true" : "false");
			break;
			
		case VistaIOStringRepn:
			svalue = *(VistaIOString *) vp;
			if (!svalue)
				svalue = "(none)";
			else if (option->dict &&
				 (dict =
				  VistaIOLookupDictValue (option->dict, VistaIOStringRepn,
						    svalue)))
				svalue = dict->keyword;
			col += fprintf (f, "%s", svalue);
			break;
			
		default:
			break;
		}
	}

	return col;
}


/*
 *  VistaIOReportBadArgs
 *
 *  Report the remaining command line arguments in argv as ones that could
 *  not be recognized.
 */

void VistaIOReportBadArgs (int argc, char **argv)
{
	int i;

	if (argc > 1) {
		fprintf (stderr, "%s: Unrecognized argument%s:", argv[0],
			 (argc == 2) ? "" : "s");
		for (i = 1; i < argc; i++)
			fprintf (stderr, " %s", argv[i]);
		fputc ('\n', stderr);
	}
}


/*
 *  VistaIOReportUsage
 *
 *  Print, to stderr, information about how to use a program based on
 *  the contents of its command argument parsing table.
 */

void VistaIOReportUsage (VistaIOStringConst program, int noptions,
		   VistaIOOptionDescRec options[], VistaIOStringConst other_args)
{
	fprintf (stderr, "\nUsage: %s <options>", program);
	if (other_args)
		fprintf (stderr, " %s", other_args);
	fprintf (stderr, ", where <options> includes:\n");
	fprintf (stderr, "    -help\n\tDisplay this help and exit.\n");
	/* Print a line describing each possible option: */
	VistaIOReportValidOptions (noptions, options);
}


/*
 *  VistaIOReportValidOptions
 *
 *  Print, to stderr, a summary of program options based on the contents of
 *  a command argument parsing table
 */

void VistaIOReportValidOptions (int noptions, VistaIOOptionDescRec options[])
{
	int i, j;
	VistaIODictEntry *dict;
	VistaIOStringConst cp;

	/* Print a line describing each possible option: */
	for (i = 0; i < noptions; i++, options++) {

		/* Print the name of the option: */
		fprintf (stderr, "    -%s ", options->keyword);

		/* Get a name for the type of value needed: */
		if (VistaIOIsIntegerRepn (options->repn))
			cp = "integer";
		else if (VistaIOIsFloatPtRepn (options->repn))
			cp = "number";
		else if (options->repn == VistaIOBooleanRepn)
			cp = "boolean";
		else
			cp = "string";

		/* Print possible option values: */
		if (options->dict && options->number == 1) {
			for (dict = options->dict; dict->keyword; dict++) {
				if (dict > options->dict)
					fputs (" | ", stderr);
				fputs (dict->keyword, stderr);
			}
		} else if (options->repn == VistaIOBooleanRepn
			   && options->number == 1) {
			fputs ("[ true | false ]", stderr);
		} else if (options->number <= 1) {
			fprintf (stderr, "<%s>",
				 options->dict ? "keyword" : cp);
			if (options->number == 0)
				fputs (" ...", stderr);
		} else {
			for (j = 1; j <= options->number; j++) {
				if (j > 1)
					fputc (' ', stderr);
				fprintf (stderr, "<%s%d>",
					 options->dict ? "keyword" : cp, j);
			}
		}
		if (options->dict && options->number != 1) {
			fputs ("\n\t<keyword> is ", stderr);
			for (dict = options->dict; dict->keyword; dict++) {
				if (dict > options->dict)
					fputs (" | ", stderr);
				fputs (dict->keyword, stderr);
			}
		}
		fputs ("\n\t", stderr);

		/* Print any blurb available, and default value(s): */
		if (options->blurb)
			fprintf (stderr, "%s. ", options->blurb);
		if (options->found == VistaIORequiredOpt)
			fputs ("Required.", stderr);
		else if (options->number > 0) {
			fprintf (stderr, "Default: ");
			VistaIOPrintOptionValue (stderr, options);
		}
		fputc ('\n', stderr);
	}
}