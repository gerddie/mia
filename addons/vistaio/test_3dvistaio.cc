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

#define VSTREAM_DOMAIN "vista3dImageIOtest"

#include <mia/internal/autotest.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>


#include <vistaio/3dvistaio.hh>
#include <unistd.h>

using namespace mia;
using namespace std;
using namespace vista_3d_io;
namespace bmpl = boost::mpl;

typedef bmpl::vector <
unsigned char,
         signed short,
         unsigned short,
         signed int,
         unsigned int,
         float,
         double
         > type;


BOOST_AUTO_TEST_CASE_TEMPLATE( test_simple_write_read, T, type )
{
       C3DBounds size(2, 3, 4);
       T3DImage<T> *image = new T3DImage<T>(size);
       image->set_voxel_size(C3DFVector(10, 20, 30));
       image->set_origin(C3DFVector(20, 30, 40));
       image->set_rotation(C3DRotation(Quaternion(0.5, 0.1, 0.7, 0.5)));
       P3DImage pimage(image);
       auto iv = image->begin();
       auto ev = image->end();
       int i = 0;

       while (iv != ev)
              *iv++ = i++;

       CVista3DImageIOPlugin io;
       CVista3DImageIOPlugin::Data images;
       images.push_back(pimage);
       stringstream filename;
       filename << "testimage-" << __type_descr<T>::value << ".vista3d";
       cvdebug() << "test with " << filename.str() << "\n";
       BOOST_REQUIRE(io.save(filename.str(), images));
       auto loaded = io.load(filename.str());
       BOOST_REQUIRE(loaded);
       BOOST_REQUIRE(loaded->size() == 1u);
       const auto& ploaded = dynamic_cast<const T3DImage<T>&>(*(*loaded)[0]);
       iv = image->begin();
       auto il = ploaded.begin();

       while (iv != ev) {
              BOOST_CHECK_EQUAL(*il, *iv);
              ++iv;
              ++il;
       }

       unlink(filename.str().c_str());
}

BOOST_AUTO_TEST_CASE( test_simple_write_read_bool )
{
       C3DBounds size(2, 3, 4);
       C3DBitImage *image = new C3DBitImage(size);
       image->set_voxel_size(C3DFVector(10, 20, 30));
       image->set_origin(C3DFVector(20, 30, 40));
       image->set_rotation(C3DRotation(Quaternion(0.5, 0.1, 0.7, 0.5)));
       P3DImage pimage(image);
       auto iv = image->begin();
       auto ev = image->end();
       bool i = false;

       while (iv != ev) {
              *iv++ = i;
              i = !i;
       }

       CVista3DImageIOPlugin io;
       CVista3DImageIOPlugin::Data images;
       images.push_back(pimage);
       stringstream filename;
       filename << "testimage-bool.vista3d";
       cvdebug() << "test with " << filename.str() << "\n";
       BOOST_REQUIRE(io.save(filename.str(), images));
       auto loaded = io.load(filename.str());
       BOOST_REQUIRE(loaded);
       BOOST_REQUIRE(loaded->size() == 1u);
       const auto& ploaded = dynamic_cast<const C3DBitImage&>(*(*loaded)[0]);
       iv = image->begin();
       auto il = ploaded.begin();

       while (iv != ev) {
              BOOST_CHECK_EQUAL(*il, *iv);
              ++iv;
              ++il;
       }

       unlink(filename.str().c_str());
}
