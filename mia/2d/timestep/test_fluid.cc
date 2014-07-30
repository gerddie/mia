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

#include <mia/internal/autotest.hh>
#include <mia/2d/timestep/fluid.hh>
#include <mia/2d/transformmock.hh>

NS_USE(fluid_timestep_2d);
NS_MIA_USE;

class C2DDummyTransformation: public C2DTransformMock {
	virtual C2DTransformation *clone() const {
		return new C2DDummyTransformation();
	}
	virtual bool save(const std::string& /*filename*/, const std::string& /*type*/) const {
		return false;
	}

	virtual P2DTransformation upscale(const C2DBounds& /*size*/) const {
		return P2DTransformation(new C2DDummyTransformation());
	}
	virtual void add(const C2DTransformation& /*a*/){
	}

	virtual void update(float /*step*/, const C2DFVectorfield& /*a*/) {
	}
	virtual size_t degrees_of_freedom() const {
		return 0;
	}
	virtual void set_identity(){
	}
	virtual C2DFMatrix derivative_at(int /*x*/, int /*y*/) const {
		return C2DFMatrix(C2DFVector(1,0), C2DFVector(0,1));
	}
	virtual float get_max_transform() const {
		return 0.0f;
	}
	virtual const C2DBounds& get_size() const {
		static C2DBounds s(0,0);
		return s;
	}
	virtual float pertuberate(C2DFVectorfield& v) const {
		C2DFVector v12(1,2); 
		transform(v.begin(), v.end(), v.begin(), [&v12](const C2DFVector& v) {return v + C2DFVector(1,2);});
		return v(0,0).norm();
	}
	virtual C2DFVector apply(const C2DFVector& x) const {
		return x;
	}
	virtual C2DFVector operator () (const C2DFVector& x) const {
		return x;
	}
	virtual float get_jacobian(const C2DFVectorfield& v, float delta) const {
		return  v(0,0).norm() / delta;
	}
	P2DImage apply(const C2DImage& image, const C2DInterpolatorFactory& /*ipf*/) const
	{
		return P2DImage(image.clone());
	}

	float divergence() const {return 0.0; }
	float curl() const {return 0.0; }

};

struct TimestepFixture {
	TimestepFixture();

	C2DBounds size;
	C2DFluidRegTimeStep plugin;
	C2DFVectorfield infield;
	C2DFVectorfield outfield;
};

TimestepFixture::TimestepFixture():
	size(1,1),
	plugin(0.5f, 1.5f),
	infield(size),
	outfield(size)
{
	infield(0,0) = C2DFVector(2.0, 2.0);
	outfield(0,0) =  C2DFVector(1.0, 0.0);
}

BOOST_FIXTURE_TEST_CASE( fluid_timestep_2d_calculate_pertuberation, TimestepFixture )
{
	C2DDummyTransformation dummy;
	BOOST_CHECK_EQUAL(plugin.calculate_pertuberation(infield, dummy), 5.0f);
	BOOST_CHECK_EQUAL(infield(0,0), C2DFVector(3.0, 4.0));
}


BOOST_FIXTURE_TEST_CASE( fluid_timestep_2d_regrid, TimestepFixture )
{
	C2DDummyTransformation dummy;
	BOOST_CHECK(plugin.has_regrid());
	BOOST_CHECK(!plugin.regrid_requested(dummy, infield, 3.0));
	BOOST_CHECK(plugin.regrid_requested(dummy, infield, 12.0));
	BOOST_CHECK(plugin.regrid_requested(dummy, outfield, 3.0));

}
