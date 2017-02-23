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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef WIN32
#include <direct.h>
#define getcwd _getcwd
#define chdir _chdir
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

// WIN32 and HURD don't define this 
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif 

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


#ifndef _GNU_SOURCE
// there should be an intrinsic (at least on intel) 
void EXPORT_CORE sincosf(float x, float *s, float *c)
{
	*s = sinf(x); 
	*c = cosf(x); 
}

void EXPORT_CORE sincos(double x, double *s, double *c)
{
	*s = sin(x); 
	*c = cos(x); 
}
#endif

NS_MIA_END

