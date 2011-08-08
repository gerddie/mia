/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

/*
  LatexBeginPluginSection{2D image similarity kernels}
  \label{sec:cost2d}
  
  2D image similarity kernels evaluate the according similarity measure between 
  two images. These kernels may be used standalone, like e.g. in linear registration, 
  or will be called from generalized image similarity cost plug-ins that also take 
  care of transforming and scaling the images during the image registration process. 

  In the following $\Omega$ is the image domain, and $S:\Omega\rightarrow \R$ stands  
  for the study or floating image, and  $R:\Omega\rightarrow \R$ for the reference or fixed image. 

  LatexEnd
*/


#include <mia/core/export_handler.hh>

#include <mia/2d/cost.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>
#include <mia/core/cost.cxx>

NS_MIA_BEGIN

template class EXPORT_HANDLER TCost<C2DImage, C2DFVectorfield>;
EXPLICIT_INSTANCE_HANDLER(C2DImageCost);

NS_MIA_END

