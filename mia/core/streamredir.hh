/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
	int overflow(int);
	int sync();
	std::streamsize xsputn ( const char * s, std::streamsize n );
private:
	void put_buffer(void);
	void put_char(int);

	virtual void do_put_buffer(const char *begin, const char *end) = 0;
	char *_M_begin;
	char *_M_cur;
	char *_M_end;
};

NS_MIA_END

#endif
