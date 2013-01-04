/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/vfio.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_BEGIN

class C2DVFIOPluginHandlerTestPath {
public: 
	C2DVFIOPluginHandlerTestPath(); 
}; 

/** 
    @cond INTERNAL  
    \ingroup test 
    \brief Class to initialiaze the plug-in search path fot testing without installing the plug-ins 
*/
struct EXPORT_2D C2DTransformCreatorHandlerTestPath {
	C2DTransformCreatorHandlerTestPath(); 
private: 
	CSplineKernelTestPath spktp; 
}; 


class C2DTransformationIOPluginHandlerTestPath {
public: 
        C2DTransformationIOPluginHandlerTestPath(); 
private: 
        C2DTransformCreatorHandlerTestPath tch; 
}; 


/// @endcond 
NS_MIA_END

