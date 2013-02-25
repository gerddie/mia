/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WIN32
#include <regex.h>
#include <dirent.h>
#else
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
#define PATH_MAX 1024
#endif

#include <limits.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <stdexcept>
#include <iomanip>

// MIA specific
#include <mia/core/utils.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

CCWDSaver::CCWDSaver():
	cwd(new char[PATH_MAX])
{
	if (!getcwd(cwd, PATH_MAX)) {
		delete[] cwd;
		throw strerror(errno);
	}
}

CCWDSaver::~CCWDSaver()
{
	if (chdir(cwd) == -1) {
		cverr() << "CCWDSaver::~CCWDSaver():" << strerror(errno) << "\n";
	}
	delete[] cwd;
}

#if 0
static bool scan_dir(const std::string& path, const std::string& pattern, std::list<std::string>& list)
{
	class TFindRegExp {
		regex_t preg;
	public:
		TFindRegExp(const std::string& pattern) {
			char buf[1024];
			int status = regcomp (&preg, pattern.c_str(), REG_EXTENDED |REG_NOSUB);
			if (status) {
				regerror(status, &preg, buf, 1024);
				std::cerr << buf << std::endl;
			}
		}

		~TFindRegExp() {
			regfree(&preg);
		}
		bool check(const char *s) {
			return !regexec(&preg, s,0, NULL, 0);
		}
	};


	CCWDSaver __saver;

	if (chdir(path.c_str())) {
		//std::cerr << path << ":" << strerror(errno) << std::endl;
		return false;
	}

	struct dirent **namelist;
	int nfiles = scandir(".", &namelist, NULL , NULL);

	TFindRegExp searcher(pattern);
	for (int i = 0; i < nfiles; i++) {
		if (searcher.check(namelist[i]->d_name))
			list.push_back(path + std::string("/") + std::string(namelist[i]->d_name));
		free(namelist[i]);

	}
	free(namelist);

	return true;
}


FSearchFiles::FSearchFiles(std::list<std::string>& __result, const std::string& __pattern):
	result(__result),
	pattern(__pattern)
{
}

void FSearchFiles::operator()(const std::string& path) {
	try {
		scan_dir(path, pattern, result);
	}
	catch (std::exception& e) {
		std::cerr << e.what();
	}
}
#endif

#ifndef _GNU_SOURCE
// there should be an intrinsic (at least on intel) 
void sincosf(float x, float *s, float *c)
{
	*s = sinf(x); 
	*c = cosf(x); 
}

void sincos(double x, double *s, double *c)
{
	*s = sin(x); 
	*c = cos(x); 
}
#endif

NS_MIA_END

