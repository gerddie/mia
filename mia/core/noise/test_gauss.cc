#include <mia/internal/autotest.hh>
#include <mia/core/noise/gauss.hh>
#include <cmath>


NS_MIA_USE
using namespace std; 
using namespace ::boost::unit_test; 
using namespace gauss_noise_generator; 

BOOST_AUTO_TEST_CASE( test_gauss )
{
	const double mu = 1.0; 
	const double sigma = 10.0; 
	
	CGaussNoiseGenerator ng(1, mu, sigma); 
	
	double sum1 = 0.0; 
	double sum2 = 0.0; 
	const size_t n = 10000000; 
	
	size_t k = n; 
	while (k--) {
		double val = ng(); 
		sum1 += val; 
		sum2 += val * val; 
	}
	
	cvdebug() << sum1 << " (" << sum2 << ")\n"; 
	
	sum1 /= n; 
	sum2 = sqrt(( sum2 - n * sum1 * sum1) / (n-1)); 
	
	cvdebug() << sum1 << " (" << sum2 << ")\n"; 

	BOOST_CHECK_CLOSE(mu, sum1, 1.0); 
	BOOST_CHECK_CLOSE(sigma, sum2, 1.0); 
	
}
