/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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
#include <mia/3d/filter/label.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
namespace bfs=boost::filesystem;
using namespace label_3dimage_filter;


static void check(const C3DImage& inp, const char* mask_descr, unsigned char *answer)
{
	P3DShape mask = C3DShapePluginHandler::instance().produce(mask_descr);
	BOOST_REQUIRE(mask);

	CLabel label(mask);
	P3DImage l = label.filter(inp);

	const C3DUBImage& result = dynamic_cast<const C3DUBImage& >(*l);

	BOOST_CHECK(equal(result.begin(), result.end(), answer));
}

BOOST_AUTO_TEST_CASE( test_label )
{
	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("..")/bfs::path("shapes"));
	C3DShapePluginHandler::set_search_path(kernelsearchpath);

	bool input[27] = { 1, 0, 0,
			   0, 0, 0,
			   0, 0, 0,

			   0, 0, 0,
			   0, 1, 0,
			   0, 0, 0,

			   0, 0, 0,
			   0, 0, 0,
			   0, 1, 1};

	unsigned char answer_6n[27] = { 1, 0, 0,
				    0, 0, 0,
				    0, 0, 0,

				    0, 0, 0,
				    0, 2, 0,
				    0, 0, 0,

				    0, 0, 0,
				    0, 0, 0,
				    0, 3, 3};

	unsigned char answer_18n[27] = { 1, 0, 0,
				     0, 0, 0,
				     0, 0, 0,

				     0, 0, 0,
				     0, 2, 0,
				     0, 0, 0,

				     0, 0, 0,
				     0, 0, 0,
				     0, 2, 2};

	unsigned char answer_26n[27] = { 1, 0, 0,
				     0, 0, 0,
				     0, 0, 0,

				     0, 0, 0,
				     0, 1, 0,
				     0, 0, 0,

				     0, 0, 0,
				     0, 0, 0,
				     0, 1, 1};


	C3DBounds size(3,3,3);

	C3DBitImage inp(size);
	copy (input, input+27, inp.begin());

	check(inp, "6n", answer_6n);
	check(inp, "18n", answer_18n);
	check(inp, "26n", answer_26n);

}
