/* -*- mia-c++ -*-
 * Copyright (c) 2007 Gert Wollny <gert at die.upm.es>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <idl/helpers.hh>


using namespace std; 
using namespace mia; 

IDL_VPTR filter2d_callback(int argc, IDL_VPTR *argv)
{
	enum IDX {
		idx_filter = 0, 
		idx_image, 
		idx_numarg
	}; 

	if (argc != idx_numarg) {
		throw invalid_argument("filter2d: only support exactly one argument"); 
	}
	
	P2DImage input = idl2mia_image2d(argv[idx_image]);
	string filter_descr = idl_get_string(argv[idx_filter]); 
	
	C2DFilterPlugin::ProductPtr f = C2DFilterPluginHandler::instance().produce(filter_descr.c_str()); 
	if (!f) {
		stringstream s; 
		s << "filter2d: unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	cvdebug() << "Start filtering\n"; 
	P2DImage result = f->filter(*input);
	cvdebug() << "Done Filtering\n"; 
	
	return mia2idl_image2d(*result); 
}
 
IDL_VPTR filter3d_callback(int argc, IDL_VPTR *argv)
{
	enum IDX {
		idx_filter = 0, 
		idx_image, 
		idx_numarg
	}; 

	if (argc != idx_numarg) {
		throw invalid_argument("filter2d: only support exactly one argument"); 
	}
	
	P3DImage input = idl2mia_image3d(argv[idx_image]);
	string filter_descr = idl_get_string(argv[idx_filter]); 
	
	C3DFilterPlugin::ProductPtr f = C3DFilterPluginHandler::instance().produce(filter_descr.c_str()); 
	if (!f) {
		stringstream s; 
		s << "filter3d: unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	cvdebug() << "Start filtering\n"; 
	P3DImage result = f->filter(*input);
	cvdebug() << "Done Filtering\n"; 
	
	return mia2idl_image3d(*result); 
}


