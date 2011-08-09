#
# Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
#

PROJECT("mia")

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0012 NEW)
endif(COMMAND cmake_policy)

include(FindPkgConfig)
include(CheckCXXSourceCompiles)

# Making releases:
#   MICRO_VERSION += 1;
#   INTERFACE_AGE += 1;
#   BINARY_AGE += 1;
# if any functions have been added, set INTERFACE_AGE to 0.
# if backwards compatibility has been broken,
# set BINARY_AGE and INTERFACE_AGE to 0.


SET(VENDOR "Gert Wollny")
SET(PACKAGE_NAME "mia")
SET(MAJOR_VERSION 2)
SET(MINOR_VERSION 0)
SET(MICRO_VERSION 7)
SET(INTERFACE_AGE 0)
SET(BINARY_AGE    0)
SET(PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}")
SET(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}")
SET(MIA_NAME "${PACKAGE_NAME}-${VERSION}")
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(PLUGIN_OUTPUT "${CMAKE_BINARY_DIR}/plugins")

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

MATH(EXPR MIA_CURRENT "${MICRO_VERSION} - ${INTERFACE_AGE}")
SET(MIA_REVISION ${MICRO_VERSION})
MATH(EXPR MIA_AGE "${BINARY_AGE} - ${INTERFACE_AGE}")

SET(LIBRARY_VERSION_INFO "${MIA_CURRENT}.${MIA_REVISION}.${MIA_AGE}")
SET(LIBRARY_SOVERSION_INFO "${MAJOR_VERSION}")

OPTION(STRICT_DEPENDECIES "require that all requested optinal dependencies are availabe" FALSE)
#SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)


INCLUDE (CheckIncludeFiles)
INCLUDE (TestBigEndian)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/pluginmacro.cmake)
INCLUDE (CheckTypeSize)

INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkCpp0xAuto.cmake)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkCpp0xLambda.cmake)


IF(NOT "${CXX_HAS_CXX_0X_AUTO}" OR NOT CXX_HAS_CXX_0X_LAMBDA)
  MESSAGE(FATAL_ERROR 
    "This source code requires the C++0x auto type and lambda features. "
    "If your compiler is supposed to support this feature, "
    "please enable the appropriate flags and delete the CMakeCache.txt file before "
    "re-running cmake.\n"
    "For g++ >= 4.5 this is '-std=c++0x' ")
ENDIF(NOT "${CXX_HAS_CXX_0X_AUTO}" OR NOT CXX_HAS_CXX_0X_LAMBDA)

ASSERT_SIZE(char 1)
ASSERT_SIZE(short 2)
ASSERT_SIZE(int 4)
ASSERT_SIZE(float 4)
ASSERT_SIZE(double 8)

SET(HAVE_INT64 "/* #undef HAVE_INT64 */")

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)

# find additional packages
#
# With MSVC using pre-installed dynamic libraries is usually a pain because of
# the incompatibilities in the compiler flags
#
IF (MSVC)
  SET(SEARCH_LIBS_DEFAULT FALSE)
ELSE(MSVC)
  SET(SEARCH_LIBS_DEFAULT TRUE)
ENDIF (MSVC)


# Taken from ITK:
# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings
IF(MSVC)
  ADD_DEFINITIONS(
    -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
    -D_CRT_IS_WCTYPE_NO_DEPRECATE
    -D_CRT_MANAGED_FP_NO_DEPRECATE
    -D_CRT_NONSTDC_NO_DEPRECATE
    -D_CRT_SECURE_NO_DEPRECATE
    -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
    -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
    -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
    -D_CRT_VCCLRIT_NO_DEPRECATE
    -D_SCL_SECURE_NO_DEPRECATE
    -D_SECURE_SCL=0
    )
ENDIF(MSVC)


SET(SOURCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
SET(BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}")
SET(LIB_ROOT_INSTALL_PATH "lib/${MIA_NAME}")
SET(CONFIG_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/config")
SET(LIB_INCLUDE_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/include")
SET(PLUGIN_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/plugins")
SET(PLUGIN_SEARCH_PATH "${CMAKE_INSTALL_PREFIX}/${PLUGIN_INSTALL_PATH}")
SET(DOC_INSTALL_PATH "shared/doc/${PACKAGE_NAME}-${PACKAGE_VERSION}")
SET(BINARY_INSTALL_PATH "bin")
SET(LIBRARY_INSTALL_PATH "lib")
SET(INCLUDE_INSTALL_PATH "include/${MIA_NAME}")


######################################################################################
#
# Handle BOOST
#
#
MACRO(GET_BOOST_LINKERFLAG VAR LIBNAMES)
  FOREACH(L  ${LIBNAMES})
    STRING(REGEX MATCH "boost_[-_a-z0-9]*" temp ${L})
    STRING(LENGTH "${temp}" TEMP_LENGTH)
    IF(${TEMP_LENGTH})
      SET(flags "${flags} -l${temp}")
    ENDIF(${TEMP_LENGTH})
  ENDFOREACH(L  ${LIBNAMES})
  
  SET(${VAR} "${flags}")
ENDMACRO(GET_BOOST_LINKERFLAG)

SET(BOOST_COMPONENTS   filesystem  regex unit_test_framework serialization system)
IF (WIN32)
  SET(BOOST_COMPONENTS  ${BOOST_COMPONENTS} date_time)
ENDIF(WIN32)

FIND_PACKAGE( Boost 1.42.0 COMPONENTS
  ${BOOST_COMPONENTS}
  REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})


#
# currently we completely disable boost based portable binary archives 
# VITSA provides what is needed in big binary archieves 
#
IF(FALSE) 
  find_path(HAVE_BOOST_PORTABLE_BINARY_ARCHIVE  
    portable_binary_archive.hpp 
    ${Boost_INCLUDE_DIR}/boost/archive)
  IF(NOT HAVE_BOOST_PORTABLE_BINARY_ARCHIVE)
    ADD_SUBDIRECTORY(backport)
    INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/backport")
  ELSE(NOT HAVE_BOOST_PORTABLE_BINARY_ARCHIVE)
    #if the portable binary archive has become part of the BOOST distribution 
    #just link the serialization library 
    SET(portable_binary_archive ${Boost_SERIALIZATION})
  ENDIF(NOT HAVE_BOOST_PORTABLE_BINARY_ARCHIVE)
ENDIF(FALSE)

ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_FILESYSTEM_VERSION=3)

SET(BOOST_DEPS  ${Boost_REGEX_LIBRARY}  ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
SET(BOOST_UNITTEST  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

IF (WIN32 AND NOT UNIX)
   FOREACH(l ${Boost_THREAD_LIBRARY}
       ${Boost_REGEX_LIBRARIES}
       ${Boost_FILESYSTEM_LIBRARY}
       ${Boost_SYSTEM}
       ${Boost_DATE_TIME}
       )
      STRING(REGEX MATCH "lib$" head ${l})
      STRING(COMPARE EQUAL "xlib" "x${head}" islib)
      IF (${islib})
	  STRING(REGEX REPLACE "lib$" "dll" lDLL ${l})
	  INSTALL(FILES ${lDLL} DESTINATION "bin")
      ENDIF(${islib})
   ENDFOREACH(l)
ENDIF (WIN32 AND NOT UNIX)

IF (MSYS OR MINGW)
  SET(BASELIBS ${BOOST_DEPS})
ENDIF (MSYS OR MINGW)

IF(UNIX)
  GET_BOOST_LINKERFLAG(BOOST_LIBFLAG "${BOOST_DEPS}")
  SET(MIA_DEPEND_LIBRARIES "${BOOST_LIBFLAG} -lpthread -ldl -L${Boost_LIBRARY_DIRS}")
ENDIF(UNIX)
#
# end BOOST 
#
################################################################################

################################################################################
#
# Intel Threading Building Blocks 
# 
FIND_PACKAGE(TBB REQUIRED)
IF(TBB_FOUND) 
  MESSAGE(STATUS "TBB found: ${TBB_LIBRARIES}")
  include_directories(${TBB_INCLUDE_DIRS})
  link_directories(${TBB_LIBRARY_DIRS})
ELSE(TBB_FOUND) 
  MESSAGE(FATAL " TBB not found")
ENDIF(TBB_FOUND)
#
# End Intel Threading Building Blocks 
#
################################################################################

################################################################################
# 
# Handle the GNU Scientific Library, this might be a showstopper 
# on MS windows when using Visual C++ 
#
PKG_CHECK_MODULES(GSL REQUIRED gsl)
INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIRS})
LINK_DIRECTORIES(${GSL_LIBRARY_DIRS})


################################################################################
# 
# search for some cblas
# 
# Note, on Gentoo on can use 'eselect blas set X' to select the actual BLAS provider
# 
# 
FIND_LIBRARY(BLAS_LIBRARY NAMES "blas" PATHS
  /usr/lib
  /usr/local/lib
  )

IF(NOT BLAS_LIBRARY)
  #try mkl 
  FIND_LIBRARY(MKLIF_LIBRARY NAMES "mkl_intel_lp64" PATHS
    /opt/intel/mkl/lib/intel64 
    )
  FIND_LIBRARY(MKL_sequential_LIBRARY NAMES "mkl_sequential" PATHS
    /opt/intel/mkl/lib/intel64 
    )
  FIND_LIBRARY(MKL_core_LIBRARY NAMES "mkl_core" PATHS
    /opt/intel/mkl/lib/intel64 
    )
  IF (MKL_core_LIBRARY AND MKL_sequential_LIBRARY AND MKLIF_LIBRARY)
    set(BLAS_LIBRARY ${MKLIF_LIBRARY} ${MKL_sequential_LIBRARY}
      ${MKL_core_LIBRARY} 
      )
  ENDIF(MKL_core_LIBRARY AND MKL_sequential_LIBRARY AND MKLIF_LIBRARY)
ENDIF(NOT BLAS_LIBRARY)

IF (BLAS_LIBRARY) 
  ADD_DEFINITIONS(-DHAVE_BLAS)
ELSE(BLAS_LIBRARY) 
  MESSAGE(FATAL_ERROR "A library that implements the BLAS interface is required")
ENDIF(BLAS_LIBRARY)
#
# End BLAS 
# 
##################################################################################

##################################################################################
#
# take care of modules loading infrastructure
#
IF(NOT WIN32)
  FIND_LIBRARY(DL dl
    PATH
    /lib
    /usr/lib
    /opt/local/lib
    /sw/lib
  )
  IF(DL_NOTFOUND)
    MESSAGE(FATAL_ERROR "non-windows systems without libdl.so are not yet supported")
  ENDIF(DL_NOTFOUND)
  SET(BASELIBS  ${BOOST_DEPS} ${DL})

ENDIF(NOT WIN32)
SET(PLUGSUFFIX ".mia")

OPTION(WARN_OLD_PLUGINSTYLE "Warn if plugins use old implementation style" OFF)
#
#
#
##################################################################################

##################################################################################
#
# FFTW related tests 
#
pkg_check_modules(FFTWF REQUIRED fftw3f)
IF(FFTWF_FOUND)
    INCLUDE_DIRECTORIES(${FFTWF_INCLUDE_DIRS})
    LINK_DIRECTORIES(${FFTWF_LIBRARY_DIRS})
    SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} fftw3f")
ENDIF(FFTWF_FOUND)

MARK_AS_ADVANCED(WITH_FFTWD)
OPTION(WITH_FFTWD "Use double precision FFTW if found" ON)

IF(WITH_FFTWD)
  pkg_check_modules(FFTWD fftw3 REQUIRED)
  IF(FFTWD_FOUND)
    INCLUDE_DIRECTORIES(${FFTWD_INCLUDE_DIRS})
    LINK_DIRECTORIES(${FFTWD_LIBRARY_DIRS})
    SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} fftw3")
  ENDIF(FFTWD_FOUND)
ENDIF(WITH_FFTWD)
#
#
####################################################################################

####################################################################################
#
# add xml library for loading/storing segmentations 
#
OPTION(WITH_XMLPP "Use XML++ for some additional features" ${SEARCH_LIBS_DEFAULT})
IF(WITH_XMLPP)
  pkg_check_modules(XMLPP libxml++-2.6)
  IF (XMLPP_FOUND)
    INCLUDE_DIRECTORIES(${XMLPP_INCLUDE_DIRS})
    LINK_DIRECTORIES(${XMLPP_LIBRARY_DIRS})
    SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} libxml++-2.6")
    SET(HAVE_LIBXMLPP 1)
  ELSE(XMLPP_FOUND)
    SET(HAVE_LIBXMLPP 0)
  ENDIF (XMLPP_FOUND)
ENDIF(WITH_XMLPP)
#
# end XML 
# 
###################################################################################

###################################################################################
#
# Look for IT++ for FastICA
#
OPTION(WITH_ITPP "Use IT++ for some additional features" ${SEARCH_LIBS_DEFAULT})
IF(WITH_ITPP)
  pkg_check_modules(ITPP itpp)
  IF(ITPP_FOUND)
    INCLUDE_DIRECTORIES(${ITPP_INCLUDE_DIRS})
    LINK_DIRECTORIES(${ITPP_LIBRARY_DIRS})
    SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} itpp")
    SET(CMAKE_REQUIRED_LIBRARIES ${ITPP_LIBRARIES})
    #if ITPP was found check if it supports the return value in separate 
    INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkITPPBoolseparate.cmake)
    IF(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE) 
      MESSAGE(WARNING "You are using  an unpatched version of IT++"
	"In this version using APPROACH_DEFL may run forever, and FastICA::separate " 
	"doesn't return a value indicating success, "
	"therefore, APPROACH_DEFL will not be used. See also: "
	"https://sourceforge.net/tracker/?func=detail&aid=3028968&group_id=37044&atid=418758")
      SET(ICA_ALWAYS_USE_SYMM 1)
    ELSE(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE) 
      SET(ICA_ALWAYS_USE_SYMM 0)
    ENDIF(NOT ITPP_FASTICA_SEPARATE_HAS_RETURNVALUE) 
  ENDIF(ITPP_FOUND)
ENDIF(WITH_ITPP)
#
# End looking for IT++ 
#
##################################################################################

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

#add the root directories to the search path
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})


ENABLE_TESTING()

ADD_SUBDIRECTORY(gsl++   )
ADD_SUBDIRECTORY(mia     )
ADD_SUBDIRECTORY(vistaio )
ADD_SUBDIRECTORY(OpenEXR )
ADD_SUBDIRECTORY(dicom   )
ADD_SUBDIRECTORY(src     )
ADD_SUBDIRECTORY(testdata)
ADD_SUBDIRECTORY(idl     )
ADD_SUBDIRECTORY(matlab  )
ADD_SUBDIRECTORY(python  )

ADD_SUBDIRECTORY(doc)

IF(UNIX)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/mia.pc)
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mia.pc DESTINATION "lib/pkgconfig")
ENDIF(UNIX)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/miaconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/miaconfig.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mia.hh DESTINATION ${INCLUDE_INSTALL_PATH})
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/miaconfig.h DESTINATION ${LIB_INCLUDE_INSTALL_PATH})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README ${CMAKE_CURRENT_SOURCE_DIR}/COPYING
  DESTINATION ${DOC_INSTALL_PATH})

INCLUDE(cmake/package.cmake)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/UseMia.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/UseMia.cmake)
INSTALL(EXPORT Mia DESTINATION ${CONFIG_INSTALL_PATH})

