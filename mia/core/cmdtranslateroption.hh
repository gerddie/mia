/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


#ifndef mia_core_cmdtranslatoroption_hh
#define mia_core_cmdtranslatoroption_hh

#include <mia/core/cmdoption.hh>
#include <mia/core/optionparser.hh>

NS_MIA_BEGIN



class CParamizedFunctor : public CParamTranslator  {
public: 
        CParamizedFunctor(const char *name); 

        class Selector {
                void add_functor(CParamizedFunctor *t); 
                
                void set_value(const char *str_value); 
                
                void write_value(std::ostream& os) const;
                
                static instance(); 
	private: 
		virtual void post_add_translator(CTranslator& t) = 0; 
                virtual bool do_set_value(const CTranslator& t, const CComplexOptionParser::CParts& options) = 0; 
                virtual void do_write_value(std::ostream& os) const = 0;
                
                std::map<std::string, std::shared_ptr<CTranslator> > m_translators;
                
        }
                
private: 
        
}; 


class CTranslatorSelector {
public: 

        

}; 

class EXPORT_CORE CCmdParametrizedFunktorOption  {
 public: 
        CCmdParametrizedFunktorOption(CParamizedFunctor& vt, const char *init, char short_opt, const char *long_opt, 
                                      const char *long_help, const char *short_help, CCmdOptionFlags flags);

 private: 
        virtual bool do_set_value(const char *str_value);
	virtual void do_write_value(std::ostream& os) const;

        ValueTranslator& m_vt;
};


NS_MIA_END


#endif 
