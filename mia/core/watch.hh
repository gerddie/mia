/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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


#ifndef mia_core_watch_h
#define mia_core_watch_h

#include <mia/core/defines.hh>

NS_MIA_BEGIN
/** 
    \ingroup misc
    \brief A class for measuring time. 
    
    This is a class to measure the process run-time time with millisecond accuracy. 
  
    
    \remark On WIN32 this class is currently a fake that always returns 0.0; 
 */
class EXPORT_CORE CWatch {
	static int overlaps;
public:
	/** construct a watch
	    \todo Make this a singelton, since no second instance is needed
	 */
	CWatch();

	/// \returns  get the current time value
	double get_seconds() const;
private:
	static void overlap_handler(int p_sig);
};

NS_MIA_END

#endif
