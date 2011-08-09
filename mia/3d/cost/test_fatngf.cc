/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <mia/internal/autotest.hh>
#include <mia/3d/cost/fatngf.hh>

using namespace std;
using namespace mia;
using namespace nfg_3dimage_fatcost; 

CSplineKernelTestPath init_path; 

BOOST_AUTO_TEST_CASE ( test_fatngf ) 
{
	const C3DBounds size(7,7,7);
	float init_ref[49] = { -1, 0, 1, 2, 1, 0, -1,
			       0, 1, 2, 3, 2, 1,  0,
			       1, 2, 3, 4, 3, 2,  1,
			       2, 3, 4, 5, 4, 3,  2,
			       1, 2, 3, 4, 3, 2,  1,
			       0, 1, 2, 3, 2, 1,  0,
			       -1, 0, 1, 2, 1, 0, -1
	};

	float init_src[49] = { 	 1, 1, 1, 1, 1, 1, 1,
				 1, 1, 1, 1, 3, 1, 1,
				 1, 1, 1, 3, 5, 3, 1,
				 1, 1, 1, 1, 3, 1, 1,
				 1, 1, 1, 1, 1, 1, 1,
				 1, 1, 1, 1, 1, 1, 1,
				 1, 1, 1, 1, 1, 1, 1
 	};

	C3DFImage *psrc = new C3DFImage(size); 
	C3DFImage *pref = new C3DFImage(size); 
	
	for (size_t i = 0; i < size.z; ++i) {
		copy(init_src, init_src + 49, psrc->begin_at(0,0,i)); 
		copy(init_ref, init_ref + 49, pref->begin_at(0,0,i)); 
	}

	P3DImage src(psrc);
	P3DImage ref(pref);

	C3DFVectorfield force_self(size);

	CFatNFG3DImageCost cost_self(ref, ref, 1.0, PEvaluator(new FDeltaScalar()));
	double cost_value_self = cost_self.evaluate_force(force_self);
	cvdebug() << "cost_value_self = " <<cost_value_self << "\n"; 
	BOOST_CHECK(fabs(cost_value_self) < 0.01); 
}

