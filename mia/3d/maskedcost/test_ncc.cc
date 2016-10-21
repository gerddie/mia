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
#include <mia/3d/maskedcost/ncc.hh>
#include <mia/core/nccsum.hh>

using namespace NS; 
using namespace mia; 




BOOST_AUTO_TEST_CASE( test_masked_ncc_zero ) 
{
        C3DBounds size(4,4,4); 
        auto ncc = BOOST_TEST_create_from_plugin<CNCC3DImageCostPlugin>("ncc");
        
        C3DFImage a(size); 
        C3DFImage b(size); 
        C3DBitImage mask(size); 

        fill(mask.begin(), mask.end(), true); 
        
        auto ia = a.begin_range(C3DBounds::_0, size); 
        auto ib = b.begin_range(C3DBounds::_0, size); 
        auto ea = a.end_range(C3DBounds::_0, size); 

        while (ia != ea) {
                *ia = ia.pos().x + ia.pos().y + ia.pos().z; 
                *ib = 2* *ia; 
                ++ib; ++ia; 
        }

        ncc->set_reference(b); 
        BOOST_CHECK_SMALL(ncc->value(b, mask), 1e-7); 

        BOOST_CHECK_SMALL(ncc->value(a, mask), 1e-7); 
        
        
}

BOOST_AUTO_TEST_CASE( test_masked_ncc_nonzero ) 
{
        C3DBounds size(4,4,4); 
        auto ncc = BOOST_TEST_create_from_plugin<CNCC3DImageCostPlugin>("ncc");
        
        C3DFImage a(size); 
        C3DFImage b(size); 
        C3DBitImage mask(size); 

        fill(mask.begin(), mask.end(), true); 
        
        auto ia = a.begin_range(C3DBounds::_0, size); 
        auto ib = b.begin_range(C3DBounds::_0, size); 
        auto ea = a.end_range(C3DBounds::_0, size); 

        NCCSums sum; 

        while (ia != ea) {
                *ia = ia.pos().x + ia.pos().y + ia.pos().z; 
                *ib = ia.pos().x + ia.pos().y * ia.pos().z; 
                sum.add(*ia, *ib); 
                ++ib; ++ia; 
        }

        ncc->set_reference(b); 
        BOOST_CHECK_CLOSE(ncc->value(a, mask), sum.value(), 0.1); 
        
        C3DFVectorfield grad(size); 

        BOOST_CHECK_CLOSE(ncc->evaluate_force(a, mask, grad), sum.value(), 0.01);


	auto ag = get_gradient(a); 
	
	auto iag = ag.begin(); 
	auto ia2 = a.begin(); 
	auto ib2 = b.begin(); 
	auto iforce = grad.begin(); 
	auto eforce = grad.end(); 

	auto gs = sum.get_grad_helper().second; 
	while (iforce != eforce) {
		auto expect = *iag * gs.get_gradient_scale(*ia2, *ib2); 
		BOOST_CHECK_CLOSE(iforce->x, expect.x, 0.01); 
		BOOST_CHECK_CLOSE(iforce->y, expect.y, 0.01); 
		BOOST_CHECK_CLOSE(iforce->z, expect.z, 0.01); 
		++iag; ++ia2; ++ib2; ++iforce;
	}
}


BOOST_AUTO_TEST_CASE( test_masked_ncc_nonzero_with_mask ) 
{
        C3DBounds size(4,4,4); 
        auto ncc = BOOST_TEST_create_from_plugin<CNCC3DImageCostPlugin>("ncc");
        
        C3DFImage a(size); 
        C3DFImage b(size); 
        C3DBitImage mask(size); 

        fill(mask.begin(), mask.end(), true); 
	mask(2,3,1) = false; 
	mask(1,2,1) = false; 
        
        auto ia = a.begin_range(C3DBounds::_0, size); 
        auto ib = b.begin_range(C3DBounds::_0, size); 
        auto im = mask.begin_range(C3DBounds::_0, size); 
        auto ea = a.end_range(C3DBounds::_0, size); 

        NCCSums sum; 

        while (ia != ea) {
		if (*im) {
			*ia = ia.pos().x + ia.pos().y + ia.pos().z; 
			*ib = ia.pos().x + ia.pos().y * ia.pos().z; 
			sum.add(*ia, *ib); 
		}
                ++ib; ++ia; ++im; 
        }

        ncc->set_reference(b); 
        BOOST_CHECK_CLOSE(ncc->value(a, mask), sum.value(), 0.1); 
        
        C3DFVectorfield grad(size); 

        BOOST_CHECK_CLOSE(ncc->evaluate_force(a, mask, grad), sum.value(), 0.01);


	auto ag = get_gradient(a); 
	
	auto iag = ag.begin(); 
	auto ia2 = a.begin(); 
	auto ib2 = b.begin(); 
	auto im2 = mask.begin(); 
	auto iforce = grad.begin(); 
	auto eforce = grad.end(); 

	auto gs = sum.get_grad_helper().second; 
	while (iforce != eforce) {
		if (*im2) {
			auto expect = *iag * gs.get_gradient_scale(*ia2, *ib2); 
			BOOST_CHECK_CLOSE(iforce->x, expect.x, 0.01); 
			BOOST_CHECK_CLOSE(iforce->y, expect.y, 0.01); 
			BOOST_CHECK_CLOSE(iforce->z, expect.z, 0.01); 
		}else{
			BOOST_CHECK_SMALL(iforce->x, 1e-8f); 
			BOOST_CHECK_SMALL(iforce->y, 1e-8f); 
			BOOST_CHECK_SMALL(iforce->z, 1e-8f); 
		}
		++iag; ++ia2; ++ib2; ++iforce; ++im2; 
	}
}

