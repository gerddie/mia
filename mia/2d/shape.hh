/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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


#ifndef mia_2d_shape_hh
#define mia_2d_shape_hh

#include <mia/core/shape.hh>
#include <mia/core/factory.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

typedef TShape<T2DVector, C2DBitImage> C2DShape;
typedef std::shared_ptr<C2DShape > P2DShape;

typedef TFactory<C2DShape> C2DShapePlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C2DShapePlugin> > C2DShapePluginHandler;

NS_MIA_END

#endif

