/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <sstream>
#include <stdexcept>
#include <mia/core/msgstream.hh>
#include <mia/core/cmdoption.hh>

using std::ostream; 
using std::string; 
using std::ostringstream; 
using std::invalid_argument;

NS_MIA_BEGIN

const bool CCmdOption::not_required = false; 
const bool CCmdOption::required = true; 

CCmdOption::CCmdOption(char short_opt, const char *long_opt, 
		       const char *long_help, const char *short_help, bool required):
	m_short_opt(short_opt), 
	m_long_opt(long_opt),
	m_long_help(long_help), 
	m_short_help(short_help),
	m_flags(required)
{
	TRACE_FUNCTION; 
	cvdebug() << "Create option '" << long_opt << "'\n"; 
        assert(long_opt);
        assert(long_help);

}

CCmdOption::~CCmdOption()
{
}

size_t CCmdOption::get_needed_args() const
{
	return do_get_needed_args();
}


void CCmdOption::add_option(CShortoptionMap& sm, CLongoptionMap& lm)
{
	do_add_option(sm, lm);
}

void CCmdOption::print_short_help(std::ostream& os) const
{
	do_print_short_help(os);
}

void CCmdOption::get_long_help(std::ostream& os) const
{
	os << long_help();
	do_get_long_help(os);
}

void CCmdOption::do_get_long_help(std::ostream& /*os*/) const
{
}

string CCmdOption::get_long_help_xml(xmlpp::Element& parent, HandlerHelpMap& handler_map) const
{
	cvdebug() << "write XML for '" << m_long_opt << "'\n"; 
	ostringstream shelp; 
	shelp << long_help();
	do_get_long_help_xml(shelp, parent, handler_map); 
	return shelp.str(); 
}

void CCmdOption::do_get_long_help_xml(std::ostream& os, xmlpp::Element& /*parent*/, HandlerHelpMap& /*handler_map*/) const
{
	do_get_long_help(os);
}

const char *CCmdOption::long_help() const
{
	return m_long_help;
}

char CCmdOption::get_short_option() const
{
	return m_short_opt; 
}

const char *CCmdOption::get_long_option() const
{
	return m_long_opt;
}

void CCmdOption::clear_required()
{
	m_flags = false; 
}

bool CCmdOption::is_required() const
{
	return m_flags; 
}

void CCmdOption::get_opt_help(std::ostream& os) const
{
	do_get_opt_help(os);
}

void   CCmdOption::write_value(std::ostream& os) const
{
	do_write_value(os);
}

const std::string CCmdOption::get_value_as_string() const
{
	return do_get_value_as_string();
}

const std::string CCmdOption::do_get_value_as_string() const
{
	return "";
}

size_t CCmdOption::do_get_needed_args() const
{
	return 1; 
}

#if 0 
void CCmdOption::do_get_long_help(std::ostream& /*os*/) const
{
}
#endif 

void CCmdOption::do_print_short_help(std::ostream& os) const
{
	if ( m_short_opt ) {
		os << '-' << m_short_opt; 
		if (  m_short_help )
			os << " " << m_short_help;
		os  << " ";
	}
}

void CCmdOption::do_get_opt_help(std::ostream& os) const
{
	if ( get_short_option() )
		os  << "  -" << get_short_option() << " ";
	else
		os  << "     ";
	os << "--" << get_long_option();
	write_value(os);
	os << " ";
}

void CCmdOption::set_value(const char *str_value)
{
	bool result;

	try {
		result = do_set_value(str_value);
	}

	catch (std::invalid_argument& x) {
		ostringstream msg;
		msg << "Error parsing --" << get_long_option() << ":" << x.what();
		throw std::invalid_argument(msg.str());
	}

	if (!result) {
		throw invalid_argument(string("Error parsing option '") +
				       string(get_long_option()) +
				       string("' from input '") +
				       string(str_value) + string("'"));
	}
	clear_required();
}

void CCmdOption::do_add_option(CShortoptionMap& sm, CLongoptionMap& lm)
{
	TRACE_FUNCTION;
	if (get_short_option() != 0) {
		assert(sm.find(get_short_option()) == sm.end() &&
		       "don't add the same short option twice" );
		sm[get_short_option()] = this;
	}

	if (get_long_option() != 0) {
		assert(lm.find(get_long_option()) == lm.end() &&
		       "don't add the same long option twice");
		lm[get_long_option()] = this;
	}
}

void CCmdOption::post_set()
{
	do_post_set(); 
}

void CCmdOption::do_post_set()
{
}

NS_MIA_END
