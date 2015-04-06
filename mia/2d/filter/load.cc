/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/2d/filter/load.hh>
#include <mia/2d/imageio.hh>

NS_BEGIN( load_2dimage_filter)

NS_MIA_USE; 

C2DLoad::C2DLoad(const std::string& name):
	m_name(name)
{
}

mia::P2DImage C2DLoad::do_filter(const mia::C2DImage& MIA_PARAM_UNUSED(image)) const
{
	return load_image2d(m_name); 
	
}

C2DLoadFilterPluginFactory::C2DLoadFilterPluginFactory(): 
	C2DFilterPlugin("load")
{
	add_parameter("file", new CStringParameter(m_filename, CCmdOptionFlags::required_input,
						   "name of the input file to load from.", 
						   &C2DImageIOPluginHandler::instance()));
}

mia::C2DFilter *C2DLoadFilterPluginFactory::do_create()const
{
	return new C2DLoad(m_filename); 
}

const std::string C2DLoadFilterPluginFactory::do_get_descr()const
{
	return "Load the input image from a file and use it to replace the current image in the pipeline."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLoadFilterPluginFactory();
}
NS_END
