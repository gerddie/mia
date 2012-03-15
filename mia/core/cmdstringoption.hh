/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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


#include <mia/core/cmdoption.hh>

NS_MIA_BEGIN

/**
    \ingroup infrastructure 

    \brief The command line option that expects a string 

    This command line option expects  a string a parameter. 
    It supports giving a hint to the help system that the actual string will be used for 
    to construct a plug-in based object. 

*/
class EXPORT_CORE CCmdStringOption : public CCmdOption {
 public:
	/**
	   Construct a string option 
	   \param[in,out] value at input the default value, at output the value read from the command line 
	   \param short_opt the one letter command line option 
	   \param long_opt the long command line option 
	   \param long_help the full help string that describes the option completely 
	   \param required  set to true if the parameter must be given by the user 
	   \param plugin_hint if the string will later be used to create an object by using plug-in then pass 
	   a pointer to the corresponding plug-in handler to give a hint the help system about this connection.
	 */
	CCmdStringOption(std::string& value, char short_opt, const char *long_opt, const char *long_help, 
			 bool required, const CPluginHandlerBase *plugin_hint);
 private: 
	bool do_set_value(const char *str_value);
	void do_write_value(std::ostream& os) const;
	void do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& handler_map) const; 
	const std::string do_get_value_as_string() const; 


	std::string& m_value; 
	const CPluginHandlerBase *m_plugin_hint; 
	
}; 
NS_MIA_END