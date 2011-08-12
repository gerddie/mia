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

#include <stdexcept>
#include <cmath>

#include <mia/core/fftslopeclassifier.hh>


using namespace std;
using namespace mia;

struct SlopeClassifierFixture {
	typedef struct {
		vector<int> periodic_indices;
		int RV_idx;
		int LV_idx;
		int baseline_idx;
		int perfusion_idx;
		int RV_peak;
		int LV_peak;
	} Result;
	void run(size_t length, size_t components, const float *data, const Result& r, bool mean_removed = false);
	void check_equal(int a, int b, const char *descr); 
};

#define BOOST_CHECK_EQUAL_DESCR( L, R, D )				\
	BOOST_CHECK_WITH_ARGS_IMPL( ::boost::test_tools::tt_detail::equal_impl_frwd(), D, CHECK, CHECK_EQUAL, (L)(R) )


void SlopeClassifierFixture::check_equal(int value, int expect, const char *descr)
{
	if (expect != -2) 
		BOOST_CHECK_EQUAL_DESCR( value, expect, descr ); 
}

void SlopeClassifierFixture::run(size_t length, size_t components, const float *data, const Result& result, bool mean_removed)
{
	CSlopeClassifier::Columns columns( components );

	for (size_t c = 0; c < components; ++c)
		columns[c].resize(length);

	const float *i = data;
	for (size_t r = 0; r < length; ++r)
		for (size_t c = 0; c < components; ++c, ++i)
			columns[c][r]  = *i;

	CSlopeClassifier c(columns, mean_removed);
	auto periodic = c.get_periodic_indices(); 
	check_equal(periodic.size(), result.periodic_indices.size(), "periodic index size"); 
	for (auto ip = periodic.begin(), it = result.periodic_idx.begin(); ip != periodic.end(); 
	     ++ip, ++it)
		check_equal(*ip), *it, "periodic index");

	check_equal(c.get_RV_idx(), result.RV_idx, "RV index");
	
	check_equal(c.get_LV_idx(), result.LV_idx, "LV index");
//	check_equal(c.get_perfusion_idx(), result.perfusion_idx, "perfusion index");
//	check_equal(c.get_baseline_idx(), result.baseline_idx, "baseline index");
	check_equal(c.get_RV_peak(), result.RV_peak, "RV peak");
	check_equal(c.get_LV_peak(), result.LV_peak, "LV peak");
}



