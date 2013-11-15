#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
#
# MIA is free software; you can redistribute it and/or modify
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
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

PROJECT("mia")

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5 FATAL_ERROR)

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
SET(MICRO_VERSION 11)
SET(INTERFACE_AGE 0)
SET(BINARY_AGE    0)

SET(PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}")
SET(VERSION "${MAJOR_VERSION}.${MINOR_VERSION}")
SET(MIA_NAME "${PACKAGE_NAME}-${VERSION}")
SET(prefix ${CMAKE_INSTALL_PREFIX})
SET(PLUGIN_OUTPUT "${CMAKE_BINARY_DIR}/plugins")

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

MATH(EXPR MIA_CURRENT "${MICRO_VERSION} - ${BINARY_AGE}")
SET(MIA_REVISION ${INTERFACE_AGE})
MATH(EXPR MIA_AGE "${BINARY_AGE} - ${INTERFACE_AGE}")

SET(LIBRARY_SOVERSION_INFO "${MIA_CURRENT}")
SET(LIBRARY_VERSION_INFO "${MIA_CURRENT}.${MIA_AGE}.${MIA_REVISION}")

OPTION(STRICT_DEPENDECIES "require that all requested optinal dependencies are availabe" FALSE)
OPTION(ALWAYS_CREATE_DOC "Create all documentation during the normal build process (normally you need to run 'make doc')" TRUE)
OPTION(BUILD_EXAMPLES "Build example plug-ins and programs" FALSE)

INCLUDE(GNUInstallDirs)

INCLUDE (CheckIncludeFiles)
INCLUDE (TestBigEndian)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros.cmake)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/pluginmacro.cmake)
INCLUDE (CheckTypeSize)

include(CheckCXXCompilerFlag)

MESSAGE("CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
IF(UNIX)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_GNU_CXX11_FLAG)
  IF(HAS_GNU_CXX11_FLAG)
    SET(CXX_11_FLAG "-std=c++11")
  ELSE(HAS_GNU_CXX11_FLAG)
    CHECK_CXX_COMPILER_FLAG("-std=c++0x" HAS_GNU_CXX0X_FLAG)
    IF(HAS_GNU_CXX0X_FLAG)
      SET(CXX_11_FLAG "-std=c++0x")
    ELSE(HAS_GNU_CXX0X_FLAG)
      MESSAGE(WARNING "Don't know how to enable C++11, if you are lucky, they are enabled by default")
    ENDIF(HAS_GNU_CXX0X_FLAG)
  ENDIF(HAS_GNU_CXX11_FLAG)
  
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CXX_11_FLAG}")
#  SET(CMAKE_REQUIRED_FLAGS "${CXX_11_FLAG}")
ENDIF(UNIX)

INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkCpp0xAuto.cmake)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkCpp0xLambda.cmake)


IF(NOT "${CXX_HAS_CXX_0X_AUTO}" OR NOT CXX_HAS_CXX_0X_LAMBDA)
  MESSAGE(FATAL_ERROR 
    "This source code requires the C++0x auto type and lambda features. "
    "If your compiler is supposed to support this feature, "
    "please enable the appropriate flags and delete the CMakeCache.txt file before "
    "re-running cmake.\n"
    "For g++ >= 4.6 this is '-std=c++0x' ")
ENDIF(NOT "${CXX_HAS_CXX_0X_AUTO}" OR NOT CXX_HAS_CXX_0X_LAMBDA)

INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkAltivecEnabled.cmake)
INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkSSEEnabled.cmake)

ASSERT_SIZE(char 1)
ASSERT_SIZE(short 2)
ASSERT_SIZE(int 4)
ASSERT_SIZE(float 4)
ASSERT_SIZE(double 8)

CHECK_TYPE_SIZE(long LONG_TYPE_SIZE)


IF (${LONG_TYPE_SIZE} EQUAL 8)
  SET(LONG_64BIT TRUE)
ELSE(${LONG_TYPE_SIZE} EQUAL 8)
  SET(LONG_64BIT FALSE)
ENDIF(${LONG_TYPE_SIZE} EQUAL 8)

TEST_BIG_ENDIAN(WORDS_BIGENDIAN)
CHECK_INCLUDE_FILES (unistd.h HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (sys/ioctl.h HAVE_SYS_IOCTL_H)

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
SET(LIB_ROOT_INSTALL_PATH "${CMAKE_INSTALL_FULL_LIBDIR}/${MIA_NAME}")
SET(CONFIG_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/config")
SET(LIB_INCLUDE_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/include")
SET(PLUGIN_INSTALL_PATH "${LIB_ROOT_INSTALL_PATH}/plugins")
SET(PLUGIN_SEARCH_PATH "${PLUGIN_INSTALL_PATH}")
SET(DOC_INSTALL_PATH "share/doc/${MIA_NAME}")
SET(FULL_DOC_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/${DOC_INSTALL_PATH}")
SET(BINARY_INSTALL_PATH "${CMAKE_INSTALL_BINDIR}")
SET(LIBRARY_INSTALL_PATH "${CMAKE_INSTALL_LIBDIR}")
SET(INCLUDE_INSTALL_PATH "${CMAKE_INSTALL_INCLUDEDIR}/${MIA_NAME}")


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

FIND_PACKAGE( Boost 1.46.1 COMPONENTS
  ${BOOST_COMPONENTS}
  REQUIRED)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_FILESYSTEM_VERSION=3)

SET(BOOST_DEPS  ${Boost_REGEX_LIBRARY}  ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
SET(BOOST_UNITTEST  ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

IF (WIN32 AND NOT UNIX)
   FOREACH(l 
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
# Intel Threading Building Blocks 
# 

SET(CMAKE_THREAD_PREFER_PTHREAD ON) 
FIND_PACKAGE(Threads)

IF(CMAKE_USE_PTHREADS_INIT)
  set(PTHREADLIB -lpthread)
ENDIF(CMAKE_USE_PTHREADS_INIT)

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
# search for some blas
# 

FIND_PACKAGE(BLAS)
IF (NOT BLAS_LIBRARIES) 
  MESSAGE(FATAL_ERROR "A library that implements the BLAS interface is required") 
ENDIF(NOT BLAS_LIBRARIES) 

IF(UNIX)
  GET_BOOST_LINKERFLAG(BOOST_LIBFLAG "${BOOST_DEPS}")
  SET(MIA_DEPEND_LIBRARIES "${BOOST_LIBFLAG} ${BLAS_LIBRARIES} ${TBB_LIBRARIES} ${PTHREADLIB} -ldl")
ENDIF(UNIX)


ADD_DEFINITIONS(-DHAVE_BLAS)
#
# End BLAS 
# 
##################################################################################

##################################################################################
#
# Find gzip for man page compression 
#
find_program(GZIP gzip)
IF(NOT GZIP) 
  MESSAGE(gzip not found, man pages will not be compressed)
ENDIF(NOT GZIP) 

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
  SET(BASELIBS  ${BOOST_DEPS} ${PTHREADLIB} ${DL})
ENDIF(NOT WIN32)
SET(PLUGSUFFIX ".mia")

OPTION(WARN_MISSING_OR_OLD_PLUGINTESTS "Warn if plugins don't provided tests or use the old buildsystem to do so" OFF)
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
# add xml library for loading/storing segmentations and document creation  
#
pkg_check_modules(XMLPP libxml++-2.6 REQUIRED)
IF (XMLPP_FOUND)
  INCLUDE_DIRECTORIES(${XMLPP_INCLUDE_DIRS})
  LINK_DIRECTORIES(${XMLPP_LIBRARY_DIRS})
  SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} libxml++-2.6")
  SET(HAVE_LIBXMLPP 1)
ENDIF (XMLPP_FOUND)

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
  
  IF (STRICT_DEPENDECIES)
    pkg_check_modules(ITPP itpp REQUIRED)
  ELSE (STRICT_DEPENDECIES)
    pkg_check_modules(ITPP itpp)
  ENDIF(STRICT_DEPENDECIES)
  
  IF(ITPP_FOUND)
    INCLUDE_DIRECTORIES(${ITPP_INCLUDE_DIRS})
    LINK_DIRECTORIES(${ITPP_LIBRARY_DIRS})
    SET(PKG_CONFIG_DEPS "${PKG_CONFIG_DEPS} itpp")
    SET(CMAKE_REQUIRED_LIBRARIES ${ITPP_LIBRARIES})
    #if ITPP was found check if it supports the return value in separate 
    INCLUDE (${CMAKE_CURRENT_SOURCE_DIR}/cmake/checkITPPBoolseparate.cmake)
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

SET(PLUGIN_TEST_ROOT ${CMAKE_CURRENT_BINARY_DIR}/plugintest)

ADD_CUSTOM_TARGET(plugin_test_linkdir mkdir -p ${PLUGIN_TEST_ROOT})
ADD_CUSTOM_TARGET(plugin_test_links DEPENDS plugin_test_linkdir)

IF(ALWAYS_CREATE_DOC)
  ADD_CUSTOM_TARGET(doc ALL)
ELSE(ALWAYS_CREATE_DOC)
  ADD_CUSTOM_TARGET(doc)  
ENDIF(ALWAYS_CREATE_DOC)


##############################################
#
# python is needed for creating end-user documentation 
#
FIND_PACKAGE(PythonInterp)
IF(PYTHONINTERP_FOUND) 
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import lxml"  RESULT_VARIABLE LXML_ERR)
  IF(LXML_ERR) 
    MESSAGE(python found, but no lxml, no user documantation will be created) 
    SET(CREATE_USERDOC FALSE)
  ELSE(LXML_ERR)
    SET(CREATE_USERDOC TRUE)
  ENDIF(LXML_ERR) 
ENDIF(PYTHONINTERP_FOUND) 

#############################################
#
# define plug-in related values and directories and add the test directory link target 
# 

DEFINE_PLUGIN_NAMES( 1d  "spacialkernel;splinebc;splinekernel" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 2dimage  "combiner;cost;creator;filter;fullcost;io;model;shape;timestep;transform" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 2dsplinepenalty  "transform" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 2dstack  "filter" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 2dtransform  "io;splinepenalty" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 2dvf  "io" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 3dimage  "combiner;cost;creator;filter;fullcost;io;model;shape;timestep;transform" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 3dlandmarklist "io"  ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 3dtransform  "io;splinepenalty" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( 3dvf "io"  ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( generator  "noise" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( mesh "filter;io" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( minimizer  "singlecost" ${PLUGIN_INSTALL_PATH})
DEFINE_PLUGIN_NAMES( rgbimage "io" ${PLUGIN_INSTALL_PATH})

ADD_SUBDIRECTORY(gsl++   )
ADD_SUBDIRECTORY(mia     )
ADD_SUBDIRECTORY(addons  )
ADD_SUBDIRECTORY(testdata)

IF(BUILD_EXAMPLES)
  ADD_SUBDIRECTORY(examples)
ENDIF(BUILD_EXAMPLES)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)
ADD_SUBDIRECTORY(doc)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/miacore.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/miacore-${VERSION}.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia2d.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/mia2d-${VERSION}.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia2dmyocardperf.pc.cmake 
  ${CMAKE_CURRENT_BINARY_DIR}/mia2dmyocardperf-${VERSION}.pc)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/miamesh.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/miamesh-${VERSION}.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia3d.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/mia3d-${VERSION}.pc)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/mia.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/mia-${VERSION}.pc)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/miacore-${VERSION}.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mia2d-${VERSION}.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mia3d-${VERSION}.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/mia2dmyocardperf-${VERSION}.pc 
  DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/miamesh-${VERSION}.pc DESTINATION "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig")

GET_PROPERTY(HAVE_DCMTK GLOBAL PROPERTY HAVE_DCMTK_PROP DEFINED)
GET_PROPERTY(HAVE_HDF5 GLOBAL PROPERTY HAVE_HDF5_PROP DEFINED)
GET_PROPERTY(HAVE_VISTAIO GLOBAL PROPERTY HAVE_VISTAIO_PROP DEFINED)
GET_PROPERTY(HAVE_VTK GLOBAL PROPERTY HAVE_VTK_PROP DEFINED)
GET_PROPERTY(HAVE_OPENEXR GLOBAL PROPERTY HAVE_OPENEXR_PROP DEFINED)
GET_PROPERTY(HAVE_PNG GLOBAL PROPERTY HAVE_PNG_PROP DEFINED)
GET_PROPERTY(HAVE_TIFF GLOBAL PROPERTY HAVE_TIFF_PROP DEFINED)
GET_PROPERTY(HAVE_JPG GLOBAL PROPERTY HAVE_JPG_PROP DEFINED)
GET_PROPERTY(HAVE_NLOPT GLOBAL PROPERTY HAVE_NLOPT_PROP DEFINED)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/miaconfig.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/miaconfig.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/miaconfig.h DESTINATION ${LIB_INCLUDE_INSTALL_PATH})

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/mia.hh DESTINATION ${INCLUDE_INSTALL_PATH})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/README ${CMAKE_CURRENT_SOURCE_DIR}/COPYING 
  DESTINATION ${DOC_INSTALL_PATH})

 
ADD_CUSTOM_TARGET(dist bash ${CMAKE_SOURCE_DIR}/create_package "${PACKAGE_NAME}" "${PACKAGE_VERSION}" "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}")
