/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/3d/imagedraw.hh>
#include <limits>

NS_MIA_BEGIN


/**
   \brief implements T3DImage as a target for drawing operations 

*/
template <typename T> 
T3DImageDrawTarget<T>::T3DImageDrawTarget(const C3DBounds& size, const C3DFVector& origin, const C3DFVector& spacing):
        C3DDrawBox(size, origin, spacing),
        m_target(size)
{
        m_target.set_voxel_size(spacing); 
        m_target.set_origin(origin); 
        
        m_color = std::numeric_limits<T>::is_integer ? std::numeric_limits<T>::max() : 1.0;
}
        
template <typename T> 
const T3DImage<T>& T3DImageDrawTarget<T>::get_image() const
{
        return m_target; 
}

template <typename T> 
void T3DImageDrawTarget<T>::set_color(T c)
{
        m_color = c; 
}

template <typename T> 
void T3DImageDrawTarget<T>::do_draw_point(const C3DBounds& p)
{
        m_target(p) = m_color; 
}

NS_MIA_END
