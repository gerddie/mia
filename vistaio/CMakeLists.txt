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

OPTION(VISTAIO_SYSTEM "use system version of libvistaio" TRUE)

IF(VISTAIO_SYSTEM)
  if (STRICT_DEPENDECIES)
    pkg_check_modules(VISTAIO REQUIRED libvistaio)
  else (STRICT_DEPENDECIES)
    pkg_check_modules(VISTAIO libvistaio)
  endif (STRICT_DEPENDECIES)
  IF( VISTAIO_FOUND )
    SET(USE_SYSTEM_VISTAIO TRUE)
  ENDIF(VISTAIO_FOUND )
ENDIF(VISTAIO_SYSTEM)
  
IF(USE_SYSTEM_VISTAIO)
  INCLUDE_DIRECTORIES(${VISTAIO_INCLUDE_DIRS})
  LINK_DIRECTORIES(${VISTAIO_LIBRARY_DIRS})
  ADD_LIBRARY(vista4mia SHARED vista4mia.cc)
  SET_TARGET_PROPERTIES(vista4mia PROPERTIES VERSION ${LIBRARY_VERSION_INFO})
  TARGET_LINK_LIBRARIES(vista4mia ${miacore})
  TARGET_LINK_LIBRARIES(vista4mia ${VISTAIO_LIBRARIES})
ELSE(USE_SYSTEM_VISTAIO)
  MESSAGE(STATUS "using internal version")
  SET(VISTAIO_SRC 
    alloc.c
    attr.c
    dictionary.c
    edges.c
    edgesType.c
    error.c
    fileio.c
    graphType.c
    history.c
    image.c
    imageType.c
    list.c
    packData.c
    type.c
    vcplist.c
    version.c
    vfield3d.c
    vfield2d.c
    volumes.c
    volumesType.c
    )

  ADD_LIBRARY(vista4mia SHARED vista4mia.cc ${VISTAIO_SRC})
  TARGET_LINK_LIBRARIES(vista4mia miacore)
  SET_TARGET_PROPERTIES(vista4mia PROPERTIES VERSION ${LIBRARY_VERSION_INFO})
  
  # test programs for internal use code
  ADD_EXECUTABLE(test-3dfield   test3dfield.c)
  TARGET_LINK_LIBRARIES(test-3dfield vista4mia)
  ADD_EXECUTABLE(test-2dfield   test2dfield.c)
  TARGET_LINK_LIBRARIES(test-2dfield vista4mia)
  ADD_TEST(vista3dfield test-3dfield)
  ADD_TEST(vista2dfield test-2dfield)

ENDIF(USE_SYSTEM_VISTAIO)

SET(DEPS2D mia2d vista4mia)
SET(DEPS3D mia3d vista4mia)
SET(DEPSMESH miamesh vista4mia)

SET(image2dio_path ${PLUGIN_INSTALL_PATH}/io/2dimage)
SET(image3dio_path ${PLUGIN_INSTALL_PATH}/io/3dimage)

SET(trans2dio_path ${PLUGIN_INSTALL_PATH}/io/2dtransform)
SET(trans3dio_path ${PLUGIN_INSTALL_PATH}/io/3dtransform)

SET(vf3dio_path ${PLUGIN_INSTALL_PATH}/io/3dvf)
SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/io/2dvf)

SET(meshio_path ${PLUGIN_INSTALL_PATH}/io/mesh)

PLUGIN(2dvistaio "${DEPS2D}" "${image2dio_path}")
PLUGIN(3dvistaio "${DEPS3D}" "${image3dio_path}")
PLUGIN(2dvfvistaio "${DEPS2D}" "${vf2dio_path}")
PLUGIN(3dvfvistaio "${DEPS3D}" "${vf3dio_path}")
PLUGIN(vistamesh "${DEPSMESH}" "${meshio_path}")
PLUGIN(2dtrans "${DEPS2D}" "${trans2dio_path}")
PLUGIN(3dtrans "${DEPS3D}" "${trans3dio_path}")


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
