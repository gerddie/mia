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

#include <ostream>
#include <string>
#include <stack>
#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup logging 
   \brief This class provides formatted writing to a text console 

   This class is used to write output to a ostream and the text is formatted 
   to not exeed the given width (e.g. the width of the text console stdout. 
   The class is used to write out the formated help of CCmdOptionList.

*/


class EXPORT_CORE CFixedWidthOutput {
public: 
	/**
	   Construct the class 
	   \param os the output stream to write to 
	   \param width the maximal width of an output line 
	 */
	CFixedWidthOutput(std::ostream& os, size_t width);
	
	/**
	   Set a new offset from the left margin, the original offset is 
	   stored in a stack, 
	   \param offset 
	 */
	void push_offset(size_t offset);  
	/**
	   restore the last offset 
	 */
	void pop_offset();  

	/**
	   Reset the offset to zero and clear the offset stack. 
	 */
	void reset_offset();  

	/**
	   Write the text to the output. New line '\\n' and tabulators '\\t' are honoured. 
	   At a new-line the line start positionis set to the actual offset. 
	   If a word doesn't fit the current line, a line break is  inserted automatically. 
	   \param text
	*/
	void write(const std::string& text);
	/**
	   Force a line break. 
	 */
	void newline(); 
	/**
	   Set to true if newline() should insert '\' to indicate  a continuing line 
	   (e.g. for writing code examples that don't fit on one line) 
	   \param value 
	*/
	void set_linecontinue(bool value); 
private: 
	std::ostream& m_os; 
	size_t m_width; 
	size_t m_pos; 
	size_t m_offset;
	bool m_line_continue; 
	std::stack<size_t> m_stack; 
}; 

NS_MIA_END
