/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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
#include <mia/core/mitestimages.hh>
#include <mia/core/splineparzenmi.hh>
#include <boost/filesystem.hpp>

NS_MIA_USE; 
using namespace std; 
using namespace boost::unit_test;
namespace bfs=boost::filesystem; 

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
        CSplineParzenMI smi(256, haar, 256, haar); 
	smi.fill(reference.begin(), reference.end(), reference.begin(), reference.end()); 
        BOOST_CHECK_CLOSE(smi.value(), -5.1013951881429653, 0.1); 
}


BOOST_FIXTURE_TEST_CASE( test_different_image_entropy, SplineMutualInformationFixture ) 
{
        PSplineKernel haar = CSplineKernelPluginHandler::instance().produce("bspline:d=0"); 
        CSplineParzenMI smi(256, haar, 256, haar); 
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
        CSplineParzenMI smi(bins, rkernel, bins, mkernel); 
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
        CSplineParzenMI smi(bins, rkernel, bins, mkernel); 
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
	list< bfs::path> sksearchpath; 
	sksearchpath.push_back( bfs::path("splinekernel"));
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 

        rkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=0");  
        mkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=3"); 


}




