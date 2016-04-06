/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/2d/morphshape.hh>
#include <stdexcept>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

using std::invalid_argument; 

C2DMorphShape::C2DMorphShape():
	m_foreground_mask(new C2DShape()), 
	m_background_mask(new C2DShape())
{
}

C2DMorphShape::C2DMorphShape(P2DShape foreground_mask, P2DShape background_mask):
	m_foreground_mask(foreground_mask), 
	m_background_mask(background_mask)
{
	for (auto f = m_foreground_mask->begin(); f != m_foreground_mask->end(); ++f) {
		if (m_background_mask->has_location(*f)) {
			throw invalid_argument("C2DMorphShape: background and foreground mask overlap"); 
		}
	}
}
 
	
	
void C2DMorphShape::add_pixel(const value_type& pixel, bool foreground)
{
	if (!m_background_mask.unique()) 
		m_background_mask = P2DShape(new C2DShape(*m_background_mask)); 
	if (!m_foreground_mask.unique()) 
		m_foreground_mask = P2DShape(new C2DShape(*m_foreground_mask)); 

	
	if (foreground) {
		if (m_background_mask->has_location(pixel)) {
			throw create_exception<invalid_argument>( "Pixel location ", pixel , 
			      " can't be added to the foreground mask, since it it already "
			      " in the background mask"); 
		}
		m_foreground_mask->insert(pixel); 
	}else{
		if (m_foreground_mask->has_location(pixel)) {
			throw create_exception<invalid_argument>( "Pixel location ", pixel, 
			      " can't be added to the background mask, since it it already "
			      " in the foreground mask"); 
		}
		m_background_mask->insert(pixel); 
	}
}

void C2DMorphShape::add_pixel(int x, int y, bool foreground)
{
	add_pixel(value_type(x,y), foreground);
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

const char *C2DMorphShape::type_descr = "morphshapes";


/*
   - This could be parallized
   - using a bit-image with the infamous vector<bool> implementation is probably a 
     bad idea. 
 */

size_t morph_hit_and_miss_2d(C2DBitImage& target, const C2DBitImage& source, const C2DMorphShape& shape)
{
	assert(target.get_size() == source.get_size()); 
	
	size_t changed_pixels = 0; 

	const C2DBounds& size = source.get_size();
	auto res_i = target.begin();
	auto src_i = source.begin(); 
	
	
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++res_i, ++src_i) {
			bool hit = true; 

			auto fgi = shape.get_foreground_mask().begin(); 
			auto fge = shape.get_foreground_mask().end();

			while (hit && fgi != fge) {
				C2DBounds nl(x + fgi->x, y + fgi->y);
				if (nl < size)
					hit &= source(nl); 
				++fgi; 
			}
			
			auto bgi = shape.get_background_mask().begin(); 
			auto bge = shape.get_background_mask().end();
			
			while (hit && bgi != bge) {
				C2DBounds nl(x + bgi->x, y + bgi->y);
				if (nl < size)
					hit &= !source(nl); 
				++bgi; 
			}
			*res_i = hit; 
			if (hit != *src_i) 
				++changed_pixels; 
		}
	return changed_pixels; 
}

/*
   - This could be parallized
   - using a bit-image with the infamous vector<bool> implementation is probably a 
     bad idea. 
 */

size_t morph_thinning_2d(C2DBitImage& target, const C2DBitImage& source, const C2DMorphShape& shape)
{
	assert(target.get_size() == source.get_size()); 
	
	size_t changed_pixels = 0; 

	const C2DBounds& size = source.get_size();
	auto res_i = target.begin();
	auto src_i = source.begin(); 
	
	copy(source.begin(), source.end(), target.begin()); 
	
	for (size_t y = 0; y < size.y; ++y)
		for (size_t x = 0; x < size.x; ++x, ++res_i, ++src_i) {
			bool hit = true; 

			auto fgi = shape.get_foreground_mask().begin(); 
			auto fge = shape.get_foreground_mask().end();

			while (hit && fgi != fge) {
				C2DBounds nl(x + fgi->x, y + fgi->y);
				if (nl < size)
					hit &= source(nl); 
				++fgi; 
			}
			
			auto bgi = shape.get_background_mask().begin(); 
			auto bge = shape.get_background_mask().end();
			
			while (hit && bgi != bge) {
				C2DBounds nl(x + bgi->x, y + bgi->y);
				if (nl < size)
					hit &= !source(nl); 
				++bgi; 
			}
			if (hit) {
				*res_i = false; 
				if (*src_i)
					++changed_pixels; 
			}
		}
	return changed_pixels; 
}

NS_MIA_END
