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

#include <mia/3d/imagecollect.hh>

NS_MIA_BEGIN
using std::invalid_argument; 

C3DImageCollector::C3DImageCollector(size_t slices):
	m_slices(slices),
	m_cur_slice(0)
{
}

template <typename T>
void C3DImageCollector::operator ()(const T2DImage<T>& image) {
	
	if (!m_image)
		m_image = P3DImage(new T3DImage<T>(C3DBounds(image.get_size().x,
							     image.get_size().y,
							     m_slices)));

	auto *out_image = dynamic_cast<T3DImage<T> *>(m_image.get());
	if (!out_image)
		throw invalid_argument("input images are not all of the same type");

	if (m_cur_slice < m_slices) {
		if (out_image->get_size().x != image.get_size().x ||
		    out_image->get_size().y != image.get_size().y)
			throw invalid_argument("input images are not all of the same size");
		
		size_t offset = image.get_size().x; 
		offset *= image.get_size().y; 
		offset *= m_cur_slice; 
		
		cvdebug() << "offset=" << offset << "\n"; 
		auto out = out_image->begin() + image.get_size().x * image.get_size().y * m_cur_slice;
		

		copy(image.begin(), image.end(), out);
		++m_cur_slice; 
	}else
		throw create_exception<invalid_argument>("C3DImageCollector: expected number of slices (", m_slices, ") exhausted"); 
}

void C3DImageCollector::add(const C2DImage &image) 
{
	mia::accumulate(*this, image); 
}

P3DImage C3DImageCollector::get_result() const {
	return m_image;
}

NS_MIA_END
