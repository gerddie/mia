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


#include <mia/core/cmdtranslatoroption.hh>

NS_MIA_BEGIN

using std::shared_ptr; 

void CValueTranslator::add_translator(CTranslator *t)
{
        assert(t); 
        
        auto name = t->get_name(); 
        assert(m_translators.find( name ) != m_translators.end()); 
        
        m_translators[name] = shared_ptr<CTranslator>(t); 
        post_add_translator(*t);
}

bool CValueTranslator::set_value(const CComplexOptionParser::CParts& options)
{
        // select the sub-parameter set 
        
        auto backend = m_translators.find( options.first ); 
        if (backend == m_translators.end()) {
                throw create_exception<invalid_argument>("CValueTranslator: unknown set translator '", 
                                                         options.first, "'requested"); 
        }
        
        return do_set_value(*backend.second, options.second);
}

void CValueTranslator::write_value(std::ostream& os) const
{
        do_write_value(os); 
}

CCmdTranslatorOption::CCmdTranslatorOption(const ValueTranslator & vt, char short_opt, const char *long_opt, 
                                           const char *long_help):
        CCmdOption(short_opt, long_opt, long_help, short_help, flags),
        m_vt(vt)
{
}

bool CCmdTranslatorOption::do_set_value(const char *str_value)
{
        CComplexOptionParser param_list(str_value);
        
        if (param_list.size() < 1) {
		throw create_exception<std::invalid_argument>( "CCmdTranslatorOption: Description string '"
		      , str_value , "' can not be interpreted. "); 
	}
        if (param_list.size() != 1) {
                cvwarn() << "The  CCmdTranslatorOption currently only accepts one parameter set, "
                        "ignoring additional " << param_list.size() << " parameter set(s)\n"; 
        }
        
        m_vt.set_value(param_list[0]);
}

void CCmdTranslatorOption::do_write_value(std::ostream& os) const
{
        m_vt.write_value(os); 
}


NS_MIA_END
