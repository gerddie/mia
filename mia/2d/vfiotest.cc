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

#include <cstdlib>
#include <climits>

#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <mia/core/msgstream.hh>
#include <mia/core/history.hh>
#include <mia/2d/vfio.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN
using namespace std;
using namespace boost::unit_test;
namespace bfs =::boost::filesystem;

const char revision[] = "FakeRevision";


const	C2DFVector pixel(1.1, 2.2);

static void check_save_load(const C2DIOVectorfield& vf, const C2DVFIOPluginHandler::Instance::Interface& vfio)
{
       const string format = vfio.get_name();
       const string tmp_name = string("vf2dtest") + string(".") + format;
       BOOST_REQUIRE(vfio.save(tmp_name.c_str(), vf));
       cvdebug() << format << "saved" << std::endl;
       std::shared_ptr<C2DIOVectorfield > reread(vfio.load(tmp_name.c_str()));
       BOOST_REQUIRE(reread.get());
       cvdebug() << vf.size() << " vs. " << reread->size() << "\n";
       BOOST_REQUIRE(vf.size() == reread->size());
       BOOST_REQUIRE(vf.get_size() == reread->get_size());

       //BOOST_CHECK_EQUAL(pixel, reread->get_pixel_size());

       for (C2DIOVectorfield::const_iterator iv = vf.begin(), ir = reread->begin();
            iv != vf.end();  ++iv, ++ir) {
              cvdebug() << *iv << " vs. " << *ir << "\n";
              BOOST_CHECK(*iv == *ir);
       }

       unlink(tmp_name.c_str());
       cvdebug() << tmp_name << " unlinked\n";
}

static void test_2dvfio_plugins(const C2DVFIOPluginHandler::value_type& p)
{
       C2DIOVectorfield vf(C2DBounds(5, 6));

       for (C2DIOVectorfield::iterator i = vf.begin(); i != vf.end(); ++i) {
#ifdef WIN32
              *i = C2DFVector((float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
#else
              *i = C2DFVector(drand48(), drand48());
#endif
       }

       //	vf.set_pixel_size(pixel);
       check_save_load(vf, *p.second);
}

void EXPORT_2DTEST add_2dvfio_tests(test_suite *suite)
{
       C2DVFIOPluginHandler::const_iterator i = C2DVFIOPluginHandler::instance().begin();
       C2DVFIOPluginHandler::const_iterator e = C2DVFIOPluginHandler::instance().end();

       if (i != e)
              suite->add( BOOST_PARAM_TEST_CASE(&test_2dvfio_plugins, i, e ));
}
NS_MIA_END
