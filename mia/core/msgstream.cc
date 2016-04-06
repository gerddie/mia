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

#include <iostream>
#include <fstream>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

const TDictMap<vstream::Level>::Table verbose_dict[] = {
#ifdef ENABLE_DEBUG_MESSAGES
	{"trace", vstream::ml_trace, "Function call trace"},
	{"debug", vstream::ml_debug, "Debug output"},
#endif
	{"info", vstream::ml_info, "Low level messages"},
	{"message", vstream::ml_message, "Normal messages"},
	{"warning", vstream::ml_warning, "Warnings"},
	{"error", vstream::ml_error, "Report errors"},
	{"fail", vstream::ml_fail, "Report test failures"},
	{"fatal", vstream::ml_fatal, "Report only fatal errors"},
	{NULL, vstream::ml_undefined, ""},
};

const TDictMap<vstream::Level> g_verbose_dict(verbose_dict);

vstream::vstream(std::ostream& output, Level l):
	m_output_level(l)
{
	m_output = &output; 
}

void vstream::set_verbosity(Level l)
{
	m_output_level = l;
}

vstream& vstream::instance()
{
	static vstream verb(m_output_target ? *m_output_target :
			    std::cerr, vstream::ml_fail);
	return verb;
}

std::ostream *vstream::m_output_target = 0;

void vstream::set_output_target(std::ostream* os)
{
	m_output_target = os;
}

vstream& vstream::operator << (Level l)
{
	m_message_level = l;
	if (m_message_level >= m_output_level) {

		switch (m_message_level) {
		case ml_debug:  
#ifndef NDEBUG			
			*m_output << "DEBUG:"; 
#endif 
			break;
		case ml_info:  *m_output << "INFO:"; break;
		case ml_message:break;
		case ml_warning:*m_output << "WARNING:"; break;
		case ml_fail:   *m_output << "FAILED:"; break;
		case ml_error:  *m_output << "ERROR:"; break;
		case ml_fatal:  *m_output << "FATAL:"; break;
		default:        *m_output << "TRACE:";
		}
	}

	return *this;
}

std::ostream&  vstream::set_stream(std::ostream& os)
{
	std::ostream& old_os = *m_output;
	m_output = &os;
	return old_os;
}

vstream & vstream::operator<<(std::ostream& (*f)(std::ostream&))
{
	if (m_message_level >= m_output_level) {
		*m_output << f;
	}
	return *this;
}


void set_verbose(bool verbose)
{
	vstream::instance().set_verbosity(verbose ? vstream::ml_message : vstream::ml_error);
}

__thread size_t CTrace::m_depth = 0;
__thread std::ostream* vstream::m_output;
__thread vstream::Level vstream::m_message_level = vstream::ml_fatal; 

NS_MIA_END
