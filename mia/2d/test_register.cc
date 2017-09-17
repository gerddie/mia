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

#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/2d/register.hh>

struct SimpleRegistrationFixture {
	
	AffineRegistrationFixture(); 

	
	void Initialize(const std::string& type);

	P2DTransformationFactory trans_factory;
	size_t start_size; 
	size_t max_iter; 
}; 

const float *src_init = {
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0	
}; 

// How to test registration creation 
BOOST_AUTO_TEST_CASE( test_simple_registration )  
{
	C2DMultiImageRegister registration(start_size, max_iter,
					   C2DRegModel& model, 
					   C2DRegTimeStep& time_step, 
					   P2DTransformationFactory& trans_factory, 
					   float outer_epsilon); 
	
	C2DBounds size(32, 32);

	

	C2DFImage src(size);
	C2DFImage ref(size);
	
	
	

}



AffineRegistrationFixture::AffineRegistrationFixture():
	start_size(16), 
	max_iter(10)
{
}

void AffineRegistrationFixture::Initialize(const std::string& type)
{
	trans_factory = C2DTransformCreatorHandler::instance().produce(type); 

}
