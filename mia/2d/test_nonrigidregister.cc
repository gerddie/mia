/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#define VSTREAM_DOMAIN "NR-TEST"

#include <cmath>
#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/datapool.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/cost.hh>

NS_MIA_USE
using namespace std;
namespace bfs=boost::filesystem;

BOOST_AUTO_TEST_CASE ( test_nothing ) 
{
	// this "test" is just here to have something in the test tree
}

#if 0

class C2DFullCostMock: public C2DFullCost {
public: 
	C2DFullCostMock(double weight, const C2DBounds& size); 

	double fx(double x, double y); 
	double fy(double x, double y); 

private:
	double do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const;
	double do_value(const C2DTransformation& t) const;
	void do_set_size(); 


	C2DBounds m_real_size; 
	C2DFVector m_scale; 
}; 



BOOST_AUTO_TEST_CASE ( test_nonrigid ) 
{
	const int mg_levels = 1; 
	C2DBounds size(32, 32); 
	C2DFullCostMock *mcost = new C2DFullCostMock(1.0, size); 
	P2DFullCost pcost(mcost); 
	C2DFullCostList costs; 
	costs.push(pcost); 

	auto transform_creator = C2DTransformCreatorHandler::instance().produce("spline:rate=4"); 
	EMinimizers minimizer = min_gd;
	
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));
	C2DNonrigidRegister nrr(costs, minimizer,  transform_creator, *ipfactory);

	P2DImage Model(new C2DUBImage(size));
	P2DImage Reference(new C2DUBImage(size));
	
	P2DTransformation transform = nrr.run(Model, Reference, mg_levels);
	
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x) {
			C2DFVector v = transform->apply(C2DFVector(x,y)); 
			

			BOOST_CHECK_CLOSE(v.x, mcost->fx(x,y), 0.1); 
			BOOST_CHECK_CLOSE(v.y, mcost->fy(x,y), 0.1); 
		}
}

C2DFullCostMock::C2DFullCostMock(double weight, const C2DBounds& size):
	C2DFullCost(weight), 
	m_real_size(size)
{
}

double C2DFullCostMock::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	const C2DBounds& size = get_current_size(); 
	BOOST_REQUIRE(size == t.get_size());
	
	double result = 0.0; 
	auto it = t.begin(); 

	C2DFVector v; 
	auto g = gradient.begin(); 
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, g += 2, ++it) {
			v.x = it->x - fx(x,y); 
			v.y = it->y - fy(x,y); 
			result += v.norm2(); 
			C2DFMatrix dg = t.derivative_at(x, y);
			C2DFVector h = dg * v; 
			g[0] = h.x;
			g[1] = h.y;
			
		}
	return result; 
}


double C2DFullCostMock::do_value(const C2DTransformation& t) const
{
	double result = 0.0; 
	auto it = t.begin(); 

	C2DFVector v;
	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++it) {

			v.x = it->x - (x - fx(x,y)); 
			v.y = it->y - (y - fy(x,y)); 
			result += v.norm2(); 

			
		}
	return result; 
	
}

void C2DFullCostMock::do_set_size()
{
	m_scale.x = float(m_real_size.x) / get_current_size().x; 
	m_scale.y = float(m_real_size.y) / get_current_size().y; 
}

double C2DFullCostMock::fx(double x, double y)
{
	return 2 * sin(M_PI * m_scale.x * x / _real_size.x); 
}

double C2DFullCostMock::fy(double x, double y)
{
	return sin(2 * M_PI * m_scale.y * y / _real_size.y); 
}


#endif 
