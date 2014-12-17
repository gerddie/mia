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

#define BOOST_TEST_MODULE TEST_ORIENTATION 
#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/3d/orientation.hh>


NS_MIA_USE
using namespace std;

static void run_test_read(const string& id, E3DImageOrientation test_orient, 
			  const string& expect)
{
	E3DImageOrientation orient;
	istringstream is(id);
	is >> orient;
	BOOST_CHECK_EQUAL(orient, test_orient);

	stringstream os;
	os << test_orient;
	BOOST_CHECK_EQUAL(os.str(), expect);

}

BOOST_AUTO_TEST_CASE( test_orientation_streamio )
{
	run_test_read("axial", ior_axial, "axial");
	run_test_read("coronal", ior_coronal, "coronal");
	run_test_read("saggital", ior_saggital, "saggital");
	run_test_read("unknown", ior_undefined, "(undefined)");
}

BOOST_AUTO_TEST_CASE( test_orientation_attribute )
{
	COrientationTranslator::register_for("orientation");
	PAttribute attr = CStringAttrTranslatorMap::instance().to_attr("orientation", "axial");
	C3DImageOrientation *io = dynamic_cast<C3DImageOrientation *>(attr.get());
	BOOST_REQUIRE(io);

	E3DImageOrientation orient = *io;
	BOOST_CHECK_EQUAL(orient, ior_axial);
}

struct OrientationTestFixture {
	void check_transform(const C3DOrientationAndPosition& op, const vector<double>& expect); 
	void check_inv_transform(const C3DOrientationAndPosition& op, const vector<double>& expect); 
private: 
	void check(const CDoubleVector& params, const vector<double>& expect); 
	
}; 

BOOST_FIXTURE_TEST_CASE(test_scale_x, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector(2.0, 1, 1), 
				     Quaternion::_1);

	vector<double> expect = {
		2.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		0.5, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 
	
}


BOOST_FIXTURE_TEST_CASE(test_shift_x, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector(3.0,0,0), C3DFVector::_1, 
				     Quaternion::_1);

	vector<double> expect = {
		1.0, 0.0, 0.0, 3.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0,-3.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 
	
}

BOOST_FIXTURE_TEST_CASE(test_rot_x, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector::_1, 
				     Quaternion(C3DDVector(0.5 * M_PI, 0, 0)));

	vector<double> expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 0.0,-1.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0,-1.0, 0.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 
}


BOOST_FIXTURE_TEST_CASE(test_scale_y, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector(1.0, 2.0, 1), 
				     Quaternion::_1);

	vector<double> expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 2.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 0.5, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 

}

BOOST_FIXTURE_TEST_CASE(test_shift_y, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default,  C3DFVector(0.0, 2.0, 0), C3DFVector::_1,
				     Quaternion::_1);

	vector<double> expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 2.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0,-2.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 

}

BOOST_FIXTURE_TEST_CASE(test_rot_y, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector::_1, 
				     Quaternion(C3DDVector(0, 0.5 * M_PI, 0)));

	vector<double> expect = {
		0.0, 0.0, 1.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
	       -1.0, 0.0, 0.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		0.0, 0.0,-1.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		1.0, 0.0, 0.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 
	
}


BOOST_FIXTURE_TEST_CASE(test_scale_z, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector(1.0, 1.0, 2.0), 
				     Quaternion::_1);

	vector<double> expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 2.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 0.5, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 

}

BOOST_FIXTURE_TEST_CASE(test_shift_z, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector(0.0, 0.0, 7.0), C3DFVector::_1, 
				     Quaternion::_1);

	vector<double> expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 7.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0,-7.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 

}

BOOST_FIXTURE_TEST_CASE(test_rot_z, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector::_1, 
				     Quaternion(C3DDVector(0, 0, 0.5 * M_PI)));

	vector<double> expect = {
		0.0,-1.0, 0.0, 0.0, 
		1.0, 0.0, 0.0, 0.0, 
	        0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		0.0, 1.0, 0.0, 0.0, 
	       -1.0, 0.0, 0.0, 0.0, 
	        0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
	check_inv_transform(op, inv_expect); 
	
}



// this is the left handed rotation 
BOOST_FIXTURE_TEST_CASE(test_rot_xyz, OrientationTestFixture) 
{
	C3DOrientationAndPosition op(ior_default, C3DFVector::_0, C3DFVector::_1, 
				     Quaternion(C3DDVector(M_PI / 6.0, M_PI / 3.0, 3 * M_PI/4.0)));

	double cos_psi = -sqrt(2.0) / 2.0; 
	double sin_psi = sqrt(2.0) / 2.0; 

	double cos_theta = 0.5; 
	double sin_theta = sqrt(3)/2.0; 

	double cos_phi = sqrt(3)/2.0; 
	double sin_phi = 0.5; 

	double a11 = cos_theta * cos_psi; 
	double a12 = - cos_phi * sin_psi + sin_phi * sin_theta * cos_psi; 
	double a13 = sin_phi * sin_psi + cos_phi * sin_theta * cos_psi; 
	
	double a21 = cos_theta * sin_psi; 
	double a22 = cos_phi * cos_psi + sin_phi * sin_theta * sin_psi; 
	double a23 = - sin_phi * cos_psi + cos_phi * sin_theta * sin_psi; 
	
	double a31 = - sin_theta; 
	double a32 = sin_phi * cos_theta;
	double a33 = cos_phi * cos_theta;


	vector<double> expect = {
		a11, a12, a13, 0.0, 
		a21, a22, a23, 0.0, 
		a31, a32, a33, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};

	check_transform(op, expect); 

	vector<double> inv_expect = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0,-1.0, 0.0, 0.0, 
		0.0, 0.0, 0.0, 1.0, 
	};
	
//	check_inv_transform(op, inv_expect); 
}


void OrientationTestFixture::check_transform(const C3DOrientationAndPosition& op, const vector<double>& expect)
{
	CDoubleVector params(16); 
	op.get_transform_parameters(params); 
	check(params, expect); 
}

void OrientationTestFixture::check_inv_transform(const C3DOrientationAndPosition& op, const vector<double>& expect)
{
	CDoubleVector params(16); 
	op.get_inverse_transform_parameters(params); 
	check(params, expect); 
}

void OrientationTestFixture::check(const CDoubleVector& params, const vector<double>& expect)
{
	for (int i = 0; i < 16; ++i) {
		cvdebug() << "[" << i << "]=" <<  params[i] << ", expect " << expect[i] << "\n"; 
		if (expect[i] == 0.0) {
			BOOST_CHECK_SMALL(params[i], 1e-5); 
		}else{
			BOOST_CHECK_CLOSE(params[i], expect[i], 1e-5);
		}
	}
}
