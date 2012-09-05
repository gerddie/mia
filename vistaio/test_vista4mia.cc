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

#include <climits>
#define BOOST_TEST_DYN_LINK

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>

#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dimageiotest.hh>

#include <mia/2d/2dvfio.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>

#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>


#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dimageiotest.hh>

#include <mia/3d/3dvfio.hh>
#include <mia/3d/3dvfiotest.hh>

#include "vista4mia.hh"


NS_MIA_USE
using namespace std;
using namespace boost;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

CSplineBoundaryConditionTestPath sbc_test_path; 
CSplineKernelTestPath init_path; 

template <typename T>
void check_value(const CAttributedData& attr_map, const string& key,  T value)
{
	const PAttribute pattr = attr_map.get_attribute(key);
	cvdebug() << "check_value(" << key << ") = '" << value << "'\n";
	const TAttribute<T> * attr = dynamic_cast<const TAttribute<T> *>(pattr.get());
	BOOST_REQUIRE(attr);
	T v = *attr;
	BOOST_CHECK(v == value);
}

template <typename T>
void check_vattr_value(VAttrList list, const string& key,  T value)
{
	T lvalue;
	BOOST_REQUIRE(VGetAttr (list, key.c_str(), NULL, (VRepnKind)vista_repnkind<T>::value, &lvalue) == VAttrFound);
	cvdebug() << "check_vattr_value: " << value << " vs. " << lvalue <<"\n";
	BOOST_CHECK(lvalue == value);
}

template <>
void check_vattr_value(VAttrList list, const string& key,  bool value)
{
	VBit lvalue;
	BOOST_REQUIRE(VGetAttr (list, key.c_str(), NULL, VBitRepn, &lvalue) == VAttrFound);
	cvdebug() << "check_vattr_value: " << value << " vs. " << lvalue <<"\n";
	bool llvalue = lvalue;
	BOOST_CHECK(llvalue == value);
}


template <>
void check_vattr_value(VAttrList list, const string& key,  const string value)
{
	VString lvalue;
	BOOST_REQUIRE(VGetAttr (list, key.c_str(), NULL, (VRepnKind)vista_repnkind<string>::value, &lvalue) == VAttrFound);
	BOOST_CHECK(string(lvalue) == value);
}

void check_translation()
{
	VAttrList vista_list1 = VCreateAttrList();

	bool bit_value = 1;
	unsigned char ubyte_value = 124;
	signed char  sbyte_value = -120;
	short short_value = -12120;
	int int_value = -12120871;
	float float_value = 1.3f;
	double double_value = 1.7;
	string string_value("a string");

	CDoubleTranslator::register_for("double");
	CFloatTranslator::register_for("float");
	CUBTranslator::register_for("ubyte");
	CSBTranslator::register_for("sbyte");
	CSSTranslator::register_for("short");
	CSITranslator::register_for("int");
	CBitTranslator::register_for("bit");


	VSetAttr(vista_list1, "bit", NULL, VBitRepn, bit_value);
	VSetAttr(vista_list1, "ubyte", NULL, VUByteRepn, ubyte_value);
	VSetAttr(vista_list1, "sbyte", NULL, VSByteRepn, sbyte_value);
        VSetAttr(vista_list1, "short", NULL, VShortRepn, short_value);
        VSetAttr(vista_list1, "int", NULL, VLongRepn,  int_value);
        VSetAttr(vista_list1, "float", NULL, VFloatRepn, float_value);
        VSetAttr(vista_list1, "double", NULL, VDoubleRepn, double_value);
        VSetAttr(vista_list1, "string", NULL, VStringRepn, string_value.c_str());

	CAttributedData attr_map;

	copy_attr_list(attr_map, vista_list1);

        check_value(attr_map, "short",  short_value);
        check_value(attr_map, "int",   int_value);
        check_value(attr_map, "float",  float_value);
        check_value(attr_map, "double", double_value);
        check_value(attr_map, "string", string_value);
	check_value(attr_map, "bit", bit_value);
	check_value(attr_map, "ubyte",  ubyte_value);
	check_value(attr_map, "sbyte",  sbyte_value);


	VAttrList vista_list2 = VCreateAttrList();
	copy_attr_list(vista_list2, attr_map);


        check_vattr_value(vista_list2,  "short",  short_value);
        check_vattr_value(vista_list2,  "int",    int_value);
        check_vattr_value(vista_list2,  "float",  float_value);
        check_vattr_value(vista_list2,  "string", string_value);
	check_vattr_value(vista_list2,  "bit",    bit_value);
	check_vattr_value(vista_list2,  "ubyte",  ubyte_value);
	check_vattr_value(vista_list2,  "sbyte",  sbyte_value);
        check_vattr_value(vista_list2,  "double", double_value);

	VDestroyAttrList(vista_list1);
	VDestroyAttrList(vista_list2);
}

static void handler_setup()
{
	CPathNameArray searchpath;
	searchpath.push_back(bfs::path("."));

	C2DImageIOPluginHandler::set_search_path(searchpath);
	C3DImageIOPluginHandler::set_search_path(searchpath);

	C2DVFIOPluginHandler::set_search_path(searchpath);
	C3DVFIOPluginHandler::set_search_path(searchpath);

	C2DTransformationIOPluginHandler::set_search_path(searchpath);
	C3DTransformationIOPluginHandler::set_search_path(searchpath);

	CPathNameArray tc2_searchpath; 
	tc2_searchpath.push_back(bfs::path("../mia/2d/transform"));

	CPathNameArray tc3_searchpath; 
	tc3_searchpath.push_back(bfs::path("../mia/3d/transform"));

	C2DTransformCreatorHandler::set_search_path(tc2_searchpath);
	C3DTransformCreatorHandler::set_search_path(tc3_searchpath);
}



static void test_3dimage_plugin_handler()
{
	const C3DImageIOPluginHandler::Instance& handler = C3DImageIOPluginHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 2u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(),  "datapool vista ");
}



static void test_2dimage_plugin_handler()
{
	const C2DImageIOPluginHandler::Instance& handler = C2DImageIOPluginHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 2u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "datapool vista ");
}

static void test_3dvf_plugin_handler()
{
	const C3DVFIOPluginHandler::Instance& handler = C3DVFIOPluginHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 3u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "cvista datapool vista ");
}


static void test_2dvf_plugin_handler()
{
	const C2DVFIOPluginHandler::Instance& handler = C2DVFIOPluginHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 2u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "datapool vista ");
}


static void test_2dtransform_plugin_handler()
{
	const C2DTransformationIOPluginHandler::Instance& handler = C2DTransformationIOPluginHandler::instance();
	BOOST_CHECK_EQUAL(handler.size(), 2u);
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "datapool vista ");
}

static void test_2dtransform_io()
{
	C2DBounds size( 20, 20); 
	
	const char *transforms[] = {
		"translate",
		"affine", 
		"rigid", 
		"spline:rate=3", 
		"spline:rate=6", 
		"vf"
	}; 
	const size_t n_transforms = sizeof(transforms)/sizeof(char*); 


	for (size_t t = 0; t < n_transforms; ++t) {
		auto tc = C2DTransformCreatorHandler::instance().produce(transforms[t]); 
		auto tr = tc->create(size); 
		auto params = tr->get_parameters(); 
		for (size_t k = 0; k < params.size(); ++k) 
			params[k] = k + 1; 
		tr->set_parameters(params);
		
		stringstream fname; 
		fname << transforms[t] << "." << "v2dt"; 
		cvdebug() << "store to '" << fname.str() << "'\n"; 
		BOOST_CHECK(C2DTransformationIOPluginHandler::instance().save(fname.str(), *tr)); 
		
		auto t_loaded = C2DTransformationIOPluginHandler::instance().load(fname.str()); 
		BOOST_CHECK(t_loaded); 
		BOOST_CHECK_EQUAL(t_loaded->get_size(), size); 
		BOOST_CHECK_EQUAL(t_loaded->get_creator_string(), transforms[t]);
		
		auto lparams = t_loaded->get_parameters();
		BOOST_CHECK_EQUAL(lparams.size(), params.size()); 
		for (size_t k = 0; k < lparams.size(); ++k) 
			BOOST_CHECK_EQUAL(lparams[k], k + 1); 
		unlink( fname.str().c_str()); 
		
	}

}


static void test_3dtransform_io()
{
	C3DBounds size( 20, 20, 10); 
	
	const char *transforms[] = {
		"translate",
		"rigid",
		"affine", 
		"spline:rate=3", 
		"spline:rate=6", 
#if 0  
		"vf"
#endif 
	}; 
	const size_t n_transforms = sizeof(transforms)/sizeof(char*); 


	for (size_t t = 0; t < n_transforms; ++t) {
		cvdebug() << "Try to create " << transforms[t] << "\n"; 
		auto tc = C3DTransformCreatorHandler::instance().produce(transforms[t]); 
		auto tr = tc->create(size); 
		auto params = tr->get_parameters(); 
		for (size_t k = 0; k < params.size(); ++k) 
			params[k] = k + 1; 
		tr->set_parameters(params);
		
		stringstream fname; 
		fname << transforms[t] << "." << "v3dt"; 
		cvdebug() << "store to '" << fname.str() << "'\n"; 
		BOOST_CHECK(C3DTransformationIOPluginHandler::instance().save(fname.str(), *tr)); 
		
		auto t_loaded = C3DTransformationIOPluginHandler::instance().load(fname.str()); 
		BOOST_CHECK(t_loaded); 
		BOOST_CHECK_EQUAL(t_loaded->get_size(), size); 
		BOOST_CHECK_EQUAL(t_loaded->get_creator_string(), transforms[t]);
		
		auto lparams = t_loaded->get_parameters();
		BOOST_CHECK_EQUAL(lparams.size(), params.size()); 
		for (size_t k = 0; k < lparams.size(); ++k) 
			BOOST_CHECK_EQUAL(lparams[k], k + 1); 
		unlink( fname.str().c_str()); 
	}
}


bool init_unit_test_suite( )
{

	handler_setup();

	test_suite *suite = &framework::master_test_suite();

	suite->add( BOOST_TEST_CASE( &check_translation));
	suite->add( BOOST_TEST_CASE( &test_2dimage_plugin_handler));
	suite->add( BOOST_TEST_CASE( &test_2dimageio_plugins));

	suite->add( BOOST_TEST_CASE( &test_3dimage_plugin_handler));
	add_3dimageio_plugin_tests( suite );

	suite->add( BOOST_TEST_CASE( &test_3dvf_plugin_handler ));
	suite->add( BOOST_TEST_CASE( &test_2dvf_plugin_handler ));

	suite->add( BOOST_TEST_CASE( &test_2dtransform_plugin_handler ));
	suite->add( BOOST_TEST_CASE( &test_2dtransform_io ));
	suite->add( BOOST_TEST_CASE( &test_3dtransform_io ));

	add_2dvfio_tests( suite );

	return true;
}

const SProgramDescription description = {
	"Tests", 
	"Vista tests", 
	"This program runs a set of tests.", 
	NULL, 
	NULL
};

int BOOST_TEST_CALL_DECL
main( int argc, char* argv[] )
{
	if (CCmdOptionList(description).parse(argc, argv) != CCmdOptionList::hr_no)
		return 0; 
	return ::boost::unit_test::unit_test_main( &init_unit_test_suite, argc, argv );
}
