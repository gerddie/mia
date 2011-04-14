/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

//


#ifndef mia_core_streamredir_hh
#define mia_core_streamredir_hh

#include <mia/core/defines.hh>

#include <iostream>
#include <sstream>

NS_MIA_BEGIN

/**
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
	char *m_cur;
	char *m_end;
};

NS_MIA_END

#endif
