#
# check existence of _Complex type (c++0x)
#

CHECK_CXX_SOURCE_COMPILES(
"#include <complex>
#include <complex.h>
int main(int argc, char *args[]) 
{
	std::complex<double> c(2.0, 1.0); 
	double _Complex *a = reinterpret_cast<double _Complex *>(&c); 
	std::complex<double> b(*a); 
	return 0; 
}
" CXX_HAS_COMPLEX_C_TYPE)