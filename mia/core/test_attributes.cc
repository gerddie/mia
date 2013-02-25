/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define BOOST_TEST_DYN_LINK

#include <climits>
#include <stdexcept>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>


#include <mia/core/attributes.hh>
#include <mia/core/msgstream.hh>

#include <boost/mpl/vector.hpp>
//#include <boost/test/unit_test_suite.hpp>
//#include <boost/test/test_case_template.hpp>
//#include <boost/test/test_tools.hpp>

NS_MIA_USE
using namespace std;
using namespace boost::unit_test;
namespace bmpl=boost::mpl;

template <typename T>
ostream& operator << (ostream& os, const vector<T>& v)
{
	os << "vector(" <<v.size() << ") = [";
	copy(v.begin(), v.end(), ostream_iterator<T>(os, ", "));
	os << "]";
	return os;
}


template <typename T>
void check_translate_type(const string& key, T value, string svalue)
{
	cvdebug() << "check_translate_type: key=" << key << ", value="<< value << ", svalue='" << svalue<<endl;

	PAttribute attr =  CStringAttrTranslatorMap::instance().to_attr(key, svalue);

	BOOST_REQUIRE(attr);

	const TAttribute<T> *test_value = dynamic_cast<const TAttribute<T> *>(attr.get());
	if (!test_value)  {
		cverr() << "returned type is " << typeid(*attr.get()).name() << "\n";
		cverr() << "expected type is " << typeid(T).name() << "\n";
		return;
	}

	BOOST_REQUIRE(test_value);

	T v = *test_value;
	BOOST_CHECK_EQUAL(v, value);

	BOOST_CHECK_EQUAL(attr->as_string(), svalue);
}

template <typename T>
void check_translate_type(const string& key, const vector<T>& value, string svalue)
{
	//cvdebug() << "check_translate_type: key=" << key << ", value="<< value << ", svalue='" << svalue<<endl;

	PAttribute attr =  CStringAttrTranslatorMap::instance().to_attr(key, svalue);
	BOOST_REQUIRE(attr);

	const TAttribute<vector<T> > *test_value = dynamic_cast<const TAttribute<vector<T> > *>(attr.get());
	if (!test_value)  {
		cverr() << "returned type is " << typeid(*attr.get()).name() << "\n";
		cverr() << "expected type is " << typeid(const TAttribute<vector<T> > *).name() << "\n";
		return;
	}

	BOOST_REQUIRE(test_value);

	vector<T> v = *test_value;
	BOOST_CHECK_EQUAL(v.size(), value.size());
	BOOST_REQUIRE(v.size() == value.size());
	BOOST_CHECK(equal(v.begin(), v.end(), value.begin()));

	BOOST_CHECK_EQUAL(attr->as_string(), svalue);
}


BOOST_AUTO_TEST_CASE( test_translator )
{
	CUSTranslator::register_for("ushort");
	CSSTranslator::register_for("sshort");
	CUBTranslator::register_for("ubyte");
	CSBTranslator::register_for("sbyte");

	CDoubleTranslator::register_for("double");
	CFloatTranslator::register_for("float");

	CBitTranslator::register_for("bit");
	
	// registering the key to the same result type is allowed
	BOOST_CHECK_NO_THROW(CBitTranslator::register_for("bit")); 

	// registering the key to a different result type will throw 
	BOOST_CHECK_THROW(CUSTranslator::register_for("bit"), invalid_argument); 


	check_translate_type("bit", true, "1");
	check_translate_type("double", 1.9, "1.9");
	check_translate_type("float", 1.7f, "1.7");
	check_translate_type("ubyte", (unsigned char)129, "129");
	check_translate_type("sbyte", (signed char)-120, "-120");
	check_translate_type("sshort", (signed short)-1231, "-1231");
	check_translate_type("ushort", (unsigned short)3213, "3213");
#ifdef LONG_64BIT
	CULTranslator::register_for("ulong");
	CSLTranslator::register_for("slong");
	check_translate_type("slong", (signed long)-1212321, "-1212321");
	check_translate_type("ulong", (unsigned long)1238763, "1238763");
#endif

}

BOOST_AUTO_TEST_CASE( test_vector_translator )
{
	CVUSTranslator::register_for("vushort");
	vector<unsigned short> values(3);
	values[0] = 0;
	values[1] = 1;
	values[2] = 3;
	check_translate_type("vushort", values, "3 0 1 3");
}

template <typename T>
void test_type_attribute()
{
	CAttributeMap attr_list;
	attr_list["zero"] = PAttribute(new TAttribute<T>(0));
	attr_list["one"] = PAttribute(new TAttribute<T>(1));

	TAttribute<T> *attr = dynamic_cast<TAttribute<T>*>(attr_list["zero"].get());
	BOOST_REQUIRE(attr);
	BOOST_REQUIRE(*attr == 0);

	attr = dynamic_cast<TAttribute<T>*>(attr_list["one"].get());
	BOOST_REQUIRE(attr);
	BOOST_REQUIRE(*attr == 1);
}



typedef bmpl::vector<bool,
		     signed char,
		     unsigned char,
		     signed short,
		     unsigned short,
		     signed int,
		     unsigned int,
#ifdef LONG_64BIT
		     signed long,
		     unsigned long,
#endif
		     float,
		     double
		     > test_types;

BOOST_AUTO_TEST_CASE_TEMPLATE( test_type_attributes, T , test_types )
{
	test_type_attribute<T>();
}

BOOST_AUTO_TEST_CASE( test_attributes )
{


	CAttributeMap attr_list;

	string test_string(" test string");

	attr_list["int_attr"] = PAttribute(new CIntAttribute(1));
	attr_list["float_attr"] = PAttribute(new CFloatAttribute(1.0f));
	attr_list["string_attr"] = PAttribute(new CStringAttribute(test_string));

	CIntAttribute *int_attr = dynamic_cast<CIntAttribute*>(attr_list["int_attr"].get());
	BOOST_CHECK(int_attr);
	BOOST_CHECK(*int_attr == 1);


	CFloatAttribute *float_attr = dynamic_cast<CFloatAttribute*>(attr_list["float_attr"].get());
	BOOST_CHECK(float_attr);
	BOOST_CHECK(*float_attr == 1.0f);

	CStringAttribute *string_attr = dynamic_cast<CStringAttribute*>(attr_list["string_attr"].get());
	BOOST_CHECK(string_attr);
	string s =  *string_attr;
	BOOST_CHECK(test_string ==  s);

	CAttributeMap sub_list;
	sub_list["val1"] = PAttribute(new CIntAttribute(1));

	attr_list["sublist_attr"] = PAttribute(new CAttributeList(sub_list));


	CAttributedData data1(PAttributeMap(new CAttributeMap(attr_list)));
	int_attr = dynamic_cast<CIntAttribute*>(data1.get_attribute("int_attr").get());
	BOOST_CHECK(int_attr);
	BOOST_CHECK(*int_attr == 1);

	CAttributedData data2(data1);
	int_attr = dynamic_cast<CIntAttribute*>(data2.get_attribute("int_attr").get());
	BOOST_CHECK(int_attr);
	BOOST_CHECK(*int_attr == 1);

	int kk = get_attribute_as<int>(*data2.get_attribute("int_attr"));
	BOOST_CHECK_EQUAL(kk, 1);

	data2.set_attribute("int_attr", PAttribute(new CIntAttribute(2)));
	int_attr = dynamic_cast<CIntAttribute*>(data2.get_attribute("int_attr").get());
	BOOST_CHECK(int_attr);
	BOOST_CHECK(*int_attr == 2);

	int_attr = dynamic_cast<CIntAttribute*>(data1.get_attribute("int_attr").get());
	BOOST_CHECK(int_attr);
	BOOST_CHECK(*int_attr == 1);

	CAttributedData data3;
	BOOST_CHECK(!data3.get_attribute("int_attr"));
}


void  add_an_attribute(CAttributedData& data, const string& key, const string& value)
{
	data.set_attribute(key, CStringAttrTranslatorMap::instance().to_attr(key, value));
}

BOOST_AUTO_TEST_CASE( test_attributes_equal )
{
	PAttribute a(new TAttribute<float>(1.3f));
	PAttribute c(new TAttribute<float>(1.4f));
	PAttribute b(new TAttribute<double>(1.2f));

	BOOST_CHECK(a->is_equal(*a));
	BOOST_CHECK(!a->is_equal(*b));
	BOOST_CHECK(!a->is_equal(*c));
}

BOOST_AUTO_TEST_CASE( test_lists_equal )
{
	CAttributedData data1;

	CBitTranslator::register_for("bit0");
	CBitTranslator::register_for("bit1");

	CDoubleTranslator::register_for("double0");
	CDoubleTranslator::register_for("double1");

	add_an_attribute(data1, "bit0", "1");
	add_an_attribute(data1, "bit1", "0");

	add_an_attribute(data1, "double0", "1.12");
	add_an_attribute(data1, "double1", "1.43");

	add_an_attribute(data1, "string", "some string");


	CAttributedData data2(data1);

	BOOST_REQUIRE(data2 == data1);

	add_an_attribute(data1, "double0", "2.0");
	BOOST_CHECK(!(data2 == data1));

	data2 = data1;
	BOOST_REQUIRE(data2 == data1);

	add_an_attribute(data1, "funny", "funny");
	BOOST_CHECK(!(data2 == data1));

	data2 = data1;

	data1.set_attribute("type", PAttribute(new TAttribute<float>(1.5f)));
	data1.set_attribute("type", PAttribute(new TAttribute<double>(1.5f)));
	BOOST_CHECK(!(data2 == data1));

}


template <typename T>
void check_add_and_read(CAttributeMap& map, const char *name, T value)
{
	typedef const TAttribute<T> * PAttr;
	add_attribute(map, name, value);

	PAttr attr = dynamic_cast<PAttr>(map[name].get());
	BOOST_REQUIRE(attr);
	T test_value = *attr;
	BOOST_CHECK_EQUAL(value, test_value);
}

template <>
void check_add_and_read(CAttributeMap& map, const char *name, const char *value)
{
	typedef const TAttribute<string>& PAttr;
	add_attribute(map, name, value);

	cvdebug() << typeid(PAttr).name() << " vs " << typeid(*map[name].get()).name() << "\n"; 
	PAttr attr = dynamic_cast<PAttr>(*map[name].get());
	string test_value = attr;
	BOOST_CHECK_EQUAL(string(value), test_value);
}

BOOST_AUTO_TEST_CASE( test_add_attribute_int )
{
	CAttributeMap map;
	check_add_and_read(map, "int", 10);
}

BOOST_AUTO_TEST_CASE( test_add_attribute_short )
{
	CAttributeMap map;
	check_add_and_read(map, "short", (short)20);
}
BOOST_AUTO_TEST_CASE( test_add_attribute_cstring )
{
	CAttributeMap map;
	check_add_and_read(map, "char*", "char*_value");
}
BOOST_AUTO_TEST_CASE( test_add_attribute_string )
{
	CAttributeMap map;
	check_add_and_read(map, "string", string("string_value"));
}
BOOST_AUTO_TEST_CASE( test_add_attribute_float )
{
	CAttributeMap map;
	check_add_and_read(map, "float", 1.0f);
}

BOOST_AUTO_TEST_CASE( test_add_attribute )
{
	CAttributeMap map;
	check_add_and_read(map, "double", 1.0);

}

BOOST_AUTO_TEST_CASE( test_attribute_data )
{
	CAttributedData data;

	BOOST_CHECK(!data.has_attribute("some"));

	data.set_attribute( "some", PAttribute(new TAttribute<float>(1.5f)));

	BOOST_CHECK(data.has_attribute("some"));

	BOOST_CHECK_EQUAL(data.get_attribute_as_string("some"), "1.5");

	CFloatTranslator::register_for("floatfromstring");
	data.set_attribute( "floatfromstring", "2.5f");

	PAttribute floatfromstring = data.get_attribute("floatfromstring");

	BOOST_REQUIRE(floatfromstring);

	const TAttribute<float> *test_value = dynamic_cast<const TAttribute<float> *>(floatfromstring.get());
	BOOST_REQUIRE(test_value);
	float tv = *test_value;

	BOOST_CHECK_EQUAL(2.5f, tv);
	BOOST_CHECK_EQUAL(2.5f, data.get_attribute_as<float>("floatfromstring"));

	data.delete_attribute("floatfromstring");
	BOOST_CHECK(!data.has_attribute("floatfromstring"));
}

bool operator==(PAttribute const & a, PAttribute const & b)
{
	return *a == *b;
}

BOOST_AUTO_TEST_CASE( test_attribute_compare )
{
	PAttribute t1(new TAttribute<string>("testwert1"));
	PAttribute t2(new TAttribute<string>("testwert2"));
	PAttribute t3(new TAttribute<int>(1));
	PAttribute t4(new TAttribute<int>(2));

	BOOST_CHECK(*t1 < *t2);
	BOOST_CHECK(*t2 < *t3);
	BOOST_CHECK(*t3 < *t4);

	BOOST_CHECK(!(*t4 < *t4));
}

BOOST_AUTO_TEST_CASE( test_attribute_as_map_key )
{
	map<PAttribute, string, pattr_less> testmap;

	PAttribute t1(new CStringAttribute("testwert1"));
	PAttribute t2(new CStringAttribute("testwert2"));
	PAttribute t3(new CIntAttribute(1));
	PAttribute t4(new CIntAttribute(1));

	testmap[t1] = "stringvalue1";
	testmap[t2] = "stringvalue2";
	testmap[t3] = "intvalue1";

	BOOST_CHECK_EQUAL(testmap[t3], "intvalue1");

	testmap[t4] = "intvalue2";

	BOOST_CHECK_EQUAL(testmap[t4], "intvalue2");
	BOOST_CHECK_EQUAL(testmap[t3], "intvalue2");


}

BOOST_AUTO_TEST_CASE( test_get_attribute_as_non_existent )
{
	CAttributedData data;
	BOOST_CHECK_THROW(data.get_attribute_as<int>("dummy"), invalid_argument);
}

BOOST_AUTO_TEST_CASE( test_get_attribute_as_wrong_type )
{
	CAttributedData data;
	data.set_attribute("dummy", PAttribute(new CFloatAttribute(1.0f)));
	BOOST_CHECK_THROW(data.get_attribute_as<int>("dummy"), bad_cast);
}


BOOST_AUTO_TEST_CASE( test_attribute_map_equal_test )
{
	CAttributeMap map1;
	map1["dummy"] = PAttribute(new CFloatAttribute(1.0f));
	CAttributeMap map2;
	map2["dummy"] = PAttribute(new CFloatAttribute(1.0f));
	
	BOOST_CHECK(map1 == map2); 

	map1["bla"] = PAttribute(new CFloatAttribute(2.0f));
	map2["ble"] = PAttribute(new CFloatAttribute(2.0f));
	BOOST_CHECK(!(map1 == map2)); 
}

BOOST_AUTO_TEST_CASE( test_attribute_map_get_attribute_as_string_not_exist )
{
	CAttributedData data;
	auto s = data.get_attribute_as_string("test"); 
	BOOST_CHECK_EQUAL(s.size(), 0u); 
		
}

BOOST_AUTO_TEST_CASE( Ensure_some_copy_on_write )
{
	CAttributedData data;
	data.set_attribute("dummy", PAttribute(new CFloatAttribute(2.0f))); 
	
	CAttributedData data2(data); 

	data.delete_attribute("dummy"); 
	BOOST_CHECK(!data.has_attribute("dummy")); 
	BOOST_CHECK(data2.has_attribute("dummy")); 
}

BOOST_AUTO_TEST_CASE( test_set_some_attribues )
{
	CAttributedData data;
	data.set_attribute("dummy", PAttribute(new CFloatAttribute(2.0f))); 
	data.set_attribute("dummy2", PAttribute(new CIntAttribute(3))); 
	
	CAttributedData data2; 
	
	data2.set_attributes(data.begin_attributes(), data.end_attributes()); 

	BOOST_CHECK(data2.has_attribute("dummy")); 
	BOOST_CHECK(data2.has_attribute("dummy2"));
}



