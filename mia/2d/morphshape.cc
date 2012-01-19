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

#include <mia/2d/morphshape.hh>
#include <stdexcept>

NS_MIA_BEGIN

C2DMorphShape::C2DMorphShape():
	m_foreground_mask(new C2DShape()), 
	m_background_mask(new C2DShape())
{
}

C2DMorphShape::C2DMorphShape(P2DShape foreground_mask, P2DShape background_mask):
	m_foreground_mask(foreground_mask), 
	m_background_mask(background_mask)
{
	auto not_found = m_background_mask->end(); 
	for (auto f = m_foreground_mask->begin(); f != m_foreground_mask->end(); ++f) {
		if (m_background_mask->has_location(*f)) {
			throw invalid_argument("C2DMorphShape: background and foreground mask overlap"); 
		}
	}
}
 
	
	
void C2DMorphShape::add_pixel(const value_type& pixel, bool foreground)
{
	if (foreground) {
		if (m_background_mask->has_location(pixel)) {
			THROW(invalid_argument, "Pixel location " << pixel << 
			      " can't be added to the foreground mask, since it it already "
			      " in the background mask"); 
		}
		m_foreground_mask->insert(pixel); 
	}else{
		if (m_foreground_mask->has_location(pixel)) {
			THROW(invalid_argument, "Pixel location " << pixel << 
			      " can't be added to the background mask, since it it already "
			      " in the foreground mask"); 
		}
		m_background_mask->insert(pixel); 
	}
}

const C2DShape& C2DMorphShape::get_foreground_mask() const 
{
	return *m_foreground_mask; 
}

const C2DShape& C2DMorphShape::get_background_mask() const
{
	return *m_background_mask; 
}


C2DMorphShape C2DMorphShape::rotate_by_90() const
{
	return C2DMorphShape(rotate_90_degree(*m_foreground_mask), 
			     rotate_90_degree(*m_background_mask)); 
}

NS_MIA_END

