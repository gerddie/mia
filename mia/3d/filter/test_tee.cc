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

#include <mia/internal/plugintester.hh>
#include <mia/3d/filter/tee.hh>
#include <mia/core/datapool.hh>
#include <mia/3d/3dimageio.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace tee_3dimage_filter;

struct TeeFixture {
	TeeFixture(); 
	~TeeFixture(); 
	

	C3DUBImage *orig; 
	P3DImage image; 
}; 

TeeFixture::TeeFixture()
{
	const unsigned char init[4] = {1,2,3,4}; 
	orig = new C3DUBImage(C3DBounds(2,2,1), init); 
	image.reset(orig); 
}

TeeFixture::~TeeFixture()
{
	boost::any dummy = CDatapool::instance().get_and_remove("test.@");
}
	

BOOST_FIXTURE_TEST_CASE( test_3dfilter_tee_shared_ptr, TeeFixture )
{
	auto t = BOOST_TEST_create_from_plugin<C3DTeeFilterPluginFactory>("tee:file=test.@");
	auto passthrough = t->filter(image); 
	BOOST_CHECK(*image == *passthrough);
	auto loaded = load_image3d("test.@");
	BOOST_CHECK(*image == *loaded);
}

BOOST_FIXTURE_TEST_CASE( test_3dfilter_tee, TeeFixture )
{
	auto t = BOOST_TEST_create_from_plugin<C3DTeeFilterPluginFactory>("tee:file=test.@");
	auto passthrough2 = t->filter(*image); 
	BOOST_CHECK(*image == *passthrough2);
	auto loaded2 = load_image3d("test.@");
	BOOST_CHECK(*image == *loaded2);
}