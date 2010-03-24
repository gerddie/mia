/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2009 - 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/2d/transformio.hh>
#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>


NS_MIA_BEGIN;

const char *io_2dtransform_type::type_descr = "transformio";


template class EXPORT_2D TIOPlugin<io_2dtransform_type>;
template class EXPORT_2D THandlerSingleton<TIOPluginHandler<C2DTransformIOPlugin> >;
template class EXPORT_2D TIOPluginHandler<C2DTransformIOPlugin>;
template class EXPORT_2D TPluginHandler<C2DTransformIOPlugin>;


NS_MIA_END

