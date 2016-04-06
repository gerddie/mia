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
#include <mia/3d/filter/combiner.hh>

#include <mia/core/datapool.hh>
#include <mia/3d/imageio.hh>
#include <mia/3d/imagetest.hh>

NS_MIA_USE

struct Combiner3DFilterFixture {

        Combiner3DFilterFixture(); 
        ~Combiner3DFilterFixture();        
        C3DFImage *image1; 
        C3DFImage *image2; 
        
        P3DImage pimage1;
        P3DImage pimage2; 
        C3DBounds size; 
};


BOOST_FIXTURE_TEST_CASE(test_sub_default, Combiner3DFilterFixture) 
{
        auto c = BOOST_TEST_create_from_plugin<C3DImageCombinerFilterPlugin>("combiner:image=other.@,op=sub");


        auto result = c->filter(pimage1); 
        BOOST_REQUIRE(result); 

        C3DFImage expect(size); 
        transform(image1->begin(), image1->end(), image2->begin(), expect.begin(), 
                  [](float a, float b){return a-b;}); 


        test_image_equal(*result, expect);

}

BOOST_FIXTURE_TEST_CASE(test_sub_reverse, Combiner3DFilterFixture) 
{
        auto c = BOOST_TEST_create_from_plugin<C3DImageCombinerFilterPlugin>("combiner:image=other.@,op=sub,reverse=1");

        auto result = c->filter(pimage1); 
        BOOST_REQUIRE(result); 

        C3DFImage expect(size); 
        transform(image2->begin(), image2->end(), image1->begin(), expect.begin(), 
                  [](float a, float b){return a-b;}); 

        test_image_equal(*result, expect);
}

Combiner3DFilterFixture::Combiner3DFilterFixture():
        size(3,4,2)
{
        image2 = new  C3DFImage(size); 
        pimage2.reset(image2); 

        float x = 0.1; 
        for (auto i = image2->begin(); i != image2->end(); ++i, x += 0.2)
                *i = x; 

        image1 = new  C3DFImage(size); 
        pimage1.reset(image1); 

        x = 0.2; 
        for (auto i = image1->begin(); i != image1->end(); ++i, x += 0.3)
                *i = x; 


        save_image("other.@", pimage2); 

}

Combiner3DFilterFixture::~Combiner3DFilterFixture()
{
        boost::any dummy = CDatapool::instance().get_and_remove("other.@");
}











