/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cerrno>
#include <cstring>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stdexcept>
#include <map>
#include <set>

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

#include <mia/core/file.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using namespace std;

namespace bf = boost::filesystem;

#ifndef WIN32
struct ExternalCompressors {

       string has_compressor(const string& program) const;
       string has_decompressor(const string& program) const;

       static const ExternalCompressors& instance();
private:
       void search_compressor(const char *suffix, const vector<const char *>& names, const vector<bf::path>&  binary_dirs);
       ExternalCompressors();
       bool try_path(const bf::path& p);
       void search_compressor(const char *suffix, const vector<const char *>& names);
       void search_decompressor(const char *suffix, const char *name, const vector<bf::path>&  binary_dirs);

       string has(const string& suffix, const map<string, string>& m) const;
       map<string, string> compressor;
       map<string, string> decompressor;
};

void ExternalCompressors::search_compressor(const char *suffix, const vector<const char *>& names, const vector<bf::path>&  binary_dirs)
{
       bool success = false;
       unsigned i = 0;

       while (!success && i < names.size()) {
              string progname(names[i]);

              for (auto bd = binary_dirs.begin(); bd != binary_dirs.end() && !success; ++bd) {
                     cvdebug() << "Search compressor " << *bd << "/" << progname << "\n";
                     success = try_path(*bd / progname);
              }

              if (success)
                     compressor[suffix] = progname;

              ++i;
       }
}

void ExternalCompressors::search_decompressor(const char *suffix, const char *name, const vector<bf::path>&  binary_dirs)
{
       bool success = false;
       string progname(name);

       for (auto bd = binary_dirs.begin(); bd != binary_dirs.end() && !success; ++bd) {
              cvdebug() << "Search decompressor " << *bd << "/" << progname << "\n";
              success = try_path(*bd / progname);
       }

       if (success)
              decompressor[suffix] = progname;
}

ExternalCompressors::ExternalCompressors()
{
       vector<bf::path> binary_dirs;
       const char *path = getenv("PATH");

       if (!path)
              return;

       string spath(path);
       boost::char_separator<char> sep(":");
       boost::tokenizer< boost::char_separator<char>> tok(spath, sep);

       for (auto t = tok.begin(); t != tok.end(); ++t)
              binary_dirs.push_back(bf::path(*t));

       search_compressor(".gz", {"pigz", "gzip"}, binary_dirs);
       search_compressor(".xz", {"pxz", "xz"}, binary_dirs);
       search_compressor(".bz2", {"bzip2"}, binary_dirs);
       search_compressor(".Z", {"compress"}, binary_dirs);
       search_decompressor(".gz", "zcat", binary_dirs);
       search_decompressor(".xz", "xzcat", binary_dirs);
       search_decompressor(".bz2", "bzcat", binary_dirs);
       search_decompressor(".Z", "zcat", binary_dirs);
}

const ExternalCompressors& ExternalCompressors::instance()
{
       static ExternalCompressors me;
       return me;
}

string ExternalCompressors::has_compressor(const string& suffix) const
{
       return has(suffix, compressor);
}
string ExternalCompressors::has_decompressor(const string& suffix) const
{
       return has(suffix, decompressor);
}

string ExternalCompressors::has(const string& suffix, const map<string, string>& m) const
{
       auto idx = m.find(suffix);

       if (idx == m.end()) {
              throw create_exception<runtime_error>("Try to do IO with compresssed file of type '",
                                                    suffix, "', but no corresponding (de-)compressor program found");
       }

       return idx->second;
}


bool ExternalCompressors::try_path(const bf::path& p)
{
       boost::system::error_code ec;
       auto st = bf::status(p, ec);

       if (ec != boost::system::errc::success)
              return false;

       if (st.permissions() & bf::others_exe)
              return true;

       return false;
}


set<string> recognized_sompression_suffixes{".gz", ".xz", ".bz2", ".Z"};

#endif

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

       if (recognized_sompression_suffixes.find(suffix) == recognized_sompression_suffixes.end()) {
              m_file = fopen (filename.c_str(), write ? "wb" : "rb");
       } else {
              cvdebug() << "Try using a (de)compressor for type " << suffix << "\n";

              if (!write) {
                     string pipe;
                     auto dcprogname = ExternalCompressors::instance().has_decompressor(suffix);
                     m_is_pipe = true;
                     pipe = dcprogname + string(" ") + filename.c_str();
                     cvdebug() << "Open pipe like '" << pipe << "'\n";
                     m_file = popen (pipe.c_str(), "r");
              } else {
                     string pipe;
                     cvdebug() << "See if compressor is available\n";
                     auto cprogname = ExternalCompressors::instance().has_compressor(suffix);
                     m_is_pipe = true;
                     pipe = cprogname + string(" > ") + filename.c_str();
                     cvdebug() << "Open pipe like '" << pipe << "'\n";
                     m_file = popen (pipe.c_str(), "w");
              }
       }

#else
// in WIN32 no pipes
       m_file = fopen (filename.c_str(), write ? "wb" : "rb");
#endif

       if (!m_file) {
              stringstream errmsg;
              errmsg << filename << ":" << strerror(errno);
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


