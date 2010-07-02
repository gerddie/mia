/*
 * Copyright (c) Leipzig, Madrid 2004-2010
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
#include <mia/core.hh>
#include <mia/2d.hh>

#include <idl/idl_helpers.hh>

#define IDL_Message_call(x,y,z)  IDL_Message(x,y,z)

using namespace std; 
using namespace mia; 
namespace bfs = boost::filesystem; 


int filter2d_internal(int argc, void *argv [])
{
	const int ifdescr = 0; 
	const int inx = 1; 
	const int iny = 2; 
	const int iintype = 3;
	const int iindata = 4;  
	const int iouttype = 5; 
	const int ioutdata = 6; 

	if (argc != 7) {
		stringstream s; 
		s << "unsupoorted number "<< argc <<" of arguments"; 
		throw invalid_argument(s.str()); 
	}
	
	cvdebug() << "Start fiter\n"; 
	
	IDL_INT nx = *(IDL_INT *)argv[inx]; 
	IDL_INT ny = *(IDL_INT *)argv[iny]; 

	cvdebug() << "copy input image\n"; 
	P2DImage image = get_2dimage(nx, ny, (IDL_STRING *)argv[iintype], argv[iindata]);

	
	cvdebug() << "Get plugin handler\n"; 
	const C2DFilterPluginHandler::Instance& ph = C2DFilterPluginHandler::instance();

	string filter_descr = idl_get_string((IDL_STRING *)argv[ifdescr]); 
	
	cvdebug() << "Create filter: '" << filter_descr <<"'\n"; 
	C2DFilterPlugin::ProductPtr f = ph.produce(filter_descr.c_str()); 
	if (!f) {
		stringstream s; 
		s << "unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	
	cvdebug() << "Run Filter\n"; 
	P2DImage result = f->filter(*image);


	string out_type = idl_get_string((IDL_STRING *)argv[iintype]); 

	cvdebug() << "Convert to output type '" << out_type << "'\n"; 
	EPixelType out_pix_type = CPixelTypeDict.get_value(out_type.c_str());

	if (out_pix_type != result->get_pixel_type()) {
		if (out_pix_type != it_bit) {
			stringstream convert_descr; 
			convert_descr << "convert:map=opt,type="<< out_type; 
			C2DFilterPlugin::ProductPtr conv = ph.produce(convert_descr.str().c_str()); 
			result = conv->filter(*result);
		}else {
			throw invalid_argument("filter32d: automatic conversion to bit pixel type not supported, use binarize");
		}
	}

	cvdebug() << "Copy to output\n"; 
	
	save_image(*result, (IDL_STRING *)argv[iouttype],  argv[ioutdata]); 

	cvdebug() << "SUCCESS\n\n";
	return 0; 
}
