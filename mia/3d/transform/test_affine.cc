/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

CSplineKernelTestPath kernel_test_path; 

struct ipfFixture {
	ipfFixture():
		ipf("bspline:d=3", "mirror")
		{
		} 
	C3DInterpolatorFactory ipf; 
}; 

struct TranslateTransFixture: public  ipfFixture{
	TranslateTransFixture():size(60, 80, 20),
				rtrans(size, ipf)
		{
			rtrans.translate(1.0, 2.0, 3.0);
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


BOOST_FIXTURE_TEST_CASE(test_affine3d, ipfFixture)
{
	C3DAffineTransformation t1(C3DBounds(10,20,15), ipf); 

	BOOST_CHECK_EQUAL(t1.degrees_of_freedom(), 12u);

	C3DFVector x0(1.0f, 2.0f, 3.0);

	C3DFVector y0 = t1(x0);
	BOOST_CHECK_EQUAL(y0, x0);

	t1.scale(std::log(2.0), std::log(3.0), std::log(4.0));

	C3DFVector y1 = t1(x0);
	BOOST_CHECK_EQUAL(y1, C3DFVector(2.0f, 6.0f, 12.0));

	t1.translate(1.0f, 2.0f, 1.5f);
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
