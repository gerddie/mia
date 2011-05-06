/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef ph_optparam_hh
#define ph_optparam_hh

#include <ostream>

#include <mia/core/shared_ptr.hh>

#include <mia/core/parameter.hh>
#include <mia/core/optionparser.hh>



NS_MIA_BEGIN

/**
   \brief A class to hold a list of named parameters.

    It takes care of setting the parameters based on its names,
    and can check whether required parameters are really set.
*/

class EXPORT_CORE CParamList {

public:

	/// a shared pointer to a parameter to make handling easier
	typedef std::shared_ptr<CParameter > PParameter;

	/**
	   \param key
	   \returns true if the parameter list already has a parameter named \a key
	*/
	bool has_key(const std::string& key) const;

	/**
	   The operator to access the parameters in the list
	   \param key the name of the parameter
	   \returns (shared) pointer to the associated parameter (or creates a new empty one)
	*/
	PParameter& operator [] (const std::string& key);

	/**
	   Set all the parameters that are given in the option list. If a parameter is unknown to the list,
	   it throws an \a invalid_argument exception
	   \param options a map of <key, value> string pairs
	*/
	void set(const CParsedOptions& options);

	/**
	   checks whether all required parameters have been set and throws an
	   \a invalid_argument exception if not.
	*/
	void check_required() const;

	/**
	   prints out help strings for all parameters in the list
	   \param os the output stream to write the help to.
	*/
	void print_help(std::ostream& os)const;
private:
	std::map<std::string, PParameter> m_params;
};

NS_MIA_END

#endif
