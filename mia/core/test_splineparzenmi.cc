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


#include <cmath>
#include <climits>
#include <iomanip>


#include <mia/internal/autotest.hh>
#include <mia/core/noisegen.hh>
#include <mia/core/mitestimages.hh>
#include <mia/core/splineparzenmi.hh>
#include <boost/filesystem.hpp>

NS_MIA_USE; 
using namespace std; 
using namespace boost::unit_test;
namespace bfs=boost::filesystem; 

CNoiseGeneratorPluginHandlerTestPath noise_kernel_init_path; 

struct SplineMutualInformationFixture  {
	SplineMutualInformationFixture();

	size_t size; 
	vector<double> reference;
	vector<double> moving;
	size_t bins; 
        PSplineKernel rkernel; 
        PSplineKernel mkernel; 
       

};


BOOST_FIXTURE_TEST_CASE( test_same_image_entropy, SplineMutualInformationFixture ) 
{
        PSplineKernel haar = CSplineKernelPluginHandler::instance().produce("bspline:d=0"); 
        CSplineParzenMI smi(256, haar, 256, haar, 0); 
	smi.fill(reference.begin(), reference.end(), reference.begin(), reference.end()); 
        BOOST_CHECK_CLOSE(smi.value(), -5.1013951881429653, 0.1); 
}


BOOST_FIXTURE_TEST_CASE( test_different_image_entropy, SplineMutualInformationFixture ) 
{
        PSplineKernel haar = CSplineKernelPluginHandler::instance().produce("bspline:d=0"); 
        CSplineParzenMI smi(256, haar, 256, haar,0); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	
        // this test value is not really evaluated ...
        BOOST_CHECK_CLOSE(smi.value(), -1.1739765269855784, 0.1); 
}


#ifdef THIS_TEST_FAILS
// either we interpre the paper wrongly, or this test can not pass
BOOST_FIXTURE_TEST_CASE( test_same_image_gradient_is_zero, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(128, mkernel, 256, mkernel); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	for (double m = 0; m < 256; m += 1)
		for (double r = 0; r < 256; r += 1)
			BOOST_CHECK_EQUAL(smi.get_gradient(m,r), 0.0); 

}
#endif

BOOST_FIXTURE_TEST_CASE( test_differnt_image_gradient_is_not_zero, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(bins, rkernel, bins, mkernel,0); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
        int cnt = 0; 
        for (double m = 0; m < 256; m += 1)
                for (double r = 0; r < 256; r += 1) {
                        if (smi.get_gradient(m,r) != 0.0)
                                ++cnt;
                }
        BOOST_CHECK(cnt > 0); 
        cvdebug() << "nozero gradient values =" << cnt <<"\n";
                
}


BOOST_FIXTURE_TEST_CASE( test_different_image_gradient, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(bins, rkernel, bins, mkernel,0); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
        int cnt = 0; 
        for (double m = 0; m < 256; m += 1)
                for (double r = 0; r < 256; r += 1) {
                        if (smi.get_gradient(m,r) != 0.0)
                                ++cnt; 
			cvdebug() << "( "
				  << setw(3) << m << ","
				  << setw(3) << r << ") = "
				  << smi.get_gradient(m,r) << "\n";
                }
                BOOST_CHECK(cnt > 0); 
        cvdebug() << "nozero gradient values =" << cnt <<"\n"; 
        
}
SplineMutualInformationFixture::SplineMutualInformationFixture():
        size(mi_test_size.width * mi_test_size.height), 
        reference(reference_init_data, reference_init_data + size),
        moving(moving_init_data, moving_init_data + size), 
        bins(64)
{        
	CPathNameArray sksearchpath({bfs::path("splinekernel")});
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 

        rkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=0");  
        mkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=3"); 
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

static double evaluate_mi_direct_256_256(const vector<float>& a, const vector<float>& b) 
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


class CSplineParzenMIFixture: public CSplineParzenMI {
public: 
	CSplineParzenMIFixture(); 
}; 

CSplineParzenMIFixture::CSplineParzenMIFixture():
	CSplineParzenMI(256, produce_spline_kernel("bspline:d=0"), 
			256, produce_spline_kernel("bspline:d=2"), 0)
{
	int real_bins = get_real_bins(); 

	vector<double> values(get_real_joint_bins); 
	
	fill_histograms(0, 256, 0, 256, values); 

}

BOOST_FIXTURE_TEST_CASE( test_MI_random, CSplineParzenMIFixture )
{

	


       

	// fill the image with random data 
	for (auto ir = reference.begin(), im = moving.begin(); ir != reference.end(); ++ir, ++im) {
		*ir = (*uniform)(); 
		*im  = (*gauss)() + *ir;
		if (*im < 0) 
			*im = 0; 
		if (*im > 255) 
			*im = 255; 
	}

        CSplineParzenMI smi();

	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	BOOST_CHECK_CLOSE(smi.value(), evaluate_mi_direct_256_256(moving, reference), 3); 

	for (double m = 0; m < 256; m += 1)
                for (double r = 0; r < 256; r += 1) {
                        double grad = smi.get_gradient(m, r);
			
			

                }

	

}





