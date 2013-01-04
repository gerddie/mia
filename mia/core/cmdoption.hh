/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_core_cmdoption_hh
#define mia_core_cmdoption_hh

#include <string>
#include <memory>
#include <iostream>
#include <mia/core/defines.hh>
#include <mia/core/handlerbase.hh>
#include <libxml++/libxml++.h>

NS_MIA_BEGIN

class CCmdOption;

/// Class to provide a maping from short option names to options
typedef std::map<char,  CCmdOption *>        CShortoptionMap;

/// Class to provide a maping from long option names to options
typedef std::map<std::string,  CCmdOption *> CLongoptionMap;


/** 
   \ingroup cmdline
   
   \brief The base class for all command line options. 
   
   The base class of all possible command line options. It defines the interface
   of the options as well as some basic functionality to create help strings. 
*/
class EXPORT_CORE CCmdOption  {
public:

	/// readability constant to show that an option is not required 
	static const bool not_required; 
	
	/// readability constant to show that an option is required 
	static const bool required; 
	
        /** The constructor
	    \param short_opt the short option character 
	    \param long_opt the long option name
	    \param long_help a long help string
	    \param short_help help to print out when only usage information is requested 
	    \param required set to true if the option must be set by the user 
        */
	CCmdOption(char short_opt, const char *long_opt, const char *long_help, 
		   const char *short_help, bool required);

        /// ensure virtual destruction
	virtual ~CCmdOption();

        /** Add this option to the given option maps
	    \param sm a \a CShortoptionMap to add this option to
	    \param lm a \a CLongoptionMap to add this option to
        */
	void add_option(CShortoptionMap& sm, CLongoptionMap& lm);

        /** \returns how many arguments after the option argument on the command 
	    line will be used by this option
        */
	size_t get_needed_args() const;

        /** Writes out the value of the option to an output stream
	    \param os the output stream to write this options value to
        */
	void   write_value(std::ostream& os) const;

        /** Set the value of the option parameter based on the input string
	    \param str_value the string value to be vonverted to the option value
        */
	void set_value(const char *str_value);

        /// \returns the long name of the option
	const char *get_long_option() const;

        /// \returns the long name of the option
	char get_short_option() const;

        /** Print the short help of the option to an output stream
	    \param os the output stream
        */
	void print_short_help(std::ostream& os) const;

        /** Print the help of the option to an output stream
	    \param os the output stream
         */
        void get_opt_help(std::ostream& os)const;

        /** Print the long option name of the option to an output stream
	    \param os the output stream
         */
	void get_long_help(std::ostream& os)const;

        /// \returns the options value as string
	const std::string get_value_as_string() const;

	/// @returns true if the option is a required option and hasn't been set
	bool is_required() const; 

        /// \returns the long help string
	const char *long_help() const;

	/**
	   Returns help string tailered for the XML help creation system and adds
	   used plug-in handlers to the helper map
	   \param parent the parent xml node to which to add documentation 
	   \param[in,out] handler_map the map of possibely recoursively called plug-in handlers 
	   \returns the text relate to the help 
	   \remark the text should probably be added to the parent node instead. 
	 */
	std::string get_long_help_xml(xmlpp::Element& parent, HandlerHelpMap& handler_map) const; 

	/** This function must be called after the parameter was set through a string in order to handle 
	    complex initialization like done for factory based command line parameters 
	 */
	void post_set(); 
protected:

	/// clear the "required" flag 
	void clear_required(); 
	
	/** 
	    write the long help string to a stream 
	    \param os the output stream 
	*/
	virtual void do_get_long_help(std::ostream& os) const;
private:
	const char *get_short_help() const;

	virtual void do_add_option(CShortoptionMap& sm, CLongoptionMap& lm);
	virtual void do_print_short_help(std::ostream& os) const;
	virtual void do_get_opt_help(std::ostream& os) const;

	virtual bool do_set_value(const char *str_value) = 0;
	virtual size_t do_get_needed_args() const;
	virtual void do_write_value(std::ostream& os) const = 0;

	virtual const std::string do_get_value_as_string() const;
	virtual	void do_post_set(); 

	virtual void do_get_long_help_xml(std::ostream& os, xmlpp::Element& parent, HandlerHelpMap& handler_map) const; 

	char m_short_opt; 
	const char *m_long_opt;
	const char *m_long_help;
	const char *m_short_help;
	bool m_flags;
};

/// a shared pointer definition of the Option
typedef std::shared_ptr<CCmdOption > PCmdOption;

NS_MIA_END

#endif
