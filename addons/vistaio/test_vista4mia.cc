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

#include <climits>

#include <mia/internal/autotest.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/imageiotest.hh>

#include <mia/2d/vfio.hh>
#include <mia/2d/transformio.hh>
#include <mia/2d/transformfactory.hh>

#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>


#include <mia/3d/imageio.hh>
#include <mia/3d/imageiotest.hh>

#include <mia/3d/vfio.hh>
#include <mia/3d/vfiotest.hh>

#include "vista4mia.hh"


NS_MIA_USE
using namespace std;
using namespace boost;
using namespace boost::unit_test;
namespace bfs = ::boost::filesystem;

template <typename T>
void check_value(const CAttributedData& attr_map, const string& key,  T value)
{
       const PAttribute pattr = attr_map.get_attribute(key);
       cvdebug() << "check_value(" << key << ") = '" << value << "' of type " << typeid(T).name() << "\n";
       const TAttribute<T> *attr = dynamic_cast<const TAttribute<T> *>(pattr.get());
       BOOST_REQUIRE(attr);
       T v = *attr;
       BOOST_CHECK(v == value);
}

template <typename T>
void check_vattr_value(VistaIOAttrList list, const string& key,  T value)
{
       T lvalue;
       BOOST_REQUIRE(VistaIOGetAttr (list, key.c_str(), NULL, (VistaIORepnKind)vista_repnkind<T>::value, &lvalue) == VistaIOAttrFound);
       cvdebug() << "check_vattr_value: " << value << " vs. " << lvalue << "\n";
       BOOST_CHECK(lvalue == value);
}

template <>
void check_vattr_value(VistaIOAttrList list, const string& key,  bool value)
{
       VistaIOBit lvalue;
       BOOST_REQUIRE(VistaIOGetAttr (list, key.c_str(), NULL, VistaIOBitRepn, &lvalue) == VistaIOAttrFound);
       cvdebug() << "check_vattr_value: " << value << " vs. " << lvalue << "\n";
       bool llvalue = lvalue;
       BOOST_CHECK(llvalue == value);
}


template <>
void check_vattr_value(VistaIOAttrList list, const string& key,  const string value)
{
       VistaIOString lvalue;
       BOOST_REQUIRE(VistaIOGetAttr (list, key.c_str(), NULL, (VistaIORepnKind)vista_repnkind<string>::value, &lvalue) == VistaIOAttrFound);
       BOOST_CHECK(string(lvalue) == value);
}

BOOST_AUTO_TEST_CASE( check_translation)
{
       VistaIOAttrList vista_list1 = VistaIOCreateAttrList();
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
       VistaIOSetAttr(vista_list1, "bit", NULL, VistaIOBitRepn, bit_value);
       VistaIOSetAttr(vista_list1, "ubyte", NULL, VistaIOUByteRepn, ubyte_value);
       VistaIOSetAttr(vista_list1, "sbyte", NULL, VistaIOSByteRepn, sbyte_value);
       VistaIOSetAttr(vista_list1, "short", NULL, VistaIOShortRepn, short_value);
       VistaIOSetAttr(vista_list1, "int", NULL, VistaIOLongRepn,  int_value);
       VistaIOSetAttr(vista_list1, "float", NULL, VistaIOFloatRepn, float_value);
       VistaIOSetAttr(vista_list1, "double", NULL, VistaIODoubleRepn, double_value);
       VistaIOSetAttr(vista_list1, "string", NULL, VistaIOStringRepn, string_value.c_str());
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
       VistaIOAttrList vista_list2 = VistaIOCreateAttrList();
       copy_attr_list(vista_list2, attr_map);
       check_vattr_value(vista_list2,  "short",  short_value);
       check_vattr_value(vista_list2,  "int",    int_value);
       check_vattr_value(vista_list2,  "float",  float_value);
       check_vattr_value(vista_list2,  "string", string_value);
       check_vattr_value(vista_list2,  "bit",    bit_value);
       check_vattr_value(vista_list2,  "ubyte",  ubyte_value);
       check_vattr_value(vista_list2,  "sbyte",  sbyte_value);
       check_vattr_value(vista_list2,  "double", double_value);
       VistaIODestroyAttrList(vista_list1);
       VistaIODestroyAttrList(vista_list2);
}

BOOST_AUTO_TEST_CASE(test_2dtransform_io)
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
       const size_t n_transforms = sizeof(transforms) / sizeof(char *);

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


BOOST_AUTO_TEST_CASE(test_3dtransform_io)
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
       const size_t n_transforms = sizeof(transforms) / sizeof(char *);

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


