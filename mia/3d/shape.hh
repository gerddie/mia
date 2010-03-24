/* -*- mia-c++  -*-
 *
 * Copyright (c) 2007 - 2010 Madrid
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef mia_3d_shape_hh
#define mia_3d_shape_hh

#include <mia/core/shape.hh>
#include <mia/core/factory.hh>
#include <mia/3d/3DImage.hh>

NS_MIA_BEGIN

typedef TShape<T3DVector, C3DBitImage> C3DShape;
typedef SHARED_PTR(C3DShape) P3DShape;

typedef TFactory<C3DShape, C3DImage, shape_type> C3DShapePlugin;
typedef THandlerSingleton<TFactoryPluginHandler<C3DShapePlugin> > C3DShapePluginHandler;

NS_MIA_END

#endif

