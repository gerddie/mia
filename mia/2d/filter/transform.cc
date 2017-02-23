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

#include <mia/2d/filter/transform.hh>
#include <mia/2d/transformio.hh>

NS_BEGIN( transform_2dimage_filter)

NS_MIA_USE; 

C2DTransform::C2DTransform(const std::string& name):
	m_name(name)
{
}

mia::P2DImage C2DTransform::do_filter(const mia::C2DImage& image) const
{
        auto transform = load_transform<P2DTransformation>(m_name); 
        return (*transform)(image); 
	
}

C2DTransformFilterPluginFactory::C2DTransformFilterPluginFactory(): 
	C2DFilterPlugin("transform")
{
	add_parameter("file", new CStringParameter(m_filename, CCmdOptionFlags::required_input,
						   "Name of the file containing the transformation.", 
						   &C2DTransformationIOPluginHandler::instance()));
}

mia::C2DFilter *C2DTransformFilterPluginFactory::do_create()const
{
	return new C2DTransform(m_filename); 
}

const std::string C2DTransformFilterPluginFactory::do_get_descr()const
{
	return "Transform the input image with the given transformation."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DTransformFilterPluginFactory();
}
NS_END
