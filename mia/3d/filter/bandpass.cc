/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
   LatexBeginPluginDescription{3D image filters}
   
   \subsection{Bandpass intensity filter}
   \begin{description}
   \item [Plugin:]bandpass
   \item [Description:]An intensity band pass filter - all pixels with intensities
   outside the given range are set to zero, all other pixels remain the
   same ($x:=x\in [min,max]\:?\: x\::\:0$
   \item [Parameters:] min, max
   
   \plugtabstart
   min&  float&  lower bound of the bandpass range &0\\\hline
   max&  float&  upper bound of the bandpass range &3.40282e+38\\\hline
   \end{tabular}
   \end{description}
   
  LatexEnd
*/


#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/filter/bandpass.hh>

NS_MIA_BEGIN
template class EXPORT mia::TBandPass<mia::C3DImage>;
template class EXPORT mia::TBandPassFilterPlugin<mia::C3DImage>;

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DImageBandPassFilterPlugin;
}
NS_MIA_END
