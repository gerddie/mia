#
# check support for lambda expressions (c++0x)
#

CHECK_CXX_SOURCE_COMPILES(
"#include <algorithm>
 #include <vector>
int main(int argc, char *args[]) 
{
	std::vector<float> c(2, 2); 
        std::transform(c.begin(), c.end(), c.begin(), [](float x){return x*x;}); 
        return 0; 
}
" CXX_HAS_CXX_0X_LAMBDA)