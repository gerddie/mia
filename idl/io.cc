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

#include <stdio.h>
#include <idl_export.h>

#include <mia/2d.hh>
#include <idl/helpers.hh>

using namespace std; 
using namespace mia; 


IDL_VPTR read2dimage_callback(int argc, IDL_VPTR *argv) 
{
	if (argc != 1) 
		throw invalid_argument("expect exactly one argument"); 

	const string fname = idl_get_string("model", argv[0]);
	
	C2DImageIOPluginHandler::Instance::PData  in_image_list = C2DImageIOPluginHandler::instance().load(fname);
	
	if (in_image_list.get() && !in_image_list->empty()) {
		return mia2idl_image2d(**in_image_list->begin()); 
	}else 
		throw runtime_error(fname + ": file or image not found"); 
}
