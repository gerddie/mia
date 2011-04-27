/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Evolutionary Anthropoloy
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

/* LatexBeginPlugin{2D image filters}
   
   \subsubsection*{Intensity invert}
   \label{filter2d:invert}
   
   \begin{description}
   
   \item [Plugin:] invert
   \item [Description:] invert the image intensities. Inversion of an image $I$ is done by a linearly mapping using 
     $\forall x in I: x \rightarrow (\max(I) - x + \min(I))$ 
   
   \item [Input:] A gray scale image of abitrary pixel type. 
   \item [Output:] The inverted image 
   
   \end{description}
   
   This plug-in doesn't take parameters. 

   LatexEnd  
 */



#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/invert.hh>
#include <mia/internal/invert.cxx>

NS_MIA_BEGIN
template class EXPORT mia::TInvert<mia::C2DImage>;
template class EXPORT mia::TInvertFilterPlugin<mia::C2DImage>;

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DImageInvertFilterPlugin;
}
NS_MIA_END
