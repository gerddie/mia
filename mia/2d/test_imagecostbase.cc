/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2010, Gert Wollny
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
#include <mia/core/shared_ptr.hh>

#include <mia/2d/imagecostbase.hh>
#include <mia/2d/transformmock.hh>

struct C2DTestImageCost: public C2DImageCostBase {
	C2DTestImageCost(P2DImage src, P2DImage ref, 
			 P2DInterpolatorFactory ipf,
			 float weight): 
		C2DImageCostBase(src, ref, ipf, weight)
	{
	}
private: 
	double do_evaluate_with_images(const C2DImage& floating, const C2DImage& ref, 
				       C2DFVectorfield& force) const; 
	
}; 


BOOST_AUTO_TEST_CASE ( test_imagecostbase ) 
{
	const C2DBounds size(1,1); 
	P2DImage src(new C2DUBImage(size)); 
	P2DImage ref(new C2DUBImage(size)); 
	P2DInterpolatorFactory ipf(create_2dinterpolation_factory(ip_bspline3)); 
	C2DTestImageCost cost(src, ref, ipf, 1.0); 

	C2DFVectorfield force(size); 
	
	C2DTransformMock t(size); 

	
	BOOST_CHECK_EQUAL(cost.evaluate(t, force), 1.0);
	BOOST_CHECK_EQUAL(force(0,0), C2DFVector(2, 3));

}


double C2DTestImageCost::do_evaluate_with_images(const C2DImage& /*floating*/, const C2DImage& /*ref*/, 
						 C2DFVectorfield& force) const
{
	force(0,0) = C2DFVector(2.0, 3.0); 
	return 1.0;
}
