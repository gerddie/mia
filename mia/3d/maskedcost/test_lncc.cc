/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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
#include <mia/3d/maskedcost/lncc.hh>

using namespace NS; 
using namespace mia; 




BOOST_AUTO_TEST_CASE( test_masked_lncc_1 ) 
{
        C3DBounds size(4,4,4); 
        auto lncc = BOOST_TEST_create_from_plugin<CLNCC3DImageCostPlugin>("lncc:w=1");

        	const float src_data[64] = {           /*   0  1  2  3  4  5  6  7  8  9     */    
		1, 1, 2, 2, 2, 3, 4, 4,        /*1                                 0 */
		4, 4, 3, 3, 2, 2, 2, 1,        /*2     1  2  1  1     1            6 */
		2, 2, 3, 4, 5, 6, 7, 8,        /*3        1  2     1  1  1  1      7 */
		8, 7, 2, 8, 3, 4, 2, 2,        /*4     1     1  1     2  1  1      7 */ 
		3, 1, 3, 4, 5, 6, 7, 8,        /*5     1     2  1        2  1      7 */
		3, 4, 4, 5, 6, 4, 2, 2,        /*6        3     3     1            7 */
	        3, 2, 3, 4, 5, 3, 1, 4,        /*7     1  1  2  2  1               7 */
		5, 6, 7, 3, 2, 1, 2, 6         /*8     1  2  1        2  1         7 */
	};                                     /*      5  9  9  8  2  7  5  3        */
	
	const float ref_data[64] = {
		1, 1, 1, 5, 1, 1, 1, 1, 
		2, 2, 2, 7, 2, 2, 2, 2,
		3, 3, 3, 5, 3, 3, 3, 3, 
		4, 4, 6, 4, 3, 4, 4, 4,
		5, 5, 5, 6, 4, 2, 1, 5, 
		6, 6, 4, 5, 6, 6, 6, 6,
	        7, 7, 7, 7, 7, 5, 7, 7, 
		8, 8, 8, 8, 8, 8, 8, 8
	};

	const bool mask_data[64] = {
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1,
	        0, 1, 1, 1, 1, 1, 1, 1, 
		0, 1, 1, 1, 1, 1, 1, 1
	};

        
	C3DFImage src_f(size, src_data); 
        C3DFImage ref_f(size, ref_data); 
        C3DBitImage mask(size, mask_data); 

        
        lncc->set_reference(ref_f); 
        BOOST_CHECK_CLOSE(lncc->value(ref_f, mask), -1, 0.1);
        
        auto v = lncc->value(src_f, mask); 
        BOOST_CHECK_CLOSE(v, -1, 0.1); 
        
        
}



