/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// $Id: miaIOStream.cc 932 2006-07-03 19:17:23Z write1 $

/*! \brief A verbose output stream

*/

#include <iostream>
#include <fstream>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

const TDictMap<vstream::Level>::Table verbose_dict[] = {
#ifndef NDEBUG
	{"trace", vstream::ml_trace},
	{"debug", vstream::ml_debug},
#endif
	{"info", vstream::ml_info},
	{"message", vstream::ml_message},
	{"warning", vstream::ml_warning},
	{"error", vstream::ml_error},
	{"fail", vstream::ml_fail},
	{"fatal", vstream::ml_fatal},
	{NULL, vstream::ml_undefined},
};

const TDictMap<vstream::Level> g_verbose_dict(verbose_dict);

vstream::vstream(std::ostream& output, Level l):
	_M_output(&output),
	_M_output_level(l),
	_M_message_level(ml_fatal)
{
}

void vstream::set_verbosity(Level l)
{
	_M_output_level = l;
}

vstream& vstream::instance()
{
	static vstream verb(_M_output_target ? *_M_output_target :
			    std::cerr, vstream::ml_fail);
	return verb;
}

std::ostream *vstream::_M_output_target = 0;

void vstream::set_output_target(std::ostream* os)
{
	_M_output_target = os;
}

vstream& vstream::operator << (Level l)
{
	_M_message_level = l;
	if (_M_message_level >= _M_output_level) {

		switch (_M_message_level) {
		case ml_debug:  *_M_output << "DEBUG:"; break;
		case ml_info:  *_M_output << "INFO:"; break;
		case ml_message:break;
		case ml_warning:*_M_output << "WARNING:"; break;
		case ml_fail:   *_M_output << "FAILED:"; break;
		case ml_error:  *_M_output << "ERROR:"; break;
		case ml_fatal:  *_M_output << "FATAL:"; break;
		default:        *_M_output << "TRACE:";
		}
	}

	return *this;
}

std::ostream&  vstream::set_stream(std::ostream& os)
{
	std::ostream& old_os = *_M_output;
	_M_output = &os;
	return old_os;
}

vstream & vstream::operator<<(std::ostream& (*f)(std::ostream&))
{
	if (_M_message_level >= _M_output_level) {
		*_M_output << f;
	}
	return *this;
}


void set_verbose(bool verbose)
{
	vstream::instance().set_verbosity(verbose ? vstream::ml_message : vstream::ml_error);
}

#ifndef NDEBUG
size_t CTrace::_M_depth = 0;
#endif

NS_MIA_END
