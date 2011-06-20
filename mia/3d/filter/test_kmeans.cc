/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
 * ETSI Telecomunicacion, UPM
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
#include <mia/3d/filter/kmeans.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace kmeans_3dimage_filter;


BOOST_AUTO_TEST_CASE( test_3dfilter_kmeans )
{
	const size_t size_x = 5;
	const size_t size_y = 4;
	const size_t size_z = 1;

	const float src[size_y*size_x*size_z] = {
		1.0, 3.0,  2.0, 10.4,  2.4, 11.0, 11.1, 25.1, 25.2, 28.0,
		29.7,11.1,12.2, 28.0, 29.9,  1.0,  2.1, 27.1,  2.5,  3.1
	};

	// "hand filtered" w = 1 -> 3x3x1
	const int src_ref[size_y * size_x*size_z] = {
		0, 0, 0, 1, 0, 1, 1, 2, 2, 2,
		2, 1, 1, 2, 2, 0, 0, 2, 0, 0
	};



	C3DBounds size(size_x, size_y, size_z);

	C3DFImage src_img(size, src);

	C3DKMeans kmeans(3);

	P3DImage res_wrap = kmeans.filter(src_img);

	C3DUBImage* res_img = dynamic_cast<C3DUBImage*>(res_wrap.get());
	BOOST_REQUIRE(res_img);
	BOOST_REQUIRE(res_img->get_size() == src_img.get_size());
	
	for (size_t z = 0; z < size_z; ++z)
		for (size_t y = 0; y < size_y; ++y)
			for (size_t x = 0; x < size_x; ++x)
				BOOST_CHECK_EQUAL((*res_img)(x,y,z), src_ref[y * size_x + x + z * size_x * size_z]);

	// test attribute
	PAttribute pattr = res_wrap->get_attribute(ATTR_IMAGE_KMEANS_CLASSES);
	const CVDoubleAttribute& attr = dynamic_cast<const CVDoubleAttribute&>(*pattr);
	std::vector<double> cls = attr;
	BOOST_CHECK_EQUAL(cls.size(), cls.size());

}
