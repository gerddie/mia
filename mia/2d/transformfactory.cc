/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
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

#include <mia/2d/transformfactory.hh>
#include <mia/internal/transformfactory.cxx>

NS_MIA_BEGIN

template class TTransformCreator<C2DTransformation>; 

C2DTransformCreatorPlugin::C2DTransformCreatorPlugin(const char *const name):
	TFactory<C2DTransformCreator>(name), 
	m_image_interpolator("bspline:d=3"),
	m_image_boundary("mirror")
{
	add_parameter("imgkernel", new CStringParameter(m_image_interpolator, false, "image interpolator kernel")); 
	add_parameter("imgboundary", new CStringParameter(m_image_boundary, false, "image interpolation boundary conditions")); 
}


C2DTransformCreatorPlugin::Product *C2DTransformCreatorPlugin::do_create() const
{
	C2DInterpolatorFactory ipf(m_image_interpolator, m_image_boundary); 
	return do_create(ipf); 
}

C2DTransformCreatorHandlerTestPath::C2DTransformCreatorHandlerTestPath()
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);
}


EXPLICIT_INSTANCE_DERIVED_FACTORY_HANDLER(C2DTransformCreator, C2DTransformCreatorPlugin); 


NS_MIA_END
