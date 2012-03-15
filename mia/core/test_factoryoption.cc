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
	CFactoryMockPluginHandler(const std::list<boost::filesystem::path>& searchpath):
		TFactoryPluginHandler<CFactoryMock>(searchpath)
	{
		add_plugin(new CFactoryMock()); 
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
	"Test", 
	"Short, descr", 
	"This program tests the command line parser.", 
	NULL, 
	NULL
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
