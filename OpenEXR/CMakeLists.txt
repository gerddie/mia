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

OPTION(WITH_OpenEXR "enable support for OpenEXR" ${SEARCH_LIBS_DEFAULT})

IF(WITH_OpenEXR)
  
  if (STRICT_DEPENDECIES)
    pkg_check_modules(EXR REQUIRED OpenEXR)
  else (STRICT_DEPENDECIES)
    pkg_check_modules(EXR OpenEXR)
  endif (STRICT_DEPENDECIES)
  
  
  IF( EXR_LIBRARIES AND EXR_INCLUDE_DIRS )
    
    SET(image2dio_path ${PLUGIN_INSTALL_PATH}/2dimage/io)
    SET(image3dio_path ${PLUGIN_INSTALL_PATH}/3dimage/io)
    SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/2dvf/io)
    
    SET(EXR_LINK_LIBS ${MIA2DLIBS} ${EXR_LIBRARIES})

    LINK_DIRECTORIES(${EXR_LIBRARY_DIRS})

    SET(DEPS2D mia2d ${EXR_LIBRARIES})
    
    INCLUDE_DIRECTORIES(${EXR_INCLUDE_DIRS})
    
    PLUGIN_PRE(2d-imageio 2dimgexr "${DEPS2D}" "${image2dio_path}")
    PLUGIN_PRE(2d-vfio    2dvfexr  "${DEPS2D}" "${vf2dio_path}")
    
    # test programs 
    ADD_EXECUTABLE(test-openexr test_openexr.cc)
    ADD_DEPENDENCIES(test-openexr 2dvfexr 2dimgexr)
    
    SET(DEPLIBS mia2dtest mia2d ${BASELIBS})
    TARGET_LINK_LIBRARIES(test-openexr ${DEPLIBS} ${BOOST_UNITTEST})
    
    IF(NOT WIN32)
      #a wrapper script is needed, to add the current build dir to the search path for dlopen
      ADD_TEST(exr sh ${CMAKE_CURRENT_SOURCE_DIR}/test-openexr.sh )
    ENDIF(NOT WIN32)  
    
  ENDIF(EXR_LIBRARIES AND EXR_INCLUDE_DIRS)
  
ENDIF(WITH_OpenEXR)
