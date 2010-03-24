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

#include <cmath>
#include <mia/internal/autotest.hh>
#include <mia/2d/register.hh>

struct AffineRegistrationFixture {

	AffineRegistrationFixture(); 

	
	void Initialize(const std::string& type);

	P2DTransformationFactory trans_factory;
	size_t start_size; 
	size_t max_iter; 
}; 


// How to test registration creation 
BOOST_AUTO_TEST_CASE( test_affine_registration )  
{
	C2DMultiImageRegister registration(start_size, max_iter,
					   C2DRegModel& model, 
					   C2DRegTimeStep& time_step, 
					   P2DTransformationFactory& trans_factory, 
					   float outer_epsilon); 
	

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
