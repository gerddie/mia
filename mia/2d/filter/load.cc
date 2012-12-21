/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <mia/2d/filter/load.hh>

NS_BEGIN( load_2dimage_filter)

using namespace mia; 
using std::string; 


C2DLoadImage::C2DLoadImage(const string& name):
m_name(name)
{
}

P2DImage C2DLoadImage::do_filter(const C2DImage& MIA_PARAM_UNUSED(image)) const
{
        return load_image<P2DImage>(m_name); 
}


C2DLoadImageFilterPlugin::C2DLoadImageFilterPlugin():
	C2DFilterPlugin("load")
{
	add_parameter("file", new CStringParameter(m_filename, true,
						   "name of the input file to load the image from.", 
						   &C2DImageIOPluginHandler::instance()));
}
	

C2DFilter *C2DLoadImageFilterPlugin::do_create()const
{
	return new C2DLoadImage(m_filename); 
}

const string C2DLoadImageFilterPlugin::do_get_descr()const
{
	return "Discard the incoming image and replace it by the loaded image";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLoadImageFilterPlugin();
}
NS_END
