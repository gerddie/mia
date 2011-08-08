/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#include <mia/3d.hh>
#include <idl/idl_helpers.hh>

#define IDL_Message_call(x,y,z)  IDL_Message(x,y,z)

using namespace std; 
using namespace mia; 
namespace bfs = boost::filesystem; 


template <typename T> 
static void save_image(const C3DImage& image, void *out_data)
{
	T *p = (T *)out_data; 

	const T3DImage<T>& img = dynamic_cast<const T3DImage<T>&>(image); 
	copy(img.begin(), img.end(), p); 
}


int filter3d_internal(int argc, void *argv [])
{
	enum idx {
		ifdescr = 0, 
		inx, 
		iny, 
		inz, 
		iintype, 
		iindata,
		iouttype, 
		ioutdata, 
		ilast
	}; 

	if (argc != ilast) {
		stringstream s; 
		s << "unsupoorted number "<< argc <<" of arguments; Expect" << ilast; 
		throw invalid_argument(s.str()); 
	}
	
	cvdebug() << "Start fiter\n"; 
	
	IDL_INT nx = *(IDL_INT *)argv[inx]; 
	IDL_INT ny = *(IDL_INT *)argv[iny]; 
	IDL_INT nz = *(IDL_INT *)argv[inz]; 

	cvdebug() << "copy input image\n"; 
	P3DImage image = get_3dimage(nx, ny, nz, (IDL_STRING *)argv[iintype], argv[iindata]);

	
	cvdebug() << "Get plugin handler\n"; 
	const C3DFilterPluginHandler::Instance& ph = C3DFilterPluginHandler::instance();

	string filter_descr = idl_get_string((IDL_STRING *)argv[ifdescr]); 
	
	cvdebug() << "Create filter: '" << filter_descr <<"'\n"; 
	C3DFilterPlugin::ProductPtr f = ph.produce(filter_descr.c_str()); 
	if (!f) {
		stringstream s; 
		s << "unable to create filter from  '"<< filter_descr << "'"; 
		throw invalid_argument(s.str()); 
	}
	
	cvdebug() << "Run Filter\n"; 
	P3DImage result = f->filter(*image);


	string out_type = idl_get_string((IDL_STRING *)argv[iintype]); 

	cvdebug() << "Convert to output type '" << out_type << "'\n"; 
	EPixelType out_pix_type = CPixelTypeDict.get_value(out_type.c_str());

	if (out_pix_type != result->get_pixel_type()) {
		if (out_pix_type != it_bit) {
			stringstream convert_descr; 
			convert_descr << "convert:map=opt,type="<< out_type; 
			C3DFilterPlugin::ProductPtr conv = ph.produce(convert_descr.str().c_str()); 
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

