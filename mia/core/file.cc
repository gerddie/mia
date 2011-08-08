/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
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
	m_must_close(!from_stdio),
	m_is_pipe(false),
	m_file(0)
{
	if (from_stdio) {
		m_file = write ? stdout : stdin;
		return;
	}
#ifndef WIN32
	int  last_dot = filename.rfind('.');

	string suffix = last_dot >= 0   ? filename.substr(last_dot) : "";

	string pipe;
	if (suffix == string(".gz")) {
		m_is_pipe = true;
		pipe = (write ? string("gzip >") : string("cat ")) + filename.c_str() + string("| zcat ");
	}else if (suffix == string(".bz2")) {
		m_is_pipe = true;
		pipe = (write ? string("bzip2 >") : string("bzcat ")) + filename.c_str();
	}else if (suffix == string(".Z")) {
		m_is_pipe = true;
		pipe = (write ? string("compress >") : string("zcat ")) + filename.c_str();
	}

	if (m_is_pipe)
		m_file = popen (pipe.c_str(), write ? "w": "r");
	else
#endif
		m_file = fopen (filename.c_str(), write ? "wb": "rb");

	if (!m_file) {
		stringstream errmsg;
		errmsg << filename <<":" << strerror(errno);
		throw runtime_error(errmsg.str());
	}
}

/** destructor, closes file if necessary */
CFile::~CFile() throw()
{
	if (m_must_close && m_file) {
#ifndef WIN32
		if (m_is_pipe)
			pclose(m_file);
		else
#endif
			fclose(m_file);
	}
}


/** \returns the stdio FILE pointer */
CFile::operator PFILE()
{
	return m_file;
}

/** \returns true if the file is not open (i.e. the FILE pointer is NULL */
bool CFile::operator ! ()const
{
	return m_file == NULL;
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

NS_MIA_END


