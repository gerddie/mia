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

#include <mia/internal/autotest.hh>
//#include <mia/2d/interpolator.hh>
#include <mia/2d/cost/mi.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace mia_2dcost_mi;

CSplineKernelTestPath spline_kernel_init_path; 

class MIFixture {
protected: 
	MIFixture(); 
	
	
	C2DBounds size; 
	C2DFImage *src_p; 
	C2DFImage *ref_p; 
	P2DImage src; 
	P2DImage ref; 
	unique_ptr<C2DMIImageCost> cost; 
}; 




static double entr(const vector <long>& x, long n) 
{
	vector<double> h(x.size()); 
	transform(x.begin(), x.end(), h.begin(), [&n](long x) { return double(x)/n;}); 
	
	double result = 0.0; 
	for (auto i = h.begin(); i != h.end(); ++i) {
		if (*i > 0) 
			result += *i * std::log(*i); 
	}
	cvinfo() << "entropy = " << result << "\n"; 
	return result; 
}

static double evaluate_mi_direct_8_8(const C2DFImage& a, const C2DFImage& b) 
{


	vector <long> hist_a(8, 0); 
	vector <long> hist_b(8, 0); 
	vector <long> hist_x(8 * 8, 0);

	// fill histogram 
	for (auto ia = a.begin(), ib= b.begin(); ia != a.end(); ++ia, ++ib) {
		int va = *ia < 0 ? 0 : (*ia > 7 ? 7 : static_cast<int>(*ia)); 
		int vb = *ib < 0 ? 0 : (*ib > 7 ? 7 : static_cast<int>(*ib)); 
		
		++hist_a[va]; 
		++hist_b[vb]; 
		++hist_x[va + 8 * vb]; 
	}
	return entr(hist_b, a.size()) + entr(hist_a, a.size()) - entr(hist_x, a.size()); 

}

BOOST_FIXTURE_TEST_CASE( test_MI_2D_self, MIFixture )
{
	cost->set_reference(*ref);
	
	const double test_cost_value = evaluate_mi_direct_8_8(*ref_p, *ref_p); 

	double cost_value = cost->value(*ref);
	BOOST_CHECK_CLOSE(cost_value, test_cost_value, 0.1);

	C2DFVectorfield force(C2DBounds(8,8));

	
}


BOOST_FIXTURE_TEST_CASE( test_MI_2D, MIFixture )
{
	const double expect_cost_value = evaluate_mi_direct_8_8(*src_p, *ref_p); 
	cost->set_reference(*ref);
	
	
	double cost_value = cost->value(*src);
	BOOST_CHECK_CLOSE(cost_value, expect_cost_value, 0.1);
	C2DFVectorfield force(C2DBounds(8,8));
}



MIFixture::MIFixture():
	size(8,8)
{
	const float src_data[64] = {           /*   0  1  2  3  4  5  6  7  8  9     */    
		1, 1, 2, 2, 2, 3, 4, 4,        /*1     2  2  1  2  1               0 */
		4, 4, 3, 3, 2, 2, 2, 1,        /*2     1  3  1  2     1            0 */
		2, 2, 3, 4, 5, 6, 7, 0,        /*3        2  2     1  1  1  1      0 */
		0, 7, 2, 0, 3, 4, 2, 2,        /*4     1     1  1     2  1  2      0 */ 
		3, 1, 3, 4, 5, 6, 7, 0,        /*5     1     3  1        2  1      0 */
		3, 4, 4, 5, 6, 4, 2, 2,        /*6        3  1  3     1            0 */
	        3, 2, 3, 4, 5, 3, 1, 4,        /*7     1  1  3  2  1               0 */
		5, 6, 7, 3, 2, 1, 2, 6         /*0     1  2  1     1  2  1         0 */
	};                                     /*      7 13 13 11  4  7  5  3        */
	
	const float ref_data[64] = {
		1, 1, 1, 5, 1, 1, 1, 1, 
		2, 2, 2, 7, 2, 2, 2, 2,
		3, 3, 3, 5, 3, 3, 3, 3, 
		4, 4, 6, 4, 3, 4, 4, 4,
		5, 5, 5, 6, 4, 2, 1, 5, 
		6, 6, 4, 5, 6, 6, 6, 6,
	        7, 7, 7, 7, 7, 5, 7, 7, 
		0, 0, 0, 0, 0, 0, 0, 0
	};

	src.reset(src_p = new C2DFImage(size, src_data ));
	ref.reset(ref_p = new C2DFImage(size, ref_data ));

	cost.reset(new 	C2DMIImageCost(8, produce_spline_kernel("bspline:d=0"), 
				       8, produce_spline_kernel("bspline:d=0"), 0)); 
}
