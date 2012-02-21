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
#include <mia/template/filtertest.hh>
#include <mia/2d/filter/sortlabel.hh>

#include <boost/mpl/vector.hpp>
#include <boost/test/test_case_template.hpp>
namespace bmpl=boost::mpl;


NS_USE(sort_label_2dimage_filter); 

NS_MIA_USE
using namespace std;

BOOST_FIXTURE_TEST_CASE(test_sort_ubyte, TFiltertestFixture<T2DImage>)
{
	// 5 - 5   1 
	// 4 - 3   2
	// 3 - 1   5
	// 2 - 2   4
	// 1 - 2   3
	// 9 - 1   6
	
	const unsigned char src[15] =   { 0, 5, 5, 5, 5, 5, 4, 4, 4, 1, 1, 2, 2, 9, 3};
	const unsigned char ref[15] =   { 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3 ,4, 4, 6, 5};

	C2DBounds size(3,5);

	auto filter = BOOST_TEST_create_from_plugin<CSortLabelFilterPlugin>("sort-label"); 
	
	run(size, src, size, ref, *filter); 
}

BOOST_FIXTURE_TEST_CASE(test_sort_ushort, TFiltertestFixture<T2DImage>)
{
	// 5 - 5   1 
	// 4 - 3   2
	// 3 - 1   5
	// 2 - 2   4
	// 1 - 2   3
	// 9 - 1   6
	
	const unsigned short src[15] =   { 0, 5, 5, 5, 5, 5, 4, 4, 4, 1, 1, 2, 2, 9, 3};
	const unsigned short ref[15] =   { 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3 ,4, 4, 6, 5};

	C2DBounds size(3,5);

	auto filter = BOOST_TEST_create_from_plugin<CSortLabelFilterPlugin>("sort-label"); 
	
	run(size, src, size, ref, *filter); 
}


typedef bmpl::vector<signed char,
		     signed short,
		     signed int,
		     unsigned int,
		     float,
		     double
#ifdef HAVE_INT64
		     ,mia_int64,
		     mia_uint64
#endif
		     > __PixelTypes;

BOOST_AUTO_TEST_CASE_TEMPLATE(test_sort_unsupported, T, __PixelTypes)
{
	CSortLabel sortlabel;
	C2DBounds size(1,1);

	BOOST_CHECK_THROW(sortlabel.filter(T2DImage<T>(size)), invalid_argument); 
}
