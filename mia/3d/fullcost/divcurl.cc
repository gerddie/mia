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
   LatexBeginPluginDescription{3D full cost functions}

      
   \subsection{DivCurl smoothing measure}
   \label{fullcost3d:divcurl}
   
   \begin{description}
   
   \item [Plugin:] divcurl
   \item [Description:] Evaluate the DivCurl measure of a given transformation $T$. 

   \begin{equation}
   F_{\text{divcurl}} := \int_{\Omega} w_{\text{div}}\left\| \nabla \nabla  \cdot (T(\vx)) \right\|^2  + 
                                       w_{\text{curl}}\left\| \nabla \curl(T(\vx)) \right\|^2 d\vx.
   \end{equation}
   
   \plugtabstart
   weight &  float & all-over weight of the cost function & 1.0  \\
   div &  float & weight $w_{\text{div}}$ of the divergence part of the transformation & 1.0  \\
   curl &  float & weight $w_{\text{curl}}$ of the rotation part of the transformation  & 1.0  \\
   \plugtabend
   
   \item [Remark:] Using this cost function only makes sense for spline based transformations  
             \ref{transform3d:spline} with spline degree 3 or higher, or densely defined 
             transfromations \ref{transform3d:vf}
   \end{description}

   LatexEnd
*/


#include <mia/3d/fullcost/divcurl.hh>
#include <mia/internal/divcurl.cxx>

NS_MIA_BEGIN

template class TDivcurlFullCostPlugin<C3DTransformation>; 
template class  TDivCurlFullCost<C3DTransformation>; 

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DDivCurlFullCostPlugin();
}

NS_MIA_END



