/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
   LatexBeginPluginDescription{3D image similarity kernels}
   
   \subsection{Sum of Squared Differences}
   \label{cost3d:ssd}
   
   \begin{description}
   
   \item [Plugin:] ssd
   \item [Description:] Evaluates the Sum of Squared Differences similarity measure between two images:
   \begin{equation}
      F_\text{SSD} (S,R) = \frac{1}{2} \int_{\Omega} \left( S(x) - R(x) \right)^2 \text{d}x 
   \end{equation}
   \item [Study:] An abitrary gray scale or binary images 
   \item [Reference:] An abitrary gray scale or binary images 
   \end{description}
   
   This plug-in doesn't take additional parameters. 

   LatexEnd  
 */


#include <mia/3d/cost.hh>

#define NS ssd_3dimage_cost
#include <mia/3d/cost/ssd.hh>

NS_BEGIN(NS)


NS_MIA_USE;
using namespace std;
using namespace boost;


template class TSSDCost<C3DImageCost>;


const string C3DSSDCostPlugin::do_get_descr()const
{
	return "3D imaga cost: sum of squared differences";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DSSDCostPlugin();
}


NS_END


