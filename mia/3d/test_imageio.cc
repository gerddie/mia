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

#include <mia/internal/autotest.hh>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/insert_range.hpp>

#include <mia/core/attribute_names.hh>
#include <mia/core/msgstream.hh>
#include <mia/3d/imageio.hh>

using namespace std;
using namespace mia;
namespace bmpl = boost::mpl;

template <typename Pixel>
void store_and_load(const char *suffix)
{
       vector<unsigned char> values{
              11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
              31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42};
       C3DBounds size(2, 3, 4);
       const C3DFVector voxel_size(2, 3, 4);
       T3DImage<Pixel> image(size);
       image.set_voxel_size(voxel_size);
       copy(values.begin(), values.end(), image.begin());
       stringstream filenamestr;
       filenamestr << "test3dio-" << __type_descr<Pixel>::value << "." << suffix;
       auto filename = filenamestr.str();
       BOOST_REQUIRE(save_image(filename, image));
       auto test_image = load_image3d(filename);
       const T3DImage<Pixel>& loaded = dynamic_cast<const T3DImage<Pixel>&>(*test_image);
       BOOST_CHECK_EQUAL(loaded.get_size(), size);
       BOOST_CHECK_EQUAL(loaded.get_voxel_size(), voxel_size);
       auto p = loaded.begin();
       auto tv = values.begin();
       auto tve = values.end();

       while (tv != tve) {
              BOOST_CHECK_EQUAL(*p, *tv);
              ++p;
              ++tv;
       }

       unlink(filename.c_str());

       if (strcmp(suffix, "hdr") == 0) {
              stringstream rawfilenamestr;
              rawfilenamestr << "test3dio-" << __type_descr<Pixel>::value << ".img";
              unlink(rawfilenamestr.str().c_str());
       }
}

struct InriaPixelTypes {
       typedef bmpl::vector<signed char,
               unsigned char,
               signed short,
               unsigned short,
               signed int,
               unsigned int,
               float,
               double
               > type;
};


BOOST_AUTO_TEST_CASE_TEMPLATE(test_volume_io_inria, T, InriaPixelTypes::type )
{
       store_and_load<T>("inr");
}


struct VffPixelTypes {
       typedef bmpl::vector<unsigned char,
               signed short
               > type;
};


BOOST_AUTO_TEST_CASE_TEMPLATE(test_volume_io_vff, T, VffPixelTypes::type )
{
       store_and_load<T>("vff");
}


struct AnalyzePixelTypes {
       typedef bmpl::vector<unsigned char,
               signed short,
               signed int,
               float,
               double
               > type;
};


BOOST_AUTO_TEST_CASE_TEMPLATE(test_volume_io_analyze, T, AnalyzePixelTypes::type )
{
       store_and_load<T>("hdr");
}
