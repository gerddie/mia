#
# Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

SET(MEXMODULE_SOURE 
  miamex_proxy.cc 
  miamex_filter2d.cc 
  miamex_filter3d.cc 
  miamex_reg2d.cc 
  miamex_reg3d.cc 
  miamex_helpers.cc
)

SET(MATLAB_PROGRAMS 
  miareg2d.m 
  miadeform2d.m 
  miareg3d.m 
  miadeform3d.m 
  testmia.m 
  testreg2d.m
)

OPTION(WITH_MATLAB "enable matlab support" TRUE)


IF (WITH_MATLAB)
  #
  # Find Matlab - note: MSVC80 and MINGW not yet supported if FIND_PACKAGE, do it the long way ...
  #
  IF (NOT MSVC80 AND NOT MINGW) 
    FIND_PACKAGE(Matlab)
  ELSE   (NOT MSVC80  AND NOT MINGW)
    GET_FILENAME_COMPONENT(MATLAB_ROOT  [HKEY_LOCAL_MACHINE\\SOFTWARE\\MathWorks\\MATLAB\\7.1;MATLABROOT]
		ABSOLUTE CACHE)
    FIND_PATH(MATLAB_INCLUDE_DIR "mex.h" 
      PATHS "${MATLAB_ROOT}/extern/include"
      )
    FIND_LIBRARY(MATLAB_MEX_LIBRARY mex libmex
        PATHS "${MATLAB_ROOT}/extern/lib/win32/microsoft/msvc71/" )
    
    FIND_LIBRARY(MATLAB_MX_LIBRARY mx libmx
      PATHS "${MATLAB_ROOT}/extern/lib/win32/microsoft/msvc71")
    
    IF (MATLAB_INCLUDE_DIR AND MATLAB_MEX_LIBRARY AND MATLAB_MX_LIBRARY)
      SET(MATLAB_FOUND 1)
    ENDIF (MATLAB_INCLUDE_DIR AND MATLAB_MEX_LIBRARY AND MATLAB_MX_LIBRARY)
  ENDIF (NOT MSVC80  AND NOT MINGW)
  
  #
  # On Unix a test should be added, whether the same libstc++ is used by the compiler and Matlab
  #
  
  IF (MATLAB_FOUND) 
    SET(MEXOUTPATH ${PLUGIN_INSTALL_PATH}/matlab)   

    INCLUDE_DIRECTORIES(${MATLAB_INCLUDE_DIR})
    
    # Add the c module
    ADD_LIBRARY(miamex  MODULE miamex.cc)
    TARGET_LINK_LIBRARIES(miamex ${MATLAB_MEX_LIBRARY}) 

    IF (WIN32)
      TARGET_LINK_LIBRARIES(miamex ${MATLAB_MX_LIBRARY}) 
      SET_TARGET_PROPERTIES(miamex PROPERTIES LINK_FLAGS /def:"${CMAKE_CURRENT_SOURCE_DIR}/miamex.def")
    ELSEIF (APPLE)
      TARGET_LINK_LIBRARIES(miamex ${DL})
    ELSE(APPLE)
      SET_TARGET_PROPERTIES(miamex PROPERTIES PREFIX "" SUFFIX ".mexglx")
      TARGET_LINK_LIBRARIES(miamex ${DL})
    ENDIF(WIN32)
    
    #add the real module
    ADD_LIBRARY(miamex_proxy  MODULE ${MEXMODULE_SOURE})
    TARGET_LINK_LIBRARIES(miamex_proxy mia3d mia2d ${MATLAB_MEX_LIBRARY} ${MATLAB_MX_LIBRARY}) 
    SET_TARGET_PROPERTIES(miamex_proxy PROPERTIES PREFIX "")
    
    INSTALL(TARGETS miamex miamex_proxy LIBRARY DESTINATION ${MEXOUTPATH})
    INSTALL(FILES   ${MATLAB_PROGRAMS} DESTINATION ${MEXOUTPATH})
    
  ELSEIF (STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "MatLab not found and strict dependencies enabled")
  ENDIF(MATLAB_FOUND)

ENDIF (WITH_MATLAB)
