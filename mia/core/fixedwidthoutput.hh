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

#include <ostream>
#include <string>
#include <stack>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

class EXPORT_CORE CFixedWidthOutput {
public: 
	CFixedWidthOutput(std::ostream& os, size_t width);
	
	void push_offset(size_t offset);  
	void pop_offset();  
	void reset_offset();  
	void write(const std::string& text);
	void newline(); 
private: 
	std::ostream& m_os; 
	size_t m_width; 
	size_t m_pos; 
	size_t m_offset;
	std::stack<size_t> m_stack; 
}; 

NS_MIA_END
