/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/internal/plugintester.hh>
#include <mia/2d/filter/label.hh>

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace label_2dimage_filter;
namespace bfs=boost::filesystem;

static void do_test_label(const C2DImage& inp, const char* filter_descr, unsigned char *answer)
{

	auto label = BOOST_TEST_create_from_plugin<C2DLabelFilterPlugin>(filter_descr); 
	
	P2DImage l = label->filter(inp);

	C2DUBImage *result = dynamic_cast<C2DUBImage *>(l.get());
	BOOST_REQUIRE(result);

	BOOST_CHECK(equal(result->begin(), result->end(), answer));
}

BOOST_AUTO_TEST_CASE( test_label )
{

	bool input[9] = { 1, 1, 0,
			  0, 0, 1,
			  0, 1, 1 };


	unsigned char answer_4n[9] = { 1, 1, 0,
				       0, 0, 2,
				       0, 2, 2};

	unsigned char answer_8n[9] = { 1, 1, 0,
				       0, 0, 1,
				       0, 1, 1};


	C2DBounds size(3,3);

	C2DBitImage inp(size);
	copy (input, input+9, inp.begin());

	do_test_label(inp, "label:n=4n", answer_4n);
	do_test_label(inp, "label:n=8n", answer_8n);


}

