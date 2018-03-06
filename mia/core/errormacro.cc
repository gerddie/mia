
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <mia/core/defines.hh>
#include <ostream>
#include <cstdlib>

#if defined(HAVE_CXXABI_H) && defined(HAVE_EXEINFO_H)
#include <execinfo.h>
#include <cxxabi.h>
#endif

NS_MIA_BEGIN

using std::ostream;

#if defined(HAVE_CXXABI_H) && defined(HAVE_EXECINFO_H)
void append_stacktrace(ostream& os)
{
       os << "backtrace:\n";
       void *addrlist[64];
       int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

       if (addrlen == 0) {
              os << "  not availble\n";
              return;
       }

       char **symbollist = backtrace_symbols(addrlist, addrlen);

       for (int i = 0; i < addrlen, ++i) {
              char *begin_name = 0, *begin_offset = 0, *end_offset = 0;
              p = symbollist[i];

              while (!begin_name && *p != 0) {
                     if (*p == '(') {
                            *p++ = 0;
                            begin_name = p;
                     }
              }

              while (!begin_offset && *p != 0) {
                     if (*p == '+') {
                            *p++ = 0;
                            begin_offset = p;
                     }
              }

              int status;
              char *ret = abi::__cxa_demangle(begin_name, 0, 0, &status);

              if (status == 0) {
                     msg << "  " << symbollist[i] << "("
                         << ret << "+" << begin_offset << "\n";
                     free(ret);
              } else {
                     msg << "  " << symbollist[i] << "("
                         << begin_name << "+" << begin_offset << "\n";
              }
       }

       free(symbollist);
}
#else
void append_stacktrace(ostream& os)
{
       os << "\nStack unwinding not implemented for this platform\n";
}
#endif

NS_MIA_END
