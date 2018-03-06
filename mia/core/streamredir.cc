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

#include <mia/core/streamredir.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using namespace std;

streamredir::streamredir():
       m_begin(new char[2048])
{
       setp(m_begin, m_begin + 2048);
}

streamredir::~streamredir()
{
       delete[] m_begin;
}

int streamredir::overflow(int c)
{
       cvdebug() << "streamredir::overflow:" << c << "\n";
       put_buffer();
       sputc(c);
       return 0;
}

streamsize streamredir::xsputn ( const char *s, streamsize n )
{
       streamsize i = 0;

       while (i < n) {
              if (*s == '\n' || *s == '\r')
                     put_buffer();
              else
                     sputc(*s);

              ++s;
              ++i;
       }

       return i;
}

int streamredir::sync()
{
       put_buffer();
       return 0;
}

void streamredir::put_char(int c)
{
       cvdebug() << "streamredir::put_char:" << c << "\n";

       if (c == '\n' || c == '\r')
              put_buffer();
       else {
              cverr() << "Handle put_char " << (char)c << "?\n";
       }
}

void streamredir::put_buffer(void)
{
       do_put_buffer(pbase(), pptr());
       setp(pbase(),  epptr());
}

NS_MIA_END
