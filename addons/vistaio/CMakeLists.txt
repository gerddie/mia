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
    pkg_check_modules(VISTAIO REQUIRED libvistaio >= 1.2.12)
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
  SET(VISTAIO_SRC vista4mia.cc)
  SET(VISTAIO4MIALIBS miacore ${VISTAIO_LIBRARIES})
  MIA_ADD_LIBRARY(vista4mia "${VISTAIO_SRC}" "${VISTAIO4MIALIBS}")
ELSE(USE_SYSTEM_VISTAIO)
  MESSAGE(STATUS "vistaio: using internal version")
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
    vista4mia.cc
    )

  
  MIA_ADD_LIBRARY(vista4mia "${VISTAIO_SRC}" miacore)
  
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

SET(image2dio_path ${PLUGIN_INSTALL_PATH}/2dimage/io)
SET(image3dio_path ${PLUGIN_INSTALL_PATH}/3dimage/io)

SET(trans2dio_path ${PLUGIN_INSTALL_PATH}/2dtransform/io)
SET(trans3dio_path ${PLUGIN_INSTALL_PATH}/3dtransform/io)

SET(vf3dio_path ${PLUGIN_INSTALL_PATH}/3dvf/io)
SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/2dvf/io)

SET(meshio_path ${PLUGIN_INSTALL_PATH}/mesh/io)

PLUGIN_PRE(2d-imageio 2dvistaio "${DEPS2D}" "${image2dio_path}")
PLUGIN_PRE(3d-imageio 3dvistaio "${DEPS3D}" "${image3dio_path}")
PLUGIN_PRE(2d-vfio 2dvfvistaio "${DEPS2D}" "${vf2dio_path}")
PLUGIN_PRE(3d-vfio 3dvfvistaio "${DEPS3D}" "${vf3dio_path}")
PLUGIN_PRE(3d-mesh vistamesh "${DEPSMESH}" "${meshio_path}")
PLUGIN_PRE(2d-transformio 2dtrans "${DEPS2D}" "${trans2dio_path}")
PLUGIN_PRE(3d-transformio 3dtrans "${DEPS3D}" "${trans3dio_path}")


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
