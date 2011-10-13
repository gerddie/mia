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

#include <cassert>
#include <iomanip>
#include <mia/core/fixedwidthoutput.hh>

NS_MIA_BEGIN

using namespace std; 

CFixedWidthOutput::CFixedWidthOutput(std::ostream& os, size_t width):
	m_os(os), 
	m_width(width), 
	m_pos(0), 
	m_offset(0)
{
}

void CFixedWidthOutput::push_offset(size_t offset)
{
	m_stack.push(m_offset); 
	m_offset += offset;
}

void CFixedWidthOutput::pop_offset()
{
	assert(!m_stack.empty()); 
	m_offset = m_stack.top(); 
	m_stack.pop(); 
}

void CFixedWidthOutput::reset_offset()
{
	while (!m_stack.empty()) {
		m_stack.pop(); 
	}
	m_offset = 0;
}

void CFixedWidthOutput::newline()
{
	m_os << '\n'; 
	if ( m_offset )
		m_os << setw(m_offset) << " "; 
	m_pos = m_offset;
}

void CFixedWidthOutput::write(const std::string& text)
{
	auto is = text.begin(); 
	auto es = text.end(); 

	while (is != es) {
		if (*is == '\n') {
			newline(); 
			++is;
		}else if (*is == '\t') {
			if (m_pos + 8 < m_width) {
				m_pos += 8; 
				m_os << setw(8) << " "; 
			}else {
				newline(); 
			}
			++is; 
		}else if (isspace(*is)) {
			++m_pos;
			if (m_pos < m_width) 
				m_os << *is; 
			else
				newline(); 
			++is; 
		}else {
			auto hs = is;
			size_t endpos = m_pos; 
			
			// search end of next word 
			while (*hs != *es && !isspace(*hs)) {
				++endpos;
				++hs; 
			}
			// word fits, so write it 
			if (endpos < m_width) {
				m_pos = endpos; 
				while (is != hs) 
					m_os << *is++;
			}else { //  newline, tab and write word regardless of size 
				newline(); 
				while (is != hs) {
					m_os << *is++;
					++m_pos;
				}
			}
		}
	}
}

NS_MIA_END
