/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#ifndef mia_core_cmdbooloption_hh
#define mia_core_cmdbooloption_hh

#include <mia/core/cmdoption.hh>

NS_MIA_BEGIN

/**
    \ingroup cmdline

    \brief The command line option that sets a flag to true when given 

    This command line option expects no parameter. If given the associated boolen value 
    will be set to true. Therefore, at initialization this boolean value is always forces 
    to be set to false.  

*/
class EXPORT_CORE CCmdBoolOption : public CCmdOption {
 public:
	/**
	   Construct a bool option 
	   \param[in,out] value at input the default value, at output the value read from the command line 
	   \param short_opt the one letter command line option 
	   \param long_opt the long command line option 
	   \param long_help the full help bool that describes the option completely 
	   \param flags specific flags for the option. Note that passing CCmdOptionFlags::required doesn't make sense. 
	*/
	CCmdBoolOption(bool& value, char short_opt, const char *long_opt, const char *long_help, CCmdOptionFlags flags);
 private: 
	bool do_set_value(const char *str_value);
	void do_write_value(std::ostream& os) const;
	void do_get_long_help_xml(std::ostream& os, CXMLElement& parent, HandlerHelpMap& handler_map) const; 
	const std::string do_get_value_as_string() const; 
	size_t do_get_needed_args() const;

	bool& m_value; 
}; 
NS_MIA_END

#endif
