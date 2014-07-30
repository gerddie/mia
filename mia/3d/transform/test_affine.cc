/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <mia/3d/transform/affine.hh>

NS_MIA_USE

using namespace std;
using namespace ::boost;
using namespace boost::unit_test;
namespace bfs=boost::filesystem;

struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C3DInterpolatorFactory ipf; 
}; 

void affin_add_translate(C3DAffineTransformation& rtrans, double x, double y, double z) 
{
	auto params = rtrans.get_parameters(); 
	params[3] += x; 
	params[7] += y; 
	params[11] += z; 
	rtrans.set_parameters(params); 
}

void affin_add_scale(C3DAffineTransformation& rtrans, double x, double y, double z) 
{
	auto params = rtrans.get_parameters(); 
	const double expx = exp(x);
	const double expy = exp(y);
	const double expz = exp(z);
	params[0] *= expx;
	params[1] *= expx;
	params[2] *= expx;
	params[3] *= expx;
	params[4] *= expy;
	params[5] *= expy;
	params[6] *= expy;
	params[7] *= expy;
	params[8] *= expz;
	params[9] *= expz;
	params[10] *= expz;
	params[11] *= expz;
	rtrans.set_parameters(params); 
}


struct TranslateTransFixture: public  ipfFixture{
	TranslateTransFixture():size(60, 80, 20),
				rtrans(size, ipf)
		{
			affin_add_translate(rtrans, 1.0, 2.0, 3.0);
		}
	C3DBounds size;
	C3DAffineTransformation rtrans;
};


BOOST_FIXTURE_TEST_CASE(basics_TranslateTransFixture, TranslateTransFixture)
{
	C3DFVector x(2, 4, 5);
	BOOST_CHECK_EQUAL(rtrans.degrees_of_freedom(), 12u);
	BOOST_CHECK_EQUAL(C3DFVector(3.0, 6.0, 8.0),  rtrans(x));
}

BOOST_FIXTURE_TEST_CASE(max_TranslateTransFixture, TranslateTransFixture)
{
	BOOST_CHECK_CLOSE(rtrans.get_max_transform(), sqrtf(14.0), 0.1);
}

BOOST_FIXTURE_TEST_CASE(set_identity_TranslateTransFixture, TranslateTransFixture)
{
	rtrans.set_identity();

	C3DAffineTransformation::const_iterator ti = rtrans.begin();

	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}
}

BOOST_FIXTURE_TEST_CASE(derivative_TranslateTransFixture, TranslateTransFixture)
{
	C3DFMatrix d = 	rtrans.derivative_at(10,10,10);
	BOOST_CHECK_EQUAL(d.x.x, 1.0f);
	BOOST_CHECK_EQUAL(d.x.y, 0.0f);
	BOOST_CHECK_EQUAL(d.x.z, 0.0f);
	BOOST_CHECK_EQUAL(d.y.x, 0.0f);
	BOOST_CHECK_EQUAL(d.y.y, 1.0f);
	BOOST_CHECK_EQUAL(d.y.z, 0.0f);
	BOOST_CHECK_EQUAL(d.z.x, 0.0f);
	BOOST_CHECK_EQUAL(d.z.y, 0.0f);
	BOOST_CHECK_EQUAL(d.z.z, 1.0f);

}

BOOST_FIXTURE_TEST_CASE(derivative_TranslateTransFixture_off_grid, TranslateTransFixture)
{
	C3DFMatrix d = 	rtrans.derivative_at(C3DFVector(10.1,10.2,10.3));
	BOOST_CHECK_EQUAL(d.x.x, 1.0f);
	BOOST_CHECK_EQUAL(d.x.y, 0.0f);
	BOOST_CHECK_EQUAL(d.x.z, 0.0f);
	BOOST_CHECK_EQUAL(d.y.x, 0.0f);
	BOOST_CHECK_EQUAL(d.y.y, 1.0f);
	BOOST_CHECK_EQUAL(d.y.z, 0.0f);
	BOOST_CHECK_EQUAL(d.z.x, 0.0f);
	BOOST_CHECK_EQUAL(d.z.y, 0.0f);
	BOOST_CHECK_EQUAL(d.z.z, 1.0f);

}


BOOST_FIXTURE_TEST_CASE(test_affine3d, ipfFixture)
{
	C3DAffineTransformation t1(C3DBounds(10,20,15), ipf); 

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 12u);

	C3DFVector x0(1.0f, 2.0f, 3.0);

	C3DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	affin_add_scale(t1, std::log(2.0), std::log(3.0), std::log(4.0));

	C3DFVector y1 = t1(x0);
	BOOST_CHECK_EQUAL(y1, C3DFVector(2.0f, 6.0f, 12.0));

	affin_add_translate(t1, 1.0f, 2.0f, 1.5f);
	BOOST_CHECK_EQUAL(t1(x0), C3DFVector(3.0f, 8.0f, 13.5));

}


BOOST_FIXTURE_TEST_CASE( test_affine3d_iterator, ipfFixture )
{
	C3DBounds size(10,20,15);

	C3DAffineTransformation t1(size, ipf);
	C3DAffineTransformation::const_iterator ti = t1.begin();
	
	for (size_t z = 0; z < size.z; ++z)
		for (size_t y = 0; y < size.y; ++y)
			for (size_t x = 0; x < size.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}
	
	BOOST_CHECK(ti == t1.end());
}

BOOST_FIXTURE_TEST_CASE( test_affine3d_ranged_iterator, ipfFixture)
{
	C3DBounds size(10,20,30);
	C3DBounds delta(1,2,3); 

	C3DAffineTransformation t1(size, ipf);
	auto ti = t1.begin_range(delta, size - delta);

	for (size_t z = delta.z; z < size.z - delta.z; ++z)
		for (size_t y = delta.y; y < size.y - delta.y; ++y)
			for (size_t x = delta.x; x < size.x - delta.x; ++x, ++ti) {
				BOOST_CHECK_EQUAL(*ti, C3DFVector(x, y, z));
			}

	BOOST_CHECK(ti == t1.end_range(delta, size - delta));
}


BOOST_FIXTURE_TEST_CASE( test_affine_clone, TranslateTransFixture )
{
	P3DTransformation clone(rtrans.clone());

	C3DAffineTransformation& spclone = dynamic_cast<C3DAffineTransformation&>(*clone);

	BOOST_CHECK_EQUAL(spclone.get_size(), rtrans.get_size());

	C3DTransformation::const_iterator ic = spclone.begin();
	C3DTransformation::const_iterator ec = spclone.end();

	C3DTransformation::const_iterator io = rtrans.begin();

	while (ic != ec ) {
		BOOST_CHECK_EQUAL(*ic, *io);
		++ic;
		++io;
	}
}

BOOST_FIXTURE_TEST_CASE (test_invers, ipfFixture)
{
	C3DBounds size(10,20, 30); 
	C3DAffineTransformation trans(size, ipf); 

	auto params = trans.get_parameters(); 
	BOOST_REQUIRE(params.size()== 12); 

	const float init_matrix[12] = {1,2,3,4, 2,3,1,2, 3, 2, 2, 2}; 
	const float inv_matrix[12]  = { 
		-4.0/13.0, -2.0/13.0,  7.0/13.0,   6.0/13.0, 
		 1.0/13.0,  7.0/13.0, -5.0/13.0,  -8.0/13.0, 
		 5.0/13.0, -4.0/13.0,  1.0/13.0, -14.0/13.0
	}; 
	copy(init_matrix, init_matrix+12, params.begin()); 

	
	trans.set_parameters(params); 
	
	unique_ptr<C3DTransformation> inverse( trans.invert()); 
	BOOST_CHECK_EQUAL(inverse->get_size(), size);

	params = inverse->get_parameters(); 
	BOOST_REQUIRE(params.size()== 12); 
	for(int i = 0; i < 12; ++i) 
		BOOST_CHECK_CLOSE(params[i], inv_matrix[i], 0.1); 
	
}
