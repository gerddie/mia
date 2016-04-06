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

#include <mia/internal/plugintester.hh> 
#include <mia/2d/fullcost/label.hh>
#include <mia/2d/transformfactory.hh>

NS_MIA_USE


struct TransformInitFixture {
	TransformInitFixture(); 

	P2DTransformationFactory tff; 
}; 


BOOST_FIXTURE_TEST_CASE ( test_labeldistance, TransformInitFixture ) 
{
        // create two images and do the thing 
        
        const unsigned char ref_data[25] = {
                1, 1, 1, 2, 2, 
                1, 1, 1, 2, 2, 
                1, 1, 1, 2, 2, 
                5, 5, 5, 2, 2, 
                5, 5, 2, 2, 2, 
	}; 

        const unsigned char mov_data[25] = {
                1, 1, 2, 2, 2, 
                1, 1, 2, 2, 2, 
                1, 1, 2, 2, 2, 
                1, 1, 5, 2, 2, 
                5, 5, 5, 5, 2, 
	}; 

        const float distances [25] = {
                0, 0, 1, 0,         0, 
                0, 0, 1, 0,         0, 
                0, 0, 1, 0,         0, 
                1, 1, 0, 0,         0, 
                0, 0, 1, sqrtf(2.0f), 0,
        }; 
        
	const float gradx [25] = {
		0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0, 0, 0, 
                0, 0, -0.5f * sqrtf(2.0f), -0.5f * (sqrtf(5.0f) - 1.0f), 0 
	}; 

        const float grady [25] = {
                0, 0,  0, 0, 0, 
                0, 0,  0, 0, 0, 
		0, 0,  0, 0, 0, 
                -1, -1,  0, 0, 0, 
		0, 0,-.5f, -.5f * (sqrtf(2.0f) - 1.0f), 0 
        }; 

        const C2DBounds size(5,5);
        
        C2DUBImage *mov_image = new C2DUBImage(size, mov_data );
        C2DUBImage *ref_image = new C2DUBImage(size, ref_data );

        P2DImage mov(mov_image); 
        P2DImage ref(ref_image);
        
        auto cost = BOOST_TEST_create_from_plugin<C2DLabelFullCostPlugin>("labelimage:maxlabel=7"); 
        
        auto t = tff->create(size); 
	auto params = t->get_parameters(); 
	std::fill(params.begin(), params.end(), 0.0); 
	t->set_parameters(params); 
        
        save_image("src.@", mov); 
        save_image("ref.@", ref); 

        cost->reinit();
        cost->set_size(size); 
        
        BOOST_CHECK_CLOSE(cost->cost_value(*t), 6 + sqrt(2.0), 0.01); 
        BOOST_CHECK_CLOSE(cost->cost_value(), 6 + sqrt(2.0), 0.01); 
        
	CDoubleVector gradient(t->degrees_of_freedom()); 
	cost->evaluate(*t, gradient);

	for(int i = 0; i < 25; ++i) {
		cvdebug() << "[" << i << "]: (" << gradient[2*i] << ", " << gradient[2*i+1]
			  << ") expect ("<< gradx[i] << ", " << grady[i] << ")\n"; 

		if (gradx[i] != 0.0) 
			BOOST_CHECK_CLOSE(gradient[2*i], gradx[i], 0.1); 
		else 
			BOOST_CHECK_SMALL(gradient[2*i], 1e-10); 
		if (grady[i] != 0.0) 
			BOOST_CHECK_CLOSE(gradient[2*i+1], grady[i], 0.1);
		else 
			BOOST_CHECK_SMALL(gradient[2*i+1], 1e-10); 
	}
}



TransformInitFixture::TransformInitFixture():
        tff(C2DTransformCreatorHandler::instance().produce("vf:imgkernel=[bspline:d=0],imgboundary=zero"))
{
        
}
