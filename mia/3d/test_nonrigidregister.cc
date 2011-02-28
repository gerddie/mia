/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

#define VSTREAM_DOMAIN "NR-TEST"

#include <cmath>
#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/datapool.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/3d/nonrigidregister.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/cost.hh>

NS_MIA_USE
using namespace std;
namespace bfs=boost::filesystem;

BOOST_AUTO_TEST_CASE ( test_nothing ) 
{
	BOOST_MESSAGE("This is a placeholder"); 
}

#if 0

class C3DFullCostMock: public C3DFullCost {
public: 
	C3DFullCostMock(double weight, const C3DBounds& size); 

	double fx(double x, double y, double z); 
	double fy(double x, double y, double z); 
	double fz(double x, double y, double z); 

private:
	double do_evaluate(const C3DTransformation& t, gsl::DoubleVector& gradient) const;
	double do_value(const C3DTransformation& t) const;
	void do_set_size(); 


	C3DBounds _M_real_size; 
	C3DFVector _M_scale; 
}; 



BOOST_AUTO_TEST_CASE ( test_nonrigid ) 
{
	const int mg_levels = 1; 
	C3DBounds size(32, 32, 32); 
	C3DFullCostMock *mcost = new C3DFullCostMock(1.0, size); 
	P3DFullCost pcost(mcost); 
	C3DFullCostList costs; 
	costs.push(pcost); 

	auto transform_creator = C3DTransformCreatorHandler::instance().produce("spline:rate=4"); 
	EMinimizers minimizer = min_gd;
	
	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(ip_bspline3));
	C3DNonrigidRegister nrr(costs, minimizer,  transform_creator, *ipfactory);

	P3DImage Model(new C3DUBImage(size));
	P3DImage Reference(new C3DUBImage(size));
	
	P3DTransformation transform = nrr.run(Model, Reference, mg_levels);
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x) {
				C3DFVector v = transform->apply(C3DFVector(x,y)); 
				
				BOOST_CHECK_CLOSE(v.x, mcost->fx(x,y,z), 0.1); 
				BOOST_CHECK_CLOSE(v.y, mcost->fy(x,y,z), 0.1); 
				BOOST_CHECK_CLOSE(v.z, mcost->fz(x,y,z), 0.1); 
			}
}

C3DFullCostMock::C3DFullCostMock(double weight, const C3DBounds& size):
	C3DFullCost(weight), 
	_M_real_size(size)
{
}

double C3DFullCostMock::do_evaluate(const C3DTransformation& t, gsl::DoubleVector& gradient) const
{
	const C3DBounds& size = get_current_size(); 
	BOOST_REQUIRE(size == t.get_size());
	
	double result = 0.0; 
	auto it = t.begin(); 

	C3DFVector v; 
	auto g = gradient.begin(); 
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, g += 2, ++it) {
				v.x = it->x - fx(x,y,z); 
				v.y = it->y - fy(x,y,z); 
				v.z = it->z - fz(x,y,z); 
				result += v.norm2(); 
				C3DFMatrix dg = t.derivative_at(x, y, z);
				C3DFVector h = dg * v; 
				g[0] = h.x;
				g[1] = h.y;
				g[2] = h.z;
				
			}
	return result; 
}

double C3DFullCostMock::do_value(const C3DTransformation& t) const
{
	double result = 0.0; 
	auto it = t.begin(); 

	C3DFVector v;
	for (size_t z = 0; z < size.z; ++z) 
		for (size_t y = 0; y < size.y; ++y) 
			for (size_t x = 0; x < size.x; ++x, ++it) {
				
				v.x = it->x - (x - fx(x,y,z)); 
				v.y = it->y - (y - fy(x,y,z)); 
				v.z = it->z - (z - fz(x,y,z)); 
				result += v.norm2(); 
				
			}
	return result; 
	
}

void C3DFullCostMock::do_set_size()
{
	_M_scale.x = float(_M_real_size.x) / get_current_size().x; 
	_M_scale.y = float(_M_real_size.y) / get_current_size().y; 
	_M_scale.z = float(_M_real_size.z) / get_current_size().z; 
}

double C3DFullCostMock::fx(double x, double y, double z)
{
	return 2 * sin(M_PI * _M_scale.x * x / _real_size.x); 
}

double C3DFullCostMock::fy(double x, double y, double z)
{
	return sin(2 * M_PI * _M_scale.y * y / _real_size.y); 
}

double C3DFullCostMock::fz(double x, double y, double z)
{
	return 1.5 * sin(2 * M_PI * _M_scale.z * z / _real_size.z); 
}


#endif 
