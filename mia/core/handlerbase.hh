/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#ifndef mia_core_handlerbase_hh
#define mia_core_handlerbase_hh

#include <mia/core/defines.hh>

#include <ostream>
#include <fstream>

NS_MIA_BEGIN




/** 
    \ingroup infrastructure 
    
    \brief The base class for all plugin handlers

    This clas provides aome basic funcionallity that is common to all plugin handlers. 
    In addition, the copy constructor and assigment operator are deleted in order to forbit copying 
    of instances of this class. 
    
*/


class EXPORT_CORE CPluginHandlerBase  {
 public: 
	CPluginHandlerBase() = default; 

	/**
	   forbid copying - doxygen should drop these from the documentation 
	 */
	CPluginHandlerBase(const CPluginHandlerBase& other) = delete; 

	/**
	   forbid copying - doxygen should drop these from the documentation 
	 */
	CPluginHandlerBase& operator  = (const CPluginHandlerBase& other) = delete;

	virtual ~CPluginHandlerBase();

	/**
	   Print out the short help to an output stream 
	   @param os
	*/
	void print_short_help(std::ostream& os) const; 

	/**
	   Print out the help to an output stream 
	   @param os
	 */
	void print_help(std::ostream& os) const; 
 private: 
	
	virtual void do_print_short_help(std::ostream& os) const = 0; 
	virtual void do_print_help(std::ostream& os) const = 0; 

}; 


NS_MIA_END


#endif
