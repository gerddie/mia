/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/core/export_handler.hh>

#include <mia/2d/2dimageio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>
#include <mia/core/errormacro.hh>

NS_MIA_BEGIN

const char *io_2dimage_type::type_descr = "2dimage";

C2DImageVector *C2DImageVector::clone() const
{
	return new C2DImageVector(*this);
}

C2DImageIOPPH::C2DImageIOPPH(const std::list< ::boost::filesystem::path>& searchpath):
	TIOPluginHandler<C2DImageIOPlugin>(searchpath)
{
	C2DFVectorTranslator::register_for("pixel");
}

template class EXPORT_HANDLER TIOPlugin<io_2dimage_type>;
template class EXPORT_HANDLER THandlerSingleton<C2DImageIOPPH>;
template class EXPORT_HANDLER TIOHandlerSingleton<C2DImageIOPPH>;
template class EXPORT_HANDLER TIOPluginHandler<C2DImageIOPlugin>;
template class EXPORT_HANDLER TPluginHandler<C2DImageIOPlugin>;


P2DImageVector EXPORT_2D create_image2d_vector(P2DImage image)
{
	P2DImageVector out_list_ptr(new C2DImageIOPluginHandler::Instance::Data);
	out_list_ptr->push_back(image);
	return out_list_ptr;
}

P2DImage load_image2d(const std::string& filename)
{
	C2DImageIOPluginHandler::Instance::PData  in_image_list =
		C2DImageIOPluginHandler::instance().load(filename);

	if (!in_image_list || in_image_list->empty()) {
		THROW(runtime_error, filename << " no image  found");
	}
	if (in_image_list->size() > 1) {
		cvwarn() << "load_image2d:'" << filename
			 << "' more then one image found, loading only first one\n";
	}

	return *in_image_list->begin();
}

bool save_image2d(const std::string& filename, P2DImage image)
{
	C2DImageIOPluginHandler::Instance::Data out_image_list;
	out_image_list.push_back(image);
	return C2DImageIOPluginHandler::instance().save("", filename, out_image_list);
}

NS_MIA_END
