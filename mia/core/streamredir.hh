/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#ifndef mia_core_streamredir_hh
#define mia_core_streamredir_hh

#include <mia/core/defines.hh>

#include <iostream>
#include <sstream>

NS_MIA_BEGIN

/**
   \ingroup logging
   \brief base class to redirect output streams

   This class implements the abstract base class for stream output redirection
   to e.g. a text window or a console.
   The specialization must implement the do_put_buffer method, which writes out
   the characters from \a begin to \a end using a function of the output device.
*/
class EXPORT_CORE streamredir: public std::streambuf   {
public:
	streamredir();
	virtual ~streamredir();
protected:
	
	/**
	   implement the overflow function to force output of buffer 
	   @param c next character to write 
	   @returns 0 
	 */
	int overflow(int c);
	/**
	   Force writing of buffer. 
	   @returns 0 
	 */
	int sync();

	/**
	   implement function to handle \a newline and \a return properly 
	   @param s string to write 
	   @param n number of bytes in string 
	   @return number of chars written 
	*/
	std::streamsize xsputn ( const char * s, std::streamsize n );
private:
	void put_buffer(void);
	void put_char(int);

	/// this function needs to be overwritten in order to write to a specific output 
	virtual void do_put_buffer(const char *begin, const char *end) = 0;
	char *m_begin;
};

NS_MIA_END

#endif
