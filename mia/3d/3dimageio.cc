
/* -*- mia-c++  -*-
 *
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

#ifdef WIN32
#  define EXPORT_HANDLER __declspec(dllexport)
#else
#  define EXPORT_HANDLER
#endif

#include <mia/3d/3dimageio.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

const char *io_3dimage_type::type_descr = "3dimage";

C3DImageVector *C3DImageVector::clone() const
{
	return new C3DImageVector(*this);
}


template class EXPORT_3D TIOPlugin<io_3dimage_type>;
template class EXPORT_3D THandlerSingleton<TIOPluginHandler<C3DImageIOPlugin> >;
template class EXPORT_3D TIOHandlerSingleton<TIOPluginHandler<C3DImageIOPlugin> >;
template class EXPORT_3D TIOPluginHandler<C3DImageIOPlugin>;
template class EXPORT_3D TPluginHandler<C3DImageIOPlugin>;

P3DImageVector EXPORT_3D create_image3d_vector(P3DImage image)
{
	P3DImageVector result(new C3DImageVector);
	result->push_back(image);
	return result;
}

P3DImage load_image3d(const std::string& filename)
{
	C3DImageIOPluginHandler::Instance::PData  in_image_list = C3DImageIOPluginHandler::instance().load(filename);
	if (in_image_list.get() && !in_image_list->empty())
		return (*in_image_list)[0];
	else
		THROW(runtime_error, "unable to load 3D image from '" << filename << "'");
}

bool save_image3d(const std::string& filename, P3DImage image)
{
	C3DImageVector out_images;
	out_images.push_back(image);
	return !C3DImageIOPluginHandler::instance().save("", filename, out_images);
}


NS_MIA_END
