#
# check whether itpp FastICA::separate has a bool return value 
#

CHECK_CXX_SOURCE_COMPILES(
"#include <itpp/signal/fastica.h>

int main(int argc, char *args[]) 
{
	itpp::mat  m_Signal(1,1);
	itpp::Fast_ICA ica(m_Signal); 
	if (ica.separate())
	  return 0; 
	else
	  return -1;
}
" ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE)