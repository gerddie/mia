/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/3d/filter/load.hh>
#include <mia/3d/imageio.hh>

NS_BEGIN( load_3dimage_filter)

NS_MIA_USE; 

C3DLoad::C3DLoad(const std::string& name):
	m_name(name)
{
}

mia::P3DImage C3DLoad::do_filter(const mia::C3DImage& MIA_PARAM_UNUSED(image)) const
{
	return load_image3d(m_name); 
	
}

C3DLoadFilterPluginFactory::C3DLoadFilterPluginFactory(): 
	C3DFilterPlugin("load")
{
	add_parameter("file", new CStringParameter(m_filename, CCmdOptionFlags::required_input,
						   "name of the input file to load from.", 
						   &C3DImageIOPluginHandler::instance()));
}

mia::C3DFilter *C3DLoadFilterPluginFactory::do_create()const
{
	return new C3DLoad(m_filename); 
}

const std::string C3DLoadFilterPluginFactory::do_get_descr()const
{
	return "Load the input image from a file and use it to replace the current image in the pipeline."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLoadFilterPluginFactory();
}
NS_END
