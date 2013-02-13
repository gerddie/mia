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


/** When filling the histogram by using splie coefficients, the numeric gradient will not be zero 
    unless the image intensities are uniformly distributed. 
BOOST_FIXTURE_TEST_CASE( test_same_image_gradient_is_zero, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(253, rkernel, 253, mkernel, 0); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
	for (double m = 2; m < 254; m += 1)
		for (double r = 0; r < 256; r += 1)
			BOOST_CHECK_EQUAL(smi.get_gradient_slow(m,r), 0.0); 

}
*/


BOOST_FIXTURE_TEST_CASE( test_differnt_image_gradient_is_not_zero, SplineMutualInformationFixture ) 
{
        CSplineParzenMI smi(bins, rkernel, bins, mkernel,0); 
	smi.fill(moving.begin(), moving.end(), reference.begin(), reference.end()); 
        int cnt = 0; 
        for (double m = 2; m < 254; m += 1)
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
        bins(128)
{        
	CPathNameArray sksearchpath({bfs::path("splinekernel")});
	CSplineKernelPluginHandler::set_search_path(sksearchpath); 

        rkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=0");  
        mkernel = CSplineKernelPluginHandler::instance().produce("bspline:d=3"); 
}


class CSplineParzenMIFixture: public CSplineParzenMI {
public: 
	CSplineParzenMIFixture(); 
protected: 
	double p(double r, double m) const;
	double px(double x) const; 
	double py(double x) const; 
	double dm(double r, double m) const; 
	double mi_direct()const; 
	double entropy_direct(const vector <double>& p)const; 
	static const int bins = 128; 
        // values obtained by using (wx)maxima 
	static constexpr double HX = -4.847144258909647; 
	static constexpr double HY = -4.699286674800261; 
	static constexpr double HXY =-9.528771470298688; 
	static constexpr double MI =  HX + HY - HXY;

	vector<double> m_hm; 
	vector<double> m_hr; 
	vector<double> m_values; 
}; 

constexpr double CSplineParzenMIFixture::HX; 
constexpr double CSplineParzenMIFixture::HY; 
constexpr double CSplineParzenMIFixture::HXY; 
constexpr double CSplineParzenMIFixture::MI;


CSplineParzenMIFixture::CSplineParzenMIFixture():
	CSplineParzenMI(bins, produce_spline_kernel("bspline:d=0"), 
			bins, produce_spline_kernel("bspline:d=3"), 0), 
	m_hm(bins), 
	m_hr(bins), 
	m_values(bins * bins)
{

	auto iv = m_values.begin(); 
	for (int r = 0; r < bins; ++r) {
		for (int m = 0; m < bins; ++m, ++iv) {
			
			*iv = p(r, m);
			m_hm[m] += *iv; 
			m_hr[r] += *iv; 
				
		}
	}
		
	fill_histograms(m_values, 0, bins-1, 0, bins-1); 
}

double CSplineParzenMIFixture::mi_direct() const
{
	return entropy_direct(m_hm) + entropy_direct(m_hr) - entropy_direct(m_values); 
}

double CSplineParzenMIFixture::entropy_direct(const vector <double>& p)const
{
	double sum = 0.0; 
	for(auto i = p.begin(); i != p.end(); ++i) 
		if (*i>0.0) 
			sum += *i * std::log(*i); 
	return sum; 
}

double CSplineParzenMIFixture::p(double y, double x) const
{
	constexpr double scale = M_PI * M_PI * M_PI * M_PI/(M_PI-4) /4294967296.0; 
	return  scale * cos(M_PI / 256.0 * (x-y))*(y - 128)*y;
}

double CSplineParzenMIFixture::px(double x) const 
{
	const double pxh = M_PI * x /256.0; 
	return M_PI / 512.0 * (sin(pxh) + cos(pxh)); 
}




double CSplineParzenMIFixture::py(double y) const 
{
	constexpr double scale =M_PI * M_PI * M_PI/(16777216 * (M_PI- 4.0)); 
	const double pyh = M_PI * y /256.0; 
	const double pymh = M_PI * (y -128)  / 256.0; 
	return scale * (y- 128 ) * y * ( sin(pyh) - sin(pymh) ); 
}

double CSplineParzenMIFixture::dm(double y, double x) const
{
	constexpr double pi4 = pow(M_PI,4.0);
	constexpr double scale_p = M_PI * pi4 / (1099511627776.0 * (M_PI - 4));
	constexpr double scale_px = M_PI * M_PI / 131072.0; 
	const double pyxh = M_PI * (y-x)  / 256.0; 
	const double pxh = M_PI * x  / 256.0; 
	
	const double dplogp_dx =  scale_p * y * (y-128) * sin(pyxh) * 
		(std::log( pi4 * (y-128)*y * cos(pyxh) / 
			   (4294967296*(M_PI-4))) + 1); 
	
	const double dpxlogopx_dx = - scale_px * (std::log(px(x)) + 1) * ( sin(pxh) - cos(pxh)); 
	
	return -dplogp_dx +dpxlogopx_dx; 
}

inline double f_log(double x) 
{
	return x * std::log(x); 
}

BOOST_FIXTURE_TEST_CASE( test_self, CSplineParzenMIFixture )
{
	for (double m = 1; m < (bins - 1); m += 1)
                for (double r = 1; r < (bins - 1); r += 1) {
			double self_grad = dm(r , m ); 
			double dmp = 5 * (f_log(p(r,m + 0.1)) - f_log(p(r,m - 0.1)));  
			double dmpx = 5 * (f_log(px(m + 0.1)) - f_log(px(m - 0.1))); 
			double test_grad = dmpx - dmp; 
			
			cvdebug() << self_grad << " vs " << test_grad << ":" << self_grad / test_grad<<"\n"; 


			if (test_grad != 0) 
				BOOST_CHECK_CLOSE(self_grad, test_grad, 0.1); 
                }
	
}



BOOST_FIXTURE_TEST_CASE( test_MI_random, CSplineParzenMIFixture )
{
	
	BOOST_CHECK_CLOSE(value(), mi_direct(), 0.3);
	BOOST_CHECK_CLOSE(value(), MI, 0.3);

	// at the boundary of the "moving" gradient we are more forgiving, since here 
	// the analytic function is not well modeled by the parzen spline histogram 
	// hence, do not test the first and last two indices. 
	// otoh for the reference values, the gradient at the boundary is NAN
	for (double m = 2; m < (bins - 2); m += 1)
                for (double r = 1; r < bins - 1; r += 1) {
                        double grad = get_gradient_slow(m, r);
			double test_grad = dm(r , m ); 

			// at 64,64 the gradient is very close to zero 
			if ((fabs(grad) > 1e-10) || (fabs(test_grad) > 1e-10))
				BOOST_CHECK_CLOSE(grad, test_grad, 0.1); 
			
			cvdebug() << "Q="<< test_grad/ grad << "\n"; 
                }

}

BOOST_FIXTURE_TEST_CASE( test_MI_76_13, CSplineParzenMIFixture )
{
	double grad = get_gradient_slow(76, 13);
	double test_grad = dm(13, 76 ); 
	BOOST_CHECK_CLOSE(grad, test_grad, 0.1); 
}





