/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
public:
	static const CWatch& instance();
	
	
	/// \returns  get the current time value
	double get_seconds() const;
protected:
	CWatch();
private:

	virtual double do_get_seconds() const = 0;
	
	CWatch(const CWatch& w) = delete;
	CWatch(CWatch&& w) = delete;
	CWatch& operator = (const CWatch& w) = delete;
	CWatch& operator = (CWatch&& w) = delete;

	
	
};

NS_MIA_END

#endif
