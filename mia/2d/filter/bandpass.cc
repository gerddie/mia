/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

/*! \brief basic type of a plugin handler

A bandpass filter

\file bandpass_2dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/bandpass.hh>
#include <mia/internal/bandpass.cxx>


NS_MIA_BEGIN
template class EXPORT mia::TBandPass<mia::C2DImage>;
template class EXPORT mia::TBandPassFilterPlugin<mia::C2DImage>;

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DImageBandPassFilterPlugin;
}
NS_MIA_END
