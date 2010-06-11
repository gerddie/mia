#
# check existence of _Complex type (c++0x)
#

CHECK_CXX_SOURCE_COMPILES(
"#include <vector>

int main(int argc, char *args[]) 
{
	std::vector<float> c(2); 
        auto i = c.begin();
        return 0; 
}
" CXX_HAS_CXX_0X_AUTO)