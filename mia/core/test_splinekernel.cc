/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>

#include <boost/filesystem/path.hpp>
#include <mia/core/splinekernel.hh>
#include <mia/core/boundary_conditions.hh>

namespace bfs=::boost::filesystem; 


NS_MIA_USE
using namespace std;
using namespace boost;

CSplineKernelTestPath init_path; 
CSplineBoundaryConditionTestPath bc_path; 

BOOST_AUTO_TEST_CASE( test_plugin_availability ) 
{
	const auto& handler = CSplineKernelPluginHandler::instance();

	BOOST_CHECK_EQUAL(handler.size(), 2u); 
	BOOST_CHECK_EQUAL(handler.get_plugin_names(), "bspline omoms "); 

}


class CBSplineKernelMock: public  CSplineKernel {
public:
	CBSplineKernelMock();
	virtual void get_weights(double x, VWeight& weight)const;
	virtual void get_derivative_weights(double x, VWeight& weight) const;
	virtual double get_weight_at(double x, int order) const;
	virtual void get_derivative_weights(double x, VWeight& weight, int order) const;
};


struct TestSplineCacheFixture {
	void check_cached(const CSplineBoundaryCondition& bc, double x, bool never_flat, int test_start_idx, 
			  const CSplineKernel::VIndex& test_indices); 

	void check_uncached(const CSplineBoundaryCondition& bc, double x, bool never_flat, int test_start_idx, 
			  const CSplineKernel::VIndex& test_indices); 

	CBSplineKernelMock kernel; 
}; 


BOOST_FIXTURE_TEST_CASE( test_mirrored_big, TestSplineCacheFixture ) 
{
	auto bc = produce_spline_boundary_condition("mirror"); 
	bc->set_width(127); 

	CSplineKernel::VIndex test_indices = {5, 6, 7, 8}; 
	check_cached(*bc, 32766.3828, true, 0, test_indices); 
	check_cached(*bc, 32766.3828, false, 0, test_indices);
	
}

CBSplineKernelMock::CBSplineKernelMock():
	CSplineKernel(3, 0, ip_bspline3)
{
}

void CBSplineKernelMock::get_weights(double x, VWeight& weight)const
{
	for(size_t i = 0; i < weight.size(); ++i) 
		weight[i] = x + i;
}

void CBSplineKernelMock::get_derivative_weights(double x, VWeight& weight) const
{
	for(size_t i = 0; i < weight.size(); ++i) 
		weight[i] = x + 2 * i;
}

double CBSplineKernelMock::get_weight_at(double x, int order) const
{
	return order * x + 1.0;
}

void CBSplineKernelMock::get_derivative_weights(double x, VWeight& weight, int order) const
{
	for(size_t i = 0; i < weight.size(); ++i) 
		weight[i] = order * (x + i);
}



void TestSplineCacheFixture::check_cached(const CSplineBoundaryCondition& bc, double x, bool never_flat, int test_start_idx, 
				   const CSplineKernel::VIndex& test_indices)
{
	CSplineKernel::SCache cache(kernel.size(), bc, never_flat); 
	
	kernel.get_cached(x, cache);

	BOOST_CHECK_EQUAL(cache.x, x); 
	
	cvdebug() << "cache {" 
		  << "  x = " <<  cache.x 
		  << "  start_idx = " << cache.start_idx
		  << "  is_flat = " << cache.is_flat
		  << "  index = " << cache.index
		  << "\n"; 

	if (!cache.is_flat) {
	    	BOOST_REQUIRE(cache.index.size() == test_indices.size()); 
		const CSplineKernel::VIndex& cindex = cache.index; 
		for (auto ci = cindex.begin(), ti = test_indices.begin(); 
		     ci != cindex.end(); ++ci, ++ti) 
			BOOST_CHECK_EQUAL(*ci, *ti); 
	} else {
		BOOST_REQUIRE(!never_flat); 
		BOOST_CHECK_EQUAL(cache.start_idx, test_start_idx); 
		BOOST_CHECK_EQUAL(cache.index[0], test_start_idx); 
	}
}

void TestSplineCacheFixture::check_uncached(const CSplineBoundaryCondition& bc, double x, bool never_flat, int test_start_idx, 
				   const CSplineKernel::VIndex& test_indices)
{
	CSplineKernel::SCache cache(kernel.size(), bc, never_flat); 
	
	kernel.get_uncached(x, cache);

	BOOST_CHECK_EQUAL(cache.x, x); 

	if (!cache.is_flat) {
	    	BOOST_REQUIRE(cache.index.size() == test_indices.size()); 
		const CSplineKernel::VIndex& cindex = cache.index; 
		for (auto ci = cindex.begin(), ti = test_indices.begin(); 
		     ci != cindex.end(); ++ci, ++ti) 
			BOOST_CHECK_EQUAL(*ci, *ti); 
	} else {
		BOOST_REQUIRE(!never_flat); 
		BOOST_CHECK_EQUAL(cache.start_idx, test_start_idx); 
		BOOST_CHECK_EQUAL(cache.index[0], test_start_idx); 
	}

}

