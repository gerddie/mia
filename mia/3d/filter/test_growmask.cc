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
#include <mia/3d/filter/growmask.hh>
namespace bfs=boost::filesystem;

NS_MIA_USE
using namespace std;
using namespace ::boost;
using namespace ::boost::unit_test;
using namespace growmask_3dimage_filter;

C3DShapePluginHandlerTestPath shape_path; 

template <typename T> 
struct void_destructor {
	/// skip deleting the pointer 
	virtual void operator () (T *) {
	}
}; 


const size_t nx = 5;
const size_t ny = 4;
const size_t nz = 3;

BOOST_FIXTURE_TEST_CASE( test_grow_mask, TFiltertestFixture<T3DImage> )
{
	bool result_mask[nx * ny * nz] = {
		0, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 1, 0,
		1, 1, 1, 0, 0,

		0, 1, 1, 0, 1,
		1, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 0, 0,

		0, 1, 1, 1, 1,
		1, 1, 1, 0, 1,
		1, 1, 0, 1, 0,
		1, 1, 1, 1, 0
	};


	unsigned short init_ref[nx * ny * nz] = {
		0, 1, 2, 3, 1,
		5, 6, 7, 1, 0,
		3, 4, 1, 1, 3,
		4, 5, 1, 2, 6,

		0, 2, 4, 5, 1,
		5, 6, 4, 0, 1,
		5, 3, 0, 1, 5,
		7, 8, 1, 3, 4,

		0, 2, 4, 1, 1,
		5, 6, 1, 0, 1,
		5, 3, 0, 1, 5,
		7, 1, 1, 1, 4
	};

	bool init_mask[nx * ny * nz] = {
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,

		0, 0, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,
		0, 1, 0, 0, 0,

		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0
	};
	
	C3DBounds size(nx,ny,nz); 
	C3DUSImage ref(size, init_ref); 
	P3DImage pref(&ref, void_destructor<C3DImage>()); 
	
	CDatapool::instance().add("ref.datapool", create_image3d_vector(pref));
	
	auto filter = BOOST_TEST_create_from_plugin<C3DGrowmaskImageFilterFactory>("growmask:ref=ref.datapool,shape=6n,min=1"); 
	
	run(size, init_mask, size, result_mask, *filter); 
	
}


