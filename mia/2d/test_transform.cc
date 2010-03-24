/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2009
 * Max-Planck-Institute for Human Cognitive and Brain Science	
 * Max-Planck-Institute for Evolutionary Anthropology 
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

#include <mia/2d/transform.hh>
#include <boost/test/auto_unit_test.hpp>

static void test_grid_transform() 
{
	P2DInterpolatorFactory ipf(create_2dinterpolation_factory("ip_nn"));
	
	C2DGridTransformation transform0(C2DBounds(0,0), ipf); 


	C2DBounds us_size(4,5); 
	C2DUBImage test_image(us_size); 
	


	C2DGridTransformation transform1 = transform0.upscale(us_size);
	BOOST_CHECK_EQUAL(us_size, transform1.get_size()); 

	C2DGridTransformation transform2(transform1.get_size(), ipf);
	
	
	
	
	

						   
	
	

}
