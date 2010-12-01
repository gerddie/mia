#include <cmath>
#include <climits>
#include <iomanip>


#include <mia/internal/autotest.hh>
#include <boost/lambda/lambda.hpp>

#include <mia/core/mitestimages.hh>
#include <mia/core/splineparzenmi.hh>

NS_MIA_USE; 
using namespace std; 
using namespace boost::unit_test;
using boost::lambda::_1; 


struct SplineMutualInformationFixture  {
	SplineMutualInformationFixture();

	size_t size; 
	vector<double> reference;
	vector<double> moving;
	size_t bins; 
        PBSplineKernel rkernel; 
        PBSplineKernel mkernel; 
       

};


BOOST_FIXTURE_TEST_CASE( test_same_image_entropy, SplineMutualInformationFixture ) 
{
        PBSplineKernel haar(new CBSplineKernel0); 
        CSplineParzenMI smi(256, haar, 256, haar); 
	smi.fill(reference.begin(), reference.end(), reference.begin(), reference.end()); 
        BOOST_CHECK_CLOSE(smi.value(), -5.1013951881429653, 0.1); 
}


BOOST_FIXTURE_TEST_CASE( test_different_image_entropy, SplineMutualInformationFixture ) 
{
        PBSplineKernel haar(new CBSplineKernel0); 
        CSplineParzenMI smi(256, haar, 256, haar); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	
        // this test value is not really evaluated ...
        BOOST_CHECK_CLOSE(smi.value(), -1.1739765269855784, 0.1); 
}

BOOST_FIXTURE_TEST_CASE( test_same_image_gradient_is_zero, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(128, mkernel, 256, mkernel); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	for (double m = 0; m < 256; m += 1)
		for (double r = 0; r < 256; r += 1)
			BOOST_CHECK_EQUAL(smi.get_gradient(m,r), 0.0); 

}

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
                }
                BOOST_CHECK(cnt > 0); 
        cvdebug() << "nozero gradient values =" << cnt <<"\n"; 
        
}
SplineMutualInformationFixture::SplineMutualInformationFixture():
        size(mi_test_size.width * mi_test_size.height), 
        reference(reverence_init_data, reverence_init_data + size),
        moving(moving_init_data, moving_init_data + size), 
        bins(64),
        rkernel(new CBSplineKernel0), 
        mkernel(new CBSplineKernel3)
{        
}




