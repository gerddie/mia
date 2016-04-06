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
#include <mia/core/cmeansinit/static.hh>

NS_MIA_USE

BOOST_AUTO_TEST_CASE (test_even)
{
	
	auto initializer = BOOST_TEST_create_from_plugin<CEqualInitializerPlugin>("even:nc=3");

	CMeans::NormalizedHistogram nh;

	auto classes = initializer->run(nh);

	BOOST_CHECK_EQUAL(classes.size(), 3u);

	BOOST_CHECK_EQUAL(classes[0], 0.0f);
	BOOST_CHECK_EQUAL(classes[1], 0.5f);
	BOOST_CHECK_EQUAL(classes[2], 1.0f);
}

BOOST_AUTO_TEST_CASE (test_predefined)
{
	
	auto initializer = BOOST_TEST_create_from_plugin<CPredefinedInitializerPlugin>("predefined:cc=[0.1,0.2,0.3,0.9]");

	CMeans::NormalizedHistogram nh;

	auto classes = initializer->run(nh);

	BOOST_CHECK_EQUAL(classes.size(), 4u);

	BOOST_CHECK_CLOSE(classes[0], 0.1f, 0.1);
	BOOST_CHECK_CLOSE(classes[1], 0.2f, 0.1);
	BOOST_CHECK_CLOSE(classes[2], 0.3f, 0.1);
	BOOST_CHECK_CLOSE(classes[3], 0.9f, 0.1);
}



