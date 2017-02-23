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

#include <mia/internal/plugintester.hh> 
#include <mia/3d/fullcost/label.hh>
#include <mia/3d/transformfactory.hh>

NS_MIA_USE


struct TransformInitFixture {
	TransformInitFixture(); 

	P3DTransformationFactory tff; 
}; 


BOOST_FIXTURE_TEST_CASE ( test_labeldistance, TransformInitFixture ) 
{
        // create two images and do the thing 
        

        const unsigned char mov_data[75] = {
                1, 1, 2, 2, 2, 
		1, 1, 2, 2, 2, 
                1, 1, 1, 2, 2, 
                1, 1, 5, 2, 2, 
                5, 5, 5, 5, 2, 
		
		1, 1, 3, 2, 2, 
                1, 1, 2, 2, 2, 
                1, 1, 1, 2, 2, 
                1, 5, 5, 2, 2, 
                5, 5, 5, 5, 2, 

		1, 1, 2, 2, 2, 
                1, 1, 2, 2, 2, 
                1, 1, 2, 2, 2, 
                1, 1, 1, 2, 2, 
                5, 5, 5, 5, 2, 

	}; 

        const unsigned char ref_data[75] = {
                1, 1, 1, 2, 2,  1, 1, 1, 2, 2,  1, 1, 1, 2, 2,  5, 5, 5, 2, 2,  5, 5, 2, 2, 2, 
		1, 1, 3, 2, 2,  1, 1, 1, 2, 2,  1, 1, 1, 2, 2,  5, 5, 5, 2, 2,  5, 5, 2, 2, 2, 
		1, 1, 1, 2, 2,  1, 1, 1, 2, 2,  1, 1, 1, 2, 2,  5, 5, 2, 2, 2,  5, 5, 2, 2, 2, 
	}; 

#if 0 
        const float distances [75] = {
		0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0, 0, 0, 
                1, 1, 0, 0, 0, 
                0, 0, 1, sqrtf(2.0f), 0, 
		
		0, 0, 0, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0, 0, 0, 
                1, 0, 0, 0, 0, 
                0, 0, 1, sqrtf(2.0f), 0, 

		0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                1, 1, 1, 0, 0, 
                0, 0, 1, sqrtf(3.0f), 0

        }; 

#endif 
        const float gradz [75] = {
                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0,   0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0,   0, 0, 0, 0.5f * (sqrtf(2.0f) - sqrtf(3.0f)) , 0, 
                0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0, 0, 0, 0,   0, 0,  0, 0, 0,   0, 0, 0, 0.5f * (sqrtf(2.0f) - sqrtf(3.0f)), 0 

        }; 

        const float grady [75] = {
                0, 0,  0, 0, 0, 
                0, 0,  0, 0, 0, 
		0, 0,  0, 0, 0, 
                -1, -1,  0, 0, 0, 
		0, 0,-.5f, -.5f * (sqrtf(2.0f) - 1.0f), 0, 

                0, 0,  0, 0, 0, 
                0, 0,  0, 0, 0, 
		0, 0,  0, 0, 0, 
                -1,0,  0, 0, 0, 
		0, 0,-.5f, -.5f * (sqrtf(2.0f) - 1.0f), 0, 

                0, 0,  0, 0, 0, 
                0, 0,  0, 0, 0, 
		0, 0,  0.5f, 0, 0, 
                -1, -1, -1, 0, 0, 
		0, 0, 0, -.5f * (sqrtf(3.0f) - sqrtf(2.0f)), 0 

        }; 
        
	const float gradx [75] = {
		0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 
                0, 0, -0.5f * sqrtf(2.0f), -0.5f * (sqrtf(5.0f) - 1.0f), 0, 

		0, 0, 0, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0, 0, 0, 
                0, 0, 0, 0, 0, 
                0, 0, -0.5f * sqrtf(2.0f), -0.5f * (sqrtf(5.0f) - 1.0f), 0,

		0, 0, 1, 0, 0, 
                0, 0, 1, 0, 0, 
                0, 0, 0.5f * sqrtf(2.0f), 0, 0, 
                0, 0, -.5f * (sqrtf(2.0f) - 1.0f) , 0, 0, 
                0, 0, -0.5f * sqrtf(3.0f), -0.5f * (sqrtf(6.0f) - 1.0f), 0 

	}; 




        const C3DBounds size(5,5,3);
        
        
        C3DUBImage *mov_image = new C3DUBImage(size, mov_data );
        C3DUBImage *ref_image = new C3DUBImage(size, ref_data );

        P3DImage mov(mov_image); 
        P3DImage ref(ref_image);
        
        auto cost = BOOST_TEST_create_from_plugin<C3DLabelFullCostPlugin>("labelimage:maxlabel=7"); 
        
        auto t = tff->create(size); 
	auto params = t->get_parameters(); 
	std::fill(params.begin(), params.end(), 0.0); 
	t->set_parameters(params); 
        
        save_image("src.@", mov); 
        save_image("ref.@", ref); 

        cost->reinit();
        cost->set_size(size); 
        
        BOOST_CHECK_CLOSE(cost->cost_value(*t), 15 + 2 * sqrt(2.0) + sqrt(3.0), 0.01); 
        BOOST_CHECK_CLOSE(cost->cost_value(), 15 + 2 * sqrt(2.0) + sqrt(3.0), 0.01); 
        

	CDoubleVector gradient(t->degrees_of_freedom()); 
	double cost_value = cost->evaluate(*t, gradient);
        BOOST_CHECK_CLOSE(cost_value, 15 + 2 * sqrt(2.0) + sqrt(3.0), 0.01); 

	for(int i = 0; i < 75; ++i) {
		cvdebug() << "[" << i << "]: (" << gradient[3*i] << ", " << gradient[3*i+1] << ", " << gradient[3*i+2]
			  << ") expect ("<< gradx[i] << ", " << grady[i] << ", " << gradz[i] << ")\n"; 

		if (gradx[i] != 0.0) 
			BOOST_CHECK_CLOSE(gradient[3*i], gradx[i], 0.1); 
		else 
			BOOST_CHECK_SMALL(gradient[3*i], 1e-10); 

		if (grady[i] != 0.0) 
			BOOST_CHECK_CLOSE(gradient[3*i+1], grady[i], 0.1);
		else 
			BOOST_CHECK_SMALL(gradient[3*i+1], 1e-10); 

		if (gradz[i] != 0.0) 
			BOOST_CHECK_CLOSE(gradient[3*i+2], gradz[i], 0.1);
		else 
			BOOST_CHECK_SMALL(gradient[3*i+2], 1e-10); 


	}

}




TransformInitFixture::TransformInitFixture():
        tff(C3DTransformCreatorHandler::instance().produce("vf:imgkernel=[bspline:d=0],imgboundary=zero"))
{
        
}
