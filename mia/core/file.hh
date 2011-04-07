/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*! \brief Wrapper for the stdio FILE type

A simple wrapper for the stdio FILE type that handles opening and closing
and takes care, whether \a stdin should be used for reading.
If the filename has the suffix ".gz"  the in/output is piped through gzip,
and if the filename has the suffix "*.bz2" the in/output is piped through bzip2,
providing transparent compression of the data.


\author Gert Wollny <gw.fossdev@gmail.com>

*/


#ifndef mia_core_file_hh
#define mia_core_file_hh

#include <cstdio>
#include <string>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

typedef FILE * PFILE;

/**
   A wrapper for the \a cstdio file type.
*/

class EXPORT_CORE CFile {
public:
	/** constructor
	    \param filename of the file to handle
	    \param from_stdio use \a stdin or \a stdout
	    \param write open for writing or reading
	*/
	CFile(const std::string& filename, bool from_stdio, bool write);

	/** destructor, closes file if necessary */
	virtual ~CFile() throw();

	/** \returns the stdio FILE pointer */
	operator PFILE();

	/** \returns true if the file is not open (i.e. the FILE pointer is NULL */
	bool operator ! ()const;

private:
	bool m_must_close;
	bool m_is_pipe;
	FILE *m_file;
};


/** A simple wrapper for the stdio FILE type that handles opening and closing for
    reading and takes care, whether \a stdin should be used */
class EXPORT_CORE CInputFile: public CFile {
public:

	/** constructor opens the file
	    \param filename is the name of the file to be openend, if it is  "-" then  \a stdin is used
	*/

	CInputFile(const std::string& filename);

	/** constructor opens the file
	    \param filename is the name of the file to be openend
	    \param from_stdin force usage of \a stdin
	*/
	CInputFile(const std::string& filename, bool from_stdin);
};

/** A simple wrapper for the stdio FILE type that handles opening and closing for
    writing and takes care, whether \a stdout should be used */
class EXPORT_CORE COutputFile: public CFile {
public:

	/** constructor opens the file
	    \param filename is the name of the file to be openend, if it is  "-" then  \a stdin is used
	*/

	COutputFile(const std::string& filename);

	/** constructor opens the file
	    \param filename is the name of the file to be openend
	    \param from_stdin force usage of \a stdin
	*/
	COutputFile(const std::string& filename, bool from_stdin);
};

/** This class provides the base for showing sime kind of progress bar during some operation */
class EXPORT_CORE CProgressCallback {
public:
	virtual ~CProgressCallback();
	/** this routine shows the progress
	    \param pos current file io position
	    \param length file size
	*/
	virtual void show_progress(int pos, int length);
};

NS_MIA_END

#endif
