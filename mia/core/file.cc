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

// $Id: miaFileIO.cc 775 2005-12-19 11:47:03Z write1 $

/*! \brief Wrapper for the stdio FILE type

A simple wrapper for the stdio FILE type that handles opening and closing
and takes care, whether \a stdin sould be used for reading

\file miaFileIO.cc
\author Gert Wollny <wollny@cbs.mpg.de>

*/

#include <cerrno>
#include <cstring>

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <mia/core/file.hh>

NS_MIA_BEGIN
using namespace std;

CFile::CFile(const string& filename, bool from_stdio, bool write):
	_M_must_close(!from_stdio),
	_M_is_pipe(false),
	_M_file(0)
{
	if (from_stdio) {
		_M_file = write ? stdout : stdin;
		return;
	}
#ifndef WIN32
	int  last_dot = filename.rfind('.');

	string suffix = last_dot >= 0   ? filename.substr(last_dot) : "";

	string pipe;
	if (suffix == string(".gz")) {
		_M_is_pipe = true;
		pipe = (write ? string("gzip >") : string("cat ")) + filename.c_str() + string("| zcat ");
	}else if (suffix == string(".bz2")) {
		_M_is_pipe = true;
		pipe = (write ? string("bzip2 >") : string("bzcat ")) + filename.c_str();
	}else if (suffix == string(".Z")) {
		_M_is_pipe = true;
		pipe = (write ? string("compress >") : string("zcat ")) + filename.c_str();
	}

	if (_M_is_pipe)
		_M_file = popen (pipe.c_str(), write ? "w": "r");
	else
#endif
		_M_file = fopen (filename.c_str(), write ? "wb": "rb");

	if (!_M_file) {
		stringstream errmsg;
		errmsg << filename <<":" << strerror(errno);
		throw runtime_error(errmsg.str());
	}
}

/** destructor, closes file if necessary */
CFile::~CFile() throw()
{
	if (_M_must_close && _M_file) {
#ifndef WIN32
		if (_M_is_pipe)
			pclose(_M_file);
		else
#endif
			fclose(_M_file);
	}
}


/** \returns the stdio FILE pointer */
CFile::operator PFILE()
{
	return _M_file;
}

/** \returns true if the file is not open (i.e. the FILE pointer is NULL */
bool CFile::operator ! ()const
{
	return _M_file == NULL;
}

CInputFile::CInputFile(const string& filename):
	CFile(filename, filename == string("-"), false)
{
}

CInputFile::CInputFile(const string& filename, bool from_stdin):
	CFile(filename, from_stdin, false)
{
}

COutputFile::COutputFile(const string& filename):
	CFile(filename, filename == string("-"), true)
{
}

COutputFile::COutputFile(const string& filename, bool to_stdout):
	CFile(filename, to_stdout, true)
{
}

CProgressCallback::~CProgressCallback()
{
}

void CProgressCallback::show_progress(int /*pos*/, int /*length*/)
{
}

NS_MIA_END


