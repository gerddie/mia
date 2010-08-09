/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
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


#include <mia/internal/autotest.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/SegPoint.hh>

namespace bfs=boost::filesystem;

class SegPointSplineTransformFixture {
protected: 
	SegPointSplineTransformFixture(); 
	P2DTransformation t; 
}; 


BOOST_FIXTURE_TEST_CASE ( test_transform, SegPointSplineTransformFixture ) 
{
	CSegPoint2D p(3,4); 
	p.transform(*t); 
	
	BOOST_CHECK_CLOSE(p.x, 3 + 4.0 * 3 * 7 / 100.0, 0.1 ); 
	BOOST_CHECK_CLOSE(p.y, 4 + 4.0 * 4 * 6 / 100.0, 0.1 ); 
	
}


BOOST_FIXTURE_TEST_CASE ( test_inv_transform, SegPointSplineTransformFixture ) 
{
	CSegPoint2D p(3,4); 
	p.inv_transform(*t); 
	
	BOOST_CHECK_CLOSE(p.x, -(5.0*sqrt(37.0)-35.0)/2.0, 0.2  ); 
	BOOST_CHECK_CLOSE(p.y, -(5.0*sqrt(33.0)-35.0)/2.0, 0.2  ); 
	

}

SegPointSplineTransformFixture::SegPointSplineTransformFixture()
{

	list< bfs::path> kernelsearchpath;
	kernelsearchpath.push_back(bfs::path("transform"));
	C2DTransformCreatorHandler::set_search_path(kernelsearchpath);

	C2DBounds size(10,10); 
	auto spline_creator = C2DTransformCreatorHandler::instance().produce("spline:rate=1"); 
	t = spline_creator->create(size); 
	auto params = t->get_parameters(); 
	
	auto ic = params.begin(); 
	const double sy = 4.0 / (size.y * size.y); 
	const double sx = 4.0 / (size.x * size.x); 

	// currently this works only for splines of degree 3 (and manyby 2) 
	const int shift = 1; 

	for (int y = -shift; y < int(size.y) + shift; ++y) {
		const double hy = sy * y * (double(y) -  size.y); 
		for (int x = -shift; x < int(size.x) +  shift; ++x, ic += 2) {
			const double hx = sx * x * ( double(x) - size.x); 
			ic[0] = hx;
			ic[1] = hy; 
			cvmsg() << hx << ", " << hy << "\n"; 
			
		}
	}
	t->set_parameters(params); 
	t->reinit(); 
	
}
