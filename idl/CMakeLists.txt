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

SET(MIADLM_PROXY_SRC
  filter2d.cc
  helpers.cc
  idlmia_proxy.cc
  reg2d.cc
  io.cc
)

SET(MIAIDL_EXAMPLES 
  filter3d.pro  
  filter.pro  
  get_dllname.pro  
  nrreg2d.pro  
  test_all.pro  
  test_echo.pro  
  testme.pro
  test_mia.prj
)

SET(IDL_PLUGIN_PATH "${PLUGIN_INSTALL_PATH}/idl")

OPTION(WITH_IDL "enable support for IDL" TRUE)

IF (NOT WIN32)
  SET(WRAP_LIBS -ldl)
ENDIF(NOT WIN32)      

IF(WITH_IDL)
  IF (NOT IDLROOT)
    IF (WIN32)                                                                                   
      GET_FILENAME_COMPONENT(IDLROOT  [HKEY_LOCAL_MACHINE\\SOFTWARE\\ITT\\IDL\\6.4;InstallDir] ABSOLUTE CACHE)
    ELSE (WIN32)
      SET(IDLROOT "/usr/local/rsi")
    ENDIF (WIN32)
  ENDIF(NOT IDLROOT)
  FIND_PATH(IDL_INCLUDE_DIR idl_export.h
    "${IDLROOT}/idl64/external/include"
    )

  IF (IDL_INCLUDE_DIR)
    IF("${IDL_INCLUDE_DIR}" MATCHES "/external/include")
      STRING(REGEX REPLACE "\(.*\)/external/include" "\\1" IDL_ROOT "${IDL_INCLUDE_DIR}" )    
    ENDIF("${IDL_INCLUDE_DIR}" MATCHES "/external/include")
    #    
    # find the IDL libraries     
    #
    IF(WIN32)
      FIND_LIBRARY(IDLLIB 
	NAMES idl32 idl 
	PATHS 
        "${IDL_ROOT}/bin/bin.x86"
	)
      
      IF("${IDLLIB}" MATCHES "/idl[0-9]*\\.lib$")
	STRING(REGEX REPLACE "\(.*\)/idl[0-9]*\\.lib" "\\1" IDL_LIB_PATH "${IDLLIB}" )    
      ENDIF("${IDLLIB}" MATCHES "/idl[0-9]*\\.*[libso]*$")
      
      SET(PLUGINLIBS mia3d mia2d miacore)
      SET(IDLPLUGINLIBS ${PLUGINLIBS} "${IDLLIB}")
      
    ELSE (WIN32)
      
      FIND_LIBRARY(IDLLIB 
	NAMES idl 
	PATHS "${IDL_ROOT}/bin/bin.linux.x86"
	)
      
      IF("${IDLLIB}" MATCHES "/libidl[0-9]*\\.so$")
	STRING(REGEX REPLACE "\(.*\)/libidl[0-9]*\\.so" "\\1" IDL_LIB_PATH "${IDLLIB}" )    
      ENDIF("${IDLLIB}" MATCHES "/libidl[0-9]*\\.so$")
      
      SET(PLUGINLIBS mia3d mia2d miacore ${BASELIBS})
      SET(IDLPLUGINLIBS ${PLUGINLIBS})
      
    ENDIF (WIN32)
    #
    # define the IDL extensuon libraries
    # 
    
    INCLUDE_DIRECTORIES(${IDL_INCLUDE_DIR})
    LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/core)
    LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/2d)
    LINK_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../mia/3d)
    
    #  
    # this is the interface library that loads the proxy library to ensure 
    # C++ exceptions activation
    #
    ADD_LIBRARY(mia MODULE idlmia.c)
    SET_TARGET_PROPERTIES(mia PROPERTIES PREFIX "")
    TARGET_LINK_LIBRARIES(mia ${IDLLIB} ${WRAP_LIBS})
    INSTALL(TARGETS mia LIBRARY DESTINATION ${IDL_PLUGIN_PATH})
    INSTALL(FILES   mia.dlm DESTINATION ${IDL_PLUGIN_PATH})
    
    ADD_LIBRARY(idlmia_proxy MODULE ${MIADLM_PROXY_SRC})
    SET_TARGET_PROPERTIES(idlmia_proxy PROPERTIES PREFIX "")
    TARGET_LINK_LIBRARIES(idlmia_proxy ${IDLPLUGINLIBS})
    IF (WIN32)
		INSTALL(TARGETS idlmia_proxy LIBRARY DESTINATION "bin")
    ELSE(WIN32)
		INSTALL(TARGETS idlmia_proxy LIBRARY DESTINATION ${IDL_PLUGIN_PATH})
	ENDIF(WIN32)
    INSTALL(FILES ${MIAIDL_EXAMPLES} DESTINATION ${IDL_PLUGIN_PATH})
    
    #
    # att the tests that need the command line version of IDL, which is not avilable 
    # on MS Windows
    #
    IF (NOT WIN32)

      SET(WRAP_LIBS -ldl)
      
      FIND_PACKAGE(Motif)
      IF (MOTIF_FOUND) 
	FIND_LIBRARY(TERMCAP termcap NAMES termcap libtermcap  PATHS 
	  /lib 
	  /usr/lib
	  )
	FIND_LIBRARY(XINERAMA Xinerama NAMES Xinerama libXinerama  PATHS /usr/lib)
	FIND_LIBRARY(XPM Xpm NAMES Xpm libXpm  PATHS /usr/lib)
	SET(IDL_STANDARTLIBS  -lrt -lm -lpthread -lgcc_s )
	
	ADD_EXECUTABLE(test-idl2 idltest2.c)	
	TARGET_LINK_LIBRARIES(test-idl2 ${IDLLIB}  ${MOTIF_LIBRARIES} ${XINERAMA} ${XPM} ${TERMCAP} ${IDL_STANDARTLIBS})
	ADD_TEST(idlcall2 /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/test-idl2.sh  ${IDL_LIB_PATH})

	IF (NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	  ADD_CUSTOM_TARGET(dlmlink ln -sf  ${CMAKE_CURRENT_SOURCE_DIR}/mia.dlm ${CMAKE_CURRENT_BINARY_DIR}/mia.dlm)
	  ADD_DEPENDENCIES(test-idl2 dlmlink)
	ENDIF(NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
	
	ADD_EXECUTABLE(test-idl-nrreg idltest.c)	
	TARGET_LINK_LIBRARIES(test-idl-nrreg ${IDLLIB}  ${MOTIF_LIBRARIES} ${XINERAMA} ${XPM} ${TERMCAP} ${IDL_STANDARTLIBS})
	ADD_TEST(test-idl-nrreg /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/test-idl.sh  ${IDL_LIB_PATH})
      ENDIF(MOTIF_FOUND)
    ENDIF (NOT WIN32)
    
  ELSEIF (STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "IDL not found and strict dependencies enabled")
  ENDIF(IDL_INCLUDE_DIR)
  
ENDIF(WITH_IDL)
