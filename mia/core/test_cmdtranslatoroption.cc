/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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


#include <mia/internal/autotest.hh>
#include <mia/core/msgstream.hh>


#include <stdexcept>

class VectorInitializer: public CParamizedFunctor {
public: 
        VectorInitializer(const char*name): CValueTranslator::CTranslator(name){}
        
        virtual std::vector<double> prepare(const std::vector<double>& init){
                
        };
}

class Translator1 : public VectorInitializer {
public: 
        Translator1(); 
        
        virtual std::vector<double> prepare(const std::vector<double>& init) {
                return std::vector<double>(m_n); 
        }
        unsigned m_n; 
}; 

class Translator2 : public VectorInitializer {
public: 
        Translator2(); 
        
        virtual std::vector<double> prepare(const std::vector<double>& init) {
                return std::vector<double>(init);
        }
}; 

struct CmdTranslateFixture {
        CmdTranslateFixture(); 

        vector<double> m_init; 

}; 

BOOST_FIXTURE_TEST_CASE( test_option_fixedempty, CmdTranslateFixture )
{
        unique_ptr<VectorInitializer> initializer;
        CCmdTranslatorOption opt(initializer, nullptr, 'i', 'initializer', 
                                 "vector initializer", "vector initializer", CCmdOptionFlags::required);
        
        opt.set_value("fixedequal:n=3,init=10.0"); 
        
        assert(initializer); 
        
        auto initalized = initializer->run(m_init); 
        
        BOOST_CHECK_EQUAL(initalized.size(), 3ul); 
        BOOST_CHECK_EQUAL(initalized[0], 10.0); 
        BOOST_CHECK_EQUAL(initalized[1], 10.0); 
        BOOST_CHECK_EQUAL(initalized[2], 10.0); 

}

Translator1::Translator1(): VectorInitializer("fixedempty"), 
        m_n(2)
{
        add_parameter("n", make_positive_param(m_n, false, "set size of init vector"));
}

Translator2::Translator2(): VectorInitializer("copy"), 
{
}

CmdTranslateFixture::CmdTranslateFixture():m_init({1,2,3,4})
{
        auto& selector = VectorInitializer::selector_instance();
        selector.add_translator(new Translator1()); 
        selector.add_translator(new Translator2());
}
