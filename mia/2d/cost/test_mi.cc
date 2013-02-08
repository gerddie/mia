/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
#include <mia/core/noisegen.hh>
#include <mia/2d/interpolator.hh>
#include <mia/2d/cost/mi.hh>
#include <cmath>


NS_MIA_USE
using namespace std;
using namespace ::boost::unit_test;
using namespace mia_2dcost_mi;

CSplineKernelTestPath spline_kernel_init_path; 
CNoiseGeneratorPluginHandlerTestPath noise_kernel_init_path; 

class MIFixture {
protected: 
	MIFixture(); 
	
	
	C2DBounds size; 
	C2DFImage *src_p; 
	C2DFImage *ref_p; 
	P2DImage src; 
	P2DImage ref; 
	C2DFVectorfield grad; 
	unique_ptr<C2DMIImageCost> cost; 
}; 


BOOST_FIXTURE_TEST_CASE( test_MI_2D_self, MIFixture )
{
	cost->set_reference(*ref);
	
	// where did this come from? 
	const double test_cost_value =-1.3245186154149495; 

	double cost_value = cost->value(*ref);
	BOOST_CHECK_CLOSE(cost_value, test_cost_value, 0.1);

	C2DFVectorfield force(C2DBounds(8,8));

	BOOST_CHECK_CLOSE(cost->evaluate_force(*ref, 0.5, force), 0.5 * test_cost_value, 0.1);

	BOOST_CHECK_EQUAL(force(3,3).x, 0.0f);
	BOOST_CHECK_EQUAL(force(3,3).y, 0.0f);
	
}



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

static double evaluate_mi_direct_256_256(const C2DFImage& a, const C2DFImage& b) 
{


	vector <long> hist_a(256, 0); 
	vector <long> hist_b(256, 0); 
	vector <long> hist_x(256 * 256, 0);

	// fill histogram 
	for (auto ia = a.begin(), ib= b.begin(); ia != a.end(); ++ia, ++ib) {
		int va = *ia < 0 ? 0 : (*ia > 255 ? 255 : static_cast<int>(*ia)); 
		int vb = *ib < 0 ? 0 : (*ib > 255 ? 255 : static_cast<int>(*ib)); 
		
		++hist_a[va]; 
		++hist_b[vb]; 
		++hist_x[va + 256 * vb]; 
	}

	return entr(hist_b, a.size()) + entr(hist_a, a.size()) - entr(hist_x, a.size()); 

}

BOOST_FIXTURE_TEST_CASE( test_MI_2D, MIFixture )
{
	const double expect_cost_value = evaluate_mi_direct_256_256(*src_p, *ref_p); 
	cost->set_reference(*ref);
	
	
	double cost_value = cost->value(*src);
	BOOST_CHECK_CLOSE(cost_value, expect_cost_value, 0.1);
	C2DFVectorfield force(C2DBounds(8,8));
	BOOST_CHECK_CLOSE(cost->evaluate_force(*src, 1.0, force), expect_cost_value, 0.1);

}


BOOST_AUTO_TEST_CASE( test_MI_2D_big )
{
	C2DBounds size(256,256); 
	
	C2DFImage reference(size); 
	C2DFImage moving(size);

	auto uniform = CNoiseGeneratorPluginHandler::instance().produce("uniform:a=0,b=255,seed=1"); 
	auto gauss = CNoiseGeneratorPluginHandler::instance().produce("gauss:mu=10,sigma=10,seed=1");

	// fill the image with random data 
	for (auto ir = reference.begin(), im = moving.begin(); ir != reference.end(); ++ir, ++im) {
		*ir = (*uniform)(); 
		*im  = (*gauss)() + *ir;
		if (*im < 0) 
			*im = 0; 
		if (*im > 255) 
			*im = 255; 
	}
	
	C2DFVectorfield force(size);

	C2DMIImageCost cost(256, produce_spline_kernel("bspline:d=0"), 
			    256, produce_spline_kernel("bspline:d=2"), 0); 

	cost.set_reference(reference);

	C2DInterpolatorFactory  ipf("bspline:d=3", "zero"); 
	unique_ptr<T2DInterpolator<float>> src_ip(ipf.create(moving.data()));
	
	double test_cost = evaluate_mi_direct_256_256(moving, reference); 
	double c = cost.evaluate_force(moving, 1.0, force);
	
	BOOST_CHECK_CLOSE(c, test_cost, 0.1); 
	cvdebug() << "test_cost/c=" << test_cost/c << "\n"; 


	auto ig = force.begin(); 

	for (unsigned int y = 0; y < size.y; ++y) 
		for (unsigned int x = 0; x < size.y; ++x,  ++ig)  {
			const float srcpixel = moving(x,y); 
			
			const C2DFVector xm(x - 1, y); 
			const C2DFVector xp(x + 1, y); 
                        moving(x,y) = (*src_ip)(xm); 
			const float fxm = evaluate_mi_direct_256_256(moving, reference);
			moving(x,y) = (*src_ip)(xp); 
                        const float fxp = evaluate_mi_direct_256_256(moving, reference);
			float gx = 0.5 * (fxp - fxm); 
			cvdebug() << "(" << x << ", " << y << ") -> " 
				  << setw(10) << fxm << " " << setw(10) << fxp 
				  << ":" << setw(10) << gx
				  << "\n"; 

			BOOST_CHECK_CLOSE(ig->x, gx, 0.1); 

			const C2DFVector ym(x, y - 1); 
			const C2DFVector yp(x, y + 1); 
			
                        moving(x,y) = (*src_ip)(ym); 
			const float fym = evaluate_mi_direct_256_256(moving, reference);
			moving(x,y) = (*src_ip)(yp); 
                        const float fyp = evaluate_mi_direct_256_256(moving, reference);
			float gy = 0.5 * (fyp - fym);
			BOOST_CHECK_CLOSE(ig->y, gy, 0.1); 
			moving(x,y) = srcpixel; 

			cvinfo() << "factor = "<< setw(3) << x << setw(4) << y << ": "
				 << setw(10) << ig->x / gx << "   " 
				 << setw(10) << ig->y / gy << "\n";  

		}




}

MIFixture::MIFixture():
	size(8,8), 
	grad(size)
{
	const float src_data[64] = {           /*   0  1  2  3  4  5  6  7  8  9     */    
		1, 1, 2, 2, 2, 3, 4, 4,        /*1     2  2  1  2  1               8 */
		4, 4, 3, 3, 2, 2, 2, 1,        /*2     1  3  1  2     1            8 */
		2, 2, 3, 4, 5, 6, 7, 8,        /*3        2  2     1  1  1  1      8 */
		8, 7, 2, 8, 3, 4, 2, 2,        /*4     1     1  1     2  1  2      8 */ 
		3, 1, 3, 4, 5, 6, 7, 8,        /*5     1     3  1        2  1      8 */
		3, 4, 4, 5, 6, 4, 2, 2,        /*6        3  1  3     1            8 */
	        3, 2, 3, 4, 5, 3, 1, 4,        /*7     1  1  3  2  1               8 */
		5, 6, 7, 3, 2, 1, 2, 6         /*8     1  2  1     1  2  1         8 */
	};                                     /*      7 13 13 11  4  7  5  3        */
	
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

	const float grady[64] = {
		+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000,
		+0.00000, 0.00000,-1.30000, 0.00000,-0.35294,-0.35294,-0.35294, 0.00000,
		-0.00000,-0.70588, 0.68182, 0.00000, 0.16667, 3.76471, 0.00000, 0.00000,
		+0.00000,-1.66667,-0.00000, 0.00000,-0.00000, 0.00000,-0.00000,-0.00000,
		+0.00000,-0.00000,-1.36364,-0.00000, 0.50000, 0.00000, 0.00000, 0.00000,
		+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.70588, 0.00000,
		-0.00000,-0.23529,-2.04545, 0.00000,-0.66667, 2.04545, 0.00000,-0.00000,
		+0.00000,+0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000
	}; 
	const float gradx[64] = {
		+ 0.00000,  0.00000, 0.00000, 0.00000, 0.00000, 0.00000,-0.00000,-0.00000, 
		+ 0.00000,- 0.00000, 1.30000,-0.00000, 0.11765,-0.00000, 0.11765,-0.00000,
		- 0.00000,- 0.11765,-1.36364, 0.00000, 0.33333, 3.76471, 1.40000, 0.00000, 
		+ 0.00000,-11.66667, 0.23529, 1.81818, 3.33333,-0.00000, 0.23529,-0.00000, 
		+ 0.00000,  0.00000,-2.04545, 0.00000, 0.33333, 0.40000, 0.00000, 0.00000,
	        - 0.00000,  0.00000, 0.00000, 0.33333,-0.20000, 0.00000, 0.23529,-0.00000,
		- 0.00000,- 0.35294,-1.36364,-0.00000,-0.16667, 2.72727, 0.00000,-0.00000,
		+ 0.00000,  0.00000,-0.00000, 0.00000, 0.00000, 0.00000, 0.00000, 0.00000
	}; 

	src.reset(src_p = new C2DFImage(size, src_data ));
	ref.reset(ref_p = new C2DFImage(size, ref_data ));

	auto ig = grad.begin(); 
	for (int i = 0; i < 64; ++i, ++ig) {
		ig->x = gradx[i]; 
		ig->y = grady[i];
	}
	cost.reset(new 	C2DMIImageCost(8, produce_spline_kernel("bspline:d=0"), 
				       8, produce_spline_kernel("bspline:d=3"), 0)); 
}
