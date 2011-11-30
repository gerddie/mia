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
  LatexBeginPluginDescription{2D neighborhood shapes}


  \subsection{Spherical shape}
  \label{shapes2d:sphere}

  \begin{description}
   
   \item [Plugin:] sphere 
   \item [Description:] provide a 2D sphere like shape that includes its interior. 
   
   \end{description}
   \plugtabstart
   r &  float & radius of the sphere & 2 \\
   \plugtabend
   
  LatexEnd
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <limits>


#include "sphere.hh"

NS_MIA_BEGIN

using namespace std;

CSphere2DShapeFactory::CSphere2DShape::CSphere2DShape(float radius)
{
	int hw = static_cast<int>(radius + 1.0);
	float r2 = radius * radius;

	for (int y = -hw; y <= hw; ++y) {
		float y2 = y * y;
		for (int x = -hw; x <= hw; ++x) {
			if (y2 + x*x <= r2)
				insert(C2DShape::Flat::value_type(x,y));
		}
	}
}


CSphere2DShapeFactory::CSphere2DShapeFactory():
	C2DShapePlugin("sphere"),
	m_r(2)
{
	add_parameter("r", new CFloatParameter(m_r, 0, numeric_limits<float>::max(), false, "sphere radius"));
}

C2DShape *CSphere2DShapeFactory::do_create()const
{
	return new CSphere2DShape(m_r);
}


const string CSphere2DShapeFactory::do_get_descr()const
{
	return string("spherical shape mask creator");
}

NS_MIA_END
