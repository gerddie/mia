/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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

/* 
   LatexBeginPlugin{2D full cost functions}

      
   \subsubsection*{DivCurl smoothing measure}
   \label{fullcost2d:divcurl}
   
   \begin{description}
   
   \item [Plugin:] divcurl
   \item [Description:] Evaluate the DivCurl measure of a given transformation $T$. 

   \begin{equation}
   F_{\text{divcurl}} := \int_{\Omega} w_{\text{div}}\left\| \div(T(\vx)) \right\|^2  + 
                                       w_{\text{curl}}\left\| \curl(T(\vx)) \right\|^2 d\vx.
   \end{equation}
   
   \plugtabstart
   weight &  float & all-over weight of the cost function & 1.0  \\
   div &  float & weight $w_{\text{div}}$ of the divergence part of the transformation & 1.0  \\
   curl &  float & weight $w_{\text{curl}}$ of the rotation part of the transformation  & 1.0  \\
   \plugtabend
   
   \item [Remark:] Using this cost function only makes sense for spline based transformations  
             \ref{transform2d:spline} with spline degree 3 or higher, or densely defined 
             transfromations \ref{transform2d:vf}
   \end{description}

   LatexEnd
*/


#include <limits>
#include <algorithm>
#include <boost/lambda/lambda.hpp>
#include <mia/2d/fullcost/divcurl.hh>

#include <mia/internal/divcurl.cxx>
NS_MIA_BEGIN

template class TDivcurlFullCostPlugin<C2DTransformation>; 
template class  TDivCurlFullCost<C2DTransformation>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DDivCurlFullCostPlugin();
}

NS_MIA_END



