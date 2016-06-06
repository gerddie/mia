/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/export_handler.hh>


#include <mia/3d/imageio.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>
#include <mia/core/tools.hh>

NS_MIA_BEGIN

const char *io_3dimage_data::data_descr = "3dimage";

C3DImageVector *C3DImageVector::clone() const
{
	return new C3DImageVector(*this);
}


template <> const char *  const 
TPluginHandler<C3DImageIOPlugin>::m_help =  
   "These plug-ins handle loading and storing of 3D images to the supported image file types.";


P3DImageVector EXPORT_3D create_image3d_vector(P3DImage image)
{
	P3DImageVector result(new C3DImageVector);
	result->push_back(image);
	return result;
}

P3DImage  EXPORT_3D load_image3d(const std::string& filename)
{
	cvdebug() << "Load 3D images from '" << filename <<"'\n"; 
	auto in_image_list = C3DImageIOPluginHandler::instance().load(filename);
	if (in_image_list.get() && !in_image_list->empty())
		return (*in_image_list)[0];
	else
		throw create_exception<std::runtime_error>( "load_image3d: unable to load 3D image from '", filename, "'");

}

bool EXPORT_3D  save_image(const std::string& filename, P3DImage image)
{
	cvinfo() << "Save image to '" << filename << "'\n"; 
	C3DImageVector out_images;
	out_images.push_back(image);
	return C3DImageIOPluginHandler::instance().save(filename, out_images);
}


bool  EXPORT_3D save_image(const std::string& filename, C3DImage& image)
{
      return save_image(filename, P3DImage(&image, void_destructor<C3DImage>())); 
}

template class TPlugin<io_3dimage_data, io_plugin_type>;
template class TIOPlugin<io_3dimage_data>;
template class THandlerSingleton<TIOPluginHandler<C3DImageIOPlugin> >;
template class TIOPluginHandler<C3DImageIOPlugin>;
template class TPluginHandler<C3DImageIOPlugin>;

NS_MIA_END

