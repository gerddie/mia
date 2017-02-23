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

#define VSTREAM_DOMAIN "3dtransformio"
#include <mia/core/export_handler.hh>

#include <mia/core/errormacro.hh>
#include <mia/3d/transformio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

NS_MIA_BEGIN

C3DTransformIOPluginHandlerImpl::C3DTransformIOPluginHandlerImpl()
{
	C3DFVectorTranslator::register_for(C3DTransformation::input_spacing_attr);
	C3DFVectorTranslator::register_for(C3DTransformation::output_spacing_attr);

}

template <> const char *  const 
	TPluginHandler<C3DTransformationIO>::m_help =  
       "These plug-ins implement support for loading and saving 3D transformations to various file types.";


template class TPlugin<C3DTransformation, io_plugin_type>;
template class TIOPlugin<C3DTransformation>;
template class THandlerSingleton<C3DTransformIOPluginHandlerImpl>;
template class TIOPluginHandler<C3DTransformationIO>;
template class TPluginHandler<C3DTransformationIO>;

NS_MIA_END
