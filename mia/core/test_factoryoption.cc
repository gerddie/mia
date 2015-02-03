/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/core/product_base.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/core/handler.hh>
#include <mia/core/factory.hh>
#include <mia/core/handler.cxx>
#include <mia/core/plugin_base.cxx>


using namespace std; 
NS_MIA_BEGIN

class CProductMock : public CProductBase {
public:
	typedef CProductMock plugin_type; 
	typedef CProductMock plugin_data; 

	static const char *data_descr; 
	static const char *type_descr; 
	CProductMock(const char *s):m_value(s){
	}; 
private: 
	string m_value;
}; 

const char *CProductMock::type_descr ="mock";  
const char *CProductMock::data_descr ="product";  

class CFactoryMock: public TFactory<CProductMock>  {
public: 
	CFactoryMock():TFactory<CProductMock>("lala"){}
		
	typedef shared_ptr<CProductMock> ProductPtr; 
	
	CProductMock *do_create() const{
		return new CProductMock("teststring"); 
	}
	const std::string do_get_descr() const{return "test mock";}
}; 

class CFactoryMockPluginHandler : public TFactoryPluginHandler<CFactoryMock> {
public: 
	CFactoryMockPluginHandler()
	{
		add_plugin(TFactoryPluginHandler<CFactoryMock>::PInterface(new CFactoryMock())); 
	}
}; 

template<> const  char * const 
TPluginHandler<CFactoryMock>::m_help = "just a mock"; 

typedef THandlerSingleton<CFactoryMockPluginHandler> CFactoryHandlerMock; 

FACTORY_TRAIT(CFactoryHandlerMock); 

BOOST_AUTO_TEST_CASE( test_a_factory_option )
{

	CFactoryHandlerMock::ProductPtr product; 

	
	PCmdOption option = make_opt(product, "lola", "lala", 'l', "Some help"); 
	
	option->set_value("lala"); 
	option->post_set(); 
	
	BOOST_REQUIRE(product); 
	BOOST_CHECK_EQUAL(product->get_init_string(), "lala"); 
	
	//option->set_value(NULL); 
	//	BOOST_CHECK(!product); 

	BOOST_CHECK_EQUAL(option->get_value_as_string(), "lala"); 
}

const SProgramDescription general_help = {
	{pdi_group,"Test"}, 
	{pdi_short,"Test factory option"}, 
	{pdi_description,"This program tests the command line parser."}
}; 


BOOST_AUTO_TEST_CASE( test_a2_factory_option )
{

	CFactoryHandlerMock::ProductPtr product; 
	PCmdOption option = make_opt(product, "lala", "lila", 'l',"Some help"); 
	
	CCmdOptionList olist(general_help); 
	olist.add(option); 
	
	vector<const char *> cmdline(1); 
	cmdline[0] = "testprogram"; 
	BOOST_CHECK_EQUAL(olist.parse(cmdline.size(), &cmdline[0]), CCmdOptionList::hr_no);
	BOOST_REQUIRE(product); 
	
	BOOST_CHECK_EQUAL(product->get_init_string(), "lala"); 
	
	//option->set_value(NULL); 
	//	BOOST_CHECK(!product); 

	BOOST_CHECK_EQUAL(option->get_value_as_string(), "lala"); 
}

BOOST_AUTO_TEST_CASE( test_string_factory_hint_check_valide)
{
        string value;
        PCmdOption popt(make_opt(value, "string", 's', "a string option",
                                 CCmdOptionFlags::validate, &CFactoryHandlerMock::instance() ));


        BOOST_CHECK_NO_THROW(popt->set_value("lala:stuff=no"));

}

BOOST_AUTO_TEST_CASE( test_string_factory_hint_check_fail)
{
        string value;
        PCmdOption popt(make_opt(value, "string", 's', "a string option",
                                 CCmdOptionFlags::validate, &CFactoryHandlerMock::instance() ));


        BOOST_CHECK_THROW(popt->set_value("lalo:stuff=no"), invalid_argument);
}


BOOST_AUTO_TEST_CASE( test_a2_factory_option_unique )
{

	CFactoryHandlerMock::UniqueProduct product; 
	PCmdOption option = make_opt(product, "lala", "lila", 'l',"Some help"); 
	
	CCmdOptionList olist(general_help); 
	olist.add(option); 
	
	vector<const char *> cmdline(1); 
	cmdline[0] = "testprogram"; 
	BOOST_CHECK_EQUAL(olist.parse(cmdline.size(), &cmdline[0]), CCmdOptionList::hr_no);
	BOOST_REQUIRE(product); 
	
	BOOST_CHECK_EQUAL(product->get_init_string(), "lala"); 
	
	//option->set_value(NULL); 
	//	BOOST_CHECK(!product); 

	BOOST_CHECK_EQUAL(option->get_value_as_string(), "lala"); 
}


BOOST_AUTO_TEST_CASE( test_another_factory_option )
{


	auto product = 	CFactoryHandlerMock::instance().produce("lala"); 
	BOOST_CHECK_EQUAL(product->get_init_string(), "lala"); 
	
	PCmdOption option = make_opt(product, "lola", "lala", 'l',"Some help"); 
	BOOST_CHECK_EQUAL(option->get_value_as_string(), "lala"); 
}

NS_MIA_END
