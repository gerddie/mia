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

OPTION(USE_VISTAIO "use libvistaio" TRUE)


IF (USE_VISTAIO)
  if (STRICT_DEPENDECIES)
    pkg_check_modules(VISTAIO REQUIRED libvistaio>=1.2.14)
  else (STRICT_DEPENDECIES)
    pkg_check_modules(VISTAIO libvistaio>=1.2.14)
  endif (STRICT_DEPENDECIES)
  
  IF( VISTAIO_FOUND )
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_VISTAIO_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    INCLUDE_DIRECTORIES(${VISTAIO_INCLUDE_DIRS})
    LINK_DIRECTORIES(${VISTAIO_LIBRARY_DIRS})
    SET(VISTAIO_SRC vista4mia.cc)
    SET(VISTAIO4MIALIBS miacore ${VISTAIO_LIBRARIES})
    MIA_ADD_LIBRARY(vista4mia "${VISTAIO_SRC}" "${VISTAIO4MIALIBS}")
    
    SET(DEPS2D mia2d vista4mia)
    SET(DEPS3D mia3d vista4mia)
    SET(DEPSMESH miamesh vista4mia)
    

    PLUGIN_WITH_TEST_AND_PREFIX2("2dimage" "io" 2dvistaio "${DEPS2D}")
    PLUGIN_WITH_TEST_AND_PREFIX2("3dimage" "io" 3dvistaio  "${DEPS3D}")
    
    PLUGIN_WITH_PREFIX2("2dvf" "io" 2dvfvistaio "${DEPS2D}")
    PLUGIN_WITH_PREFIX2("3dvf" "io" 3dvfvistaio "${DEPS3D}")
    PLUGIN_WITH_PREFIX2("mesh" "io" vistamesh "${DEPSMESH}")
    PLUGIN_WITH_PREFIX2("2dtransform" "io" 2dtrans "${DEPS2D}")
    PLUGIN_WITH_PREFIX2("3dtransform" "io" 3dtrans "${DEPS3D}")
    
  
    ADD_EXECUTABLE(test-vista4mia  test_vista4mia.cc)
    ADD_DEPENDENCIES(test-vista4mia  2dvistaio 3dvistaio 3dvfvistaio)
    
    SET(VISTADEPLIBS vista4mia mia3dtest mia2dtest mia3d mia2d)
    
    TARGET_LINK_LIBRARIES(test-vista4mia ${VISTADEPLIBS} ${BOOST_UNITTEST})
    
    SET(INSTALL_TARGETS vista4mia)	
    INSTALL_BASE("${INSTALL_TARGETS}")
    
    IF (WIN32)
      ADD_TEST(vista "${CMAKE_CURRENT_SOURCE_DIR}/test-vista4mia.bat")
    ELSE (WIN32)
      ADD_TEST(vista test-vista4mia)
    ENDIF(WIN32)
  
  ENDIF( VISTAIO_FOUND )
ENDIF(USE_VISTAIO)
