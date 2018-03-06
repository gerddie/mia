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

#ifndef mia_core_paramtranslator_hh
#define mia_core_paramtranslator_hh

#include <map>
#include <set>
#include <string>
#include <ostream>

#include <mia/core/defines.hh>
#include <mia/core/optparam.hh>

NS_MIA_BEGIN

class EXPORT_CORE CParamTranslator
{
public:
       CParamTranslator(const char *name);

       virtual ~CParamTranslator();

       /**
         Add a parameter to the parameter list. If the name of
         the parameter already exists this function throws an \a
         invalid_argument exception.
         \param name Name of the new parameter
         \param param the actual parameter
       */
       void add_parameter(const std::string& name, CParameter *param);

       /**
         Set the parameter according to the given option map. If the
         parameter name does not exists, the function will throw an
         \a invalid_argument exception. Depending on the parameter type
         setting it might also throw an \a invalid_argument exception.
         \param options the options map
       */
       void set_parameters(const CParsedOptions& options);

       /**
         This function checks, whether all requzired parameters have really been set.
         It throws an \a invalid_argument
       */
       void check_parameters();

       void get_short_help(std::ostream& os) const;

       void get_help(std::ostream& os) const;

       void get_help_xml(CXMLElement& root) const;

       virtual void do_get_help_xml(CXMLElement& root) const;

       /// \returns the name of the translator
       const char *get_name() const;

       /// \returns the description of the translator
       const std::string get_descr() const;
protected:
       CParamList& get_parameters();

private:
       virtual const std::string do_get_descr() const = 0;

       CParamList  m_parameters;

       // plugin name
       const char *m_name;

};

NS_MIA_END

#endif
