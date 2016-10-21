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

#include <mia/internal/autotest.hh>
#include <mia/3d/fifotestfixture.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/fifof/mlv.hh>

NS_USE(mlv_2dstack_filter);
NS_MIA_USE; 
using namespace std; 



BOOST_FIXTURE_TEST_CASE( test_fifof_mlv_const, fifof_Fixture )
{
	const size_t slices = 6;
	const int size_x = 6;
	const int size_y = 6;
	const C2DBounds size(size_x,size_y);

	float input_data[slices * size_x * size_y] = {
		2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	};

	float test_data[slices * size_x * size_y] = {
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	};

	prepare(input_data, test_data, size, slices);
	C2DMLVnFifoFilter f(1);

	call_test(f);
}

BOOST_FIXTURE_TEST_CASE( test_fifof_mlv_variable, fifof_Fixture )
{
	srand48(0); 
	for (int w = 1; w < 2; ++w) {
		cvdebug() << "test filter of width " << w << "\n";

		stringstream help_filter_descr; 
		help_filter_descr << "mlv:w=" << w; 

		int isize = 4 * w + 2;
		int zsize = isize + 20; 
		C3DBounds size(isize, isize, zsize);
		C3DBounds mid(2 * w + 1, 2 * w + 1, 2 * w + 11); 

		C3DFImage src(size);
		for (auto i = src.begin_range(C3DBounds::_0, size), e = src.end_range(C3DBounds::_0, size); 
		     i != e; ++i) {
			*i = (i.pos() - mid).norm2(); 
		}
		
		auto ref = run_filter(src, help_filter_descr.str().c_str());
		const C3DFImage& test_data = dynamic_cast<const C3DFImage&>(*ref); 
		
		C2DBounds slice_size(isize, isize); 
		cvdebug() << "Test image size " << slice_size << " with " << zsize << " slices\n"; 
		prepare(&src(0,0,0), &test_data(0,0,0), slice_size, zsize);
		C2DMLVnFifoFilter filter(w);
		call_test(filter);
	}

}
