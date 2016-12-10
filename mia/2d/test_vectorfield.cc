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

#include <cmath>
#include <mia/internal/autotest.hh>

#include <mia/core/spacial_kernel.hh>
#include <mia/2d/interpolator.hh>
#include <mia/2d/vectorfield.hh>

using namespace mia; 

BOOST_AUTO_TEST_CASE (test_vectorfield_as_inverse_of)
{
	C2DBounds size(10,10);
	
	C2DFVectorfield other(size);
	
	for (auto io = other.begin_range(C2DBounds::_0, size);
	     io != other.end_range(C2DBounds::_0, size); ++io)  {
		*io = C2DFVector(sin(io.pos().x * M_PI / 9), sin(io.pos().y * M_PI / 18));
	}
	
	C2DFVectorfield me(size);
	me.update_as_inverse_of(other, 1e-14f, 50);

        C2DInterpolatorFactory ipf(produce_spline_kernel("bspline:d=1"), 
                                   *produce_spline_boundary_condition("zero"), 
                                   *produce_spline_boundary_condition("zero"));
        
        
        std::unique_ptr<T2DInterpolator<C2DFVector>  > interp_me(ipf.create(me));
        std::unique_ptr<T2DInterpolator<C2DFVector>  > interp_other(ipf.create(other));

        for(unsigned y = 0; y < 10; ++y)
                for(unsigned x = 0; x < 10; ++x) {
                        
                        C2DFVector pos(x,y);
                        C2DFVector pos_t = pos - (*interp_me)(pos);
                        C2DFVector pos_tt = pos_t - (*interp_other)(pos_t) - pos;
                        
                        cvdebug() << pos << ": delta = " << pos_tt << "\n";
                        
                        BOOST_CHECK_SMALL(pos_tt.x, 1e-5f);
                        BOOST_CHECK_SMALL(pos_tt.y, 1e-5f);

                        
                        // test also the inverse, but with a higher tolerance,
                        // since this was not optimized 
                        C2DFVector pos_it = pos - (*interp_other)(pos);
                        C2DFVector pos_inv = (*interp_me)(pos_it);
                        C2DFVector pos_itt = pos_it - pos_inv - pos;
                        cvdebug() << pos << ": pos-other = "<< pos_it
                                  << ", intp = " << pos_inv
                                  << ", inv delta = " << pos_itt << "\n";
                        
                        BOOST_CHECK_SMALL(pos_itt.x, 0.05f); 
                        BOOST_CHECK_SMALL(pos_itt.y, 0.05f); 
                        
                }
}


using namespace std;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct FieldSplineFixture {
	FieldSplineFixture():
		size(30, 32),
		field(size),
		ipf(produce_spline_kernel("bspline:d=4"), 
		    *produce_spline_boundary_condition("mirror"), 
		    *produce_spline_boundary_condition("mirror"))
	{
		auto i = field.begin();
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++i) {
				*i = C2DFVector( fx(x, y), fy(x, y));
			}

		std::shared_ptr<T2DInterpolator<C2DFVector>  > source(ipf.create(field));

	}
	C2DBounds size;
	C2DFVectorfield field;
	C2DInterpolatorFactory ipf;

protected:
	float fx(float x, float y);
	float fy(float x, float y);
};

float FieldSplineFixture::fx(float x, float y)
{
	x /= 25.0;
	y /= 50.0;
	return (x + 1.0) / (x +  y  * x + 2.0);
}

float FieldSplineFixture::fy(float x, float y)
{
	x /= 25.0;
	y /= 50.0;
	return (y + 1) / (x * x + y + 10);
}

BOOST_FIXTURE_TEST_CASE( test_vector_trilininterp , FieldSplineFixture )
{
	C2DFVector iploc(20.4, 17.3);
	auto ip = field.get_interpol_val_at(iploc);

	BOOST_CHECK_CLOSE(ip.x, fx(iploc.x, iploc.y), 0.005);
	BOOST_CHECK_CLOSE(ip.y, fy(iploc.x, iploc.y), 0.005); 
	
}


BOOST_FIXTURE_TEST_CASE( test_vector_field2d_splines, FieldSplineFixture )
{
	std::shared_ptr<T2DInterpolator<C2DFVector>  > source(ipf.create(field));

	C2DFVector result = (*source)(C2DFVector(5.4, 23.5));

	BOOST_CHECK_CLOSE(result.x, fx(5.4, 23.5), 0.1);
	BOOST_CHECK_CLOSE(result.y, fy(5.4, 23.5), 0.1);

}


BOOST_AUTO_TEST_CASE( test_vf_cat )
{

	C2DFVector init_a[9] = {
		C2DFVector(2,3), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(-1,-1), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0)
	};

	C2DFVector init_b[9] = {
		C2DFVector(0,0), C2DFVector(0,0), C2DFVector(0,0),
		C2DFVector(0,0), C2DFVector(1,1), C2DFVector(0.5,0),
		C2DFVector(0,0), C2DFVector(0,0.5), C2DFVector(4,5)
	};


	C2DBounds size(3,3);
	C2DFVectorfield a(size);
	C2DFVectorfield b(size);

	copy(init_a, init_a + 9, a.begin());
	copy(init_b, init_b + 9, b.begin());

	a += b;

	BOOST_CHECK( a(1,1) == C2DFVector(3,4));

	BOOST_CHECK( a(2,1) == C2DFVector(0.0,-0.5));
	BOOST_CHECK( a(1,2) == C2DFVector(-0.5,0.0));

}

