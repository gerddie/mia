/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/3d/filter/tee.hh>
#include <mia/3d/imageio.hh>

NS_BEGIN( tee_3dimage_filter)

NS_MIA_USE; 

C3DTee::C3DTee(const std::string& name):
	m_name(name)
{
}

template <class T>
C3DTee::result_type C3DTee::operator () (const T3DImage<T>& data) const
{
	TRACE_FUNCTION; 
	P3DImage result(new T3DImage<T>(data)); 
	save_image(m_name, result); 
	return result; 
}


mia::P3DImage C3DTee::do_filter(const mia::C3DImage& image) const
{
	return mia::filter(*this, image); 
	
}

mia::P3DImage C3DTee::do_filter(mia::P3DImage image) const
{
	save_image(m_name, image); 
	return image; 
}


C3DTeeFilterPluginFactory::C3DTeeFilterPluginFactory(): 
	C3DFilterPlugin("tee")
{
	add_parameter("file", new CStringParameter(m_filename, true,
						   "name of the output file to save the image too.", 
						   &C3DImageIOPluginHandler::instance()));
}

mia::C3DFilter *C3DTeeFilterPluginFactory::do_create()const
{
	return new C3DTee(m_filename); 
}

const std::string C3DTeeFilterPluginFactory::do_get_descr()const
{
	return "Save the input image to a file and also pass it through to the next filter"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DTeeFilterPluginFactory();
}
NS_END
