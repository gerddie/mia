/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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
   LatexBeginPlugin{2D image filters}
   
   \subsection{Pixel value conversion}
   \label{filter2d:convert}
   
   \begin{description}
   \item [Plugin:]convert
   \item [Description:] Pixel conversion filter
   \item [Conversions:] Mapping methods 
   \begin{description}
   \item [copy:] pixel values are copied, values outside of the target range are clipped
   \item [linear:] pixel values are transformed according to $x \rightarrow a \times x + b$ , 
       values outside of the target range are clipped
   \item [opt:] The range of the input pixels \emph{found in the input image} 
       is mapped to the range of the output type. 
   \item [range:] The range of the input pixel \emph{type} is mapped to the range of the output type. 
   \end{description}
   \item [Parameters:] a, b, map, repn
   
   \plugtabstart
   a&  float&  scaling for linear mapping & 1.0\\\hline
   b&  float&  shift of linear mapping    & 0.0\\\hline
   map& string & type of mapping (copy|linear|opt|range) & opt\\\hline
   repn & string & target pixel type ( sbyte | ubyte | sshort | ushort |
				sint | uint | slong | ulong | float | double) & ubyte \\\hline
   \plugtabend
   
   \end{description}

   LatexEnd  
 */


#include <mia/2d/filter/convert.hh>
#include <mia/internal/convert.cxx>

NS_MIA_BEGIN
template class EXPORT mia::TConvert<mia::C2DImage>;
template class EXPORT mia::TConvertFilterPlugin<mia::C2DImage>;


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new TConvertFilterPlugin<mia::C2DImage>();
}

NS_MIA_END
