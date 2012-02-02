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

#ifndef mia_core_paramoption_hh
#define mia_core_paramoption_hh

#include <mia/core/cmdoption.hh>
#include <mia/core/parameter.hh>


NS_MIA_BEGIN
/**
   \brief command line option that handles a parameter 

   This command line option handles a parameter that is set to a certain value given 
   on the command line (contraty to just setting a flag). 
*/
class CParamOption : public CCmdOption {
public: 
	/**
	   Conrtructor to create the command line option. 
	   \param short_option the single letter option value, can be zero for none 
	   \param long_opt the long option name 
	   \param param the actual parameter, this value must be allocated by calling \a new, 
	   destruction is taken care of by this class. 
	 */
	CParamOption(char short_option, const char *long_opt, CParameter *param);
	
private:
	virtual void do_post_set(); 

	virtual const std::string do_get_value_as_string() const; 
	virtual void do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& handler_map) const; 
	virtual void do_get_long_help(std::ostream& os) const;
	virtual bool do_set_value(const char *str_value);
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const;

	std::unique_ptr<CParameter> m_param; 

}; 

NS_MIA_END

#endif
