###################################################################################
#
# Look for IT++ for FastICA
#
OPTION(WITH_ITPP "Use IT++ for alternative FastICA implementation" ${SEARCH_LIBS_DEFAULT})
IF(WITH_ITPP)
  
  IF (STRICT_DEPENDECIES)
    pkg_check_modules(ITPP itpp REQUIRED)
  ELSE (STRICT_DEPENDECIES)
    pkg_check_modules(ITPP itpp)
  ENDIF(STRICT_DEPENDECIES)
  
  IF(ITPP_FOUND)
    INCLUDE_DIRECTORIES(${ITPP_INCLUDE_DIRS})
    LINK_DIRECTORIES(${ITPP_LIBRARY_DIRS})
    
    #if ITPP was found check if it supports the return value in separate 
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

    IF(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE) 
      MESSAGE(WARNING "You are using  an unpatched version of IT++. "
	"In this version using APPROACH_DEFL may run forever, and FastICA::separate " 
	"doesn't return a value indicating success, "
	"therefore, APPROACH_DEFL will not be used. See also: "
	"https://sourceforge.net/tracker/?func=detail&aid=3028968&group_id=37044&atid=418758")
      SET(ICA_ALWAYS_USE_SYMM 1)
    ELSE(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE) 
      SET(ICA_ALWAYS_USE_SYMM 0)
    ENDIF(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE)

    SET( FAST_ICA_LIBRARIES ${ITPP_LIBRARIES} miacore)
    PLUGINGROUP_WITH_TEST_AND_PREFIX2("fastica" "implementation" "ica" "${FAST_ICA_LIBRARIES}")
    
  ENDIF(ITPP_FOUND)
ENDIF(WITH_ITPP)
#
# End looking for IT++ 
#
##################################################################################
