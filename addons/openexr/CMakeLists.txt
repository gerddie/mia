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
    
    PLUGIN_WITH_PREFIX2("2dimage" "io" 2dimgexr "${DEPS2D}")
    PLUGIN_WITH_PREFIX2("2dvf" "io"    2dvfexr "${DEPS2D}")
    
    # test programs 
    ADD_EXECUTABLE(test-openexr test_openexr.cc)
    ADD_DEPENDENCIES(test-openexr 2dimage-io-openexr 2dvf-io-openexr)
    
    SET(DEPLIBS mia2dtest mia2d ${BASELIBS})
    TARGET_LINK_LIBRARIES(test-openexr ${DEPLIBS} ${BOOST_UNITTEST})
    
    ADD_TEST(exr test-openexr)
    
    
  ENDIF(EXR_LIBRARIES AND EXR_INCLUDE_DIRS)
  
ENDIF(WITH_OpenEXR)
