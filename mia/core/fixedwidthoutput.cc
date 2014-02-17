/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <cassert>
#include <iomanip>
#include <mia/core/fixedwidthoutput.hh>

NS_MIA_BEGIN

using namespace std; 

CFixedWidthOutput::CFixedWidthOutput(std::ostream& os, size_t width):
	m_os(os), 
	m_width(width), 
	m_pos(0), 
	m_offset(0), 
	m_line_continue(false)
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

void CFixedWidthOutput::set_linecontinue(bool value)
{
	m_line_continue = value; 
}

void CFixedWidthOutput::newline()
{
	if (m_line_continue) 
		m_os << '\\'; 
	m_os << '\n'; 
	if ( m_offset )
		m_os << setw(m_offset) << " "; 
	m_pos = m_offset;
}

void CFixedWidthOutput::write(const std::string& text)
{
	size_t cur_width = m_line_continue ? m_width -1 : m_width; 

	auto is = text.begin(); 
	auto es = text.end(); 

	while (is != es) {
		if (*is == '\n') {
			newline(); 
			++is;
		}else if (*is == '\t') {
			if (m_pos + 8 < cur_width) {
				m_pos += 8; 
				m_os << setw(8) << " "; 
			}else {
				newline(); 
			}
			++is; 
		}else if (isspace(*is)) {
			++m_pos;
			if (m_pos < cur_width) 
				m_os << *is; 
			else
				newline(); 
			++is; 
		}else {
			auto hs = is;
			size_t endpos = m_pos; 
			
			// search end of next word 
			while (hs != es && !isspace(*hs)) {
				++endpos;
				++hs; 
			}
			// word fits, so write it 
			if (endpos < cur_width) {
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
