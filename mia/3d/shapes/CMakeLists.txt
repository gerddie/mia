#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

SET(shape3d 
	    3d-shapes
)


SET(SHAPES_SRC basic_shapes.cc sphere.cc)
add_library(3d-shapes-common OBJECT ${SHAPES_SRC})
IF(NOT WIN32)
  set_source_files_properties(${SHAPES_SRC}  PROPERTIES COMPILE_FLAGS "-fPIC")
  set_target_properties(3d-shapes-common  PROPERTIES COMPILE_FLAGS -DVSTREAM_DOMAIN='"${plugname}"') 
ENDIF(NOT WIN32)


add_library(3d-shapes MODULE $<TARGET_OBJECTS:3d-shapes-common>)

TARGET_LINK_LIBRARIES(3d-shapes ${MIA3DLIBS})
SET_TARGET_PROPERTIES(3d-shapes PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
INSTALL(TARGETS 3d-shapes LIBRARY DESTINATION "${PLUGIN_INSTALL_PATH}/3dimage/shape")

ADD_CUSTOM_TARGET(3d-shapes_test_link 
  ln -sf "${CMAKE_CURRENT_BINARY_DIR}/3d-shapes.mia" ${PLUGIN_TEST_ROOT}/${PLUGIN_INSTALL_PATH}/3dimage/shape/ 
  DEPENDS 3dimage_shape_testdir 3d-shapes)

ADD_DEPENDENCIES(plugin_test_links 3d-shapes_test_link)

add_executable(test-3d-shapes test_shapes.cc $<TARGET_OBJECTS:3d-shapes-common>)
IF(NOT WIN32)
  set_target_properties(test-3d-shapes PROPERTIES 
    COMPILE_FLAGS -DVSTREAM_DOMAIN='"3d-shapes"' 
    COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
ELSE(NOT WIN32)
  set_target_properties(test-3d-shapes PROPERTIES
    COMPILE_FLAGS -DBOOST_TEST_DYN_LINK)
ENDIF(NOT WIN32)
#  target_link_libraries(test-${plugname} ${plugname}-common)
target_link_libraries(test-3d-shapes ${MIA3DLIBS} ${BOOST_UNITTEST})
add_test(3d-shapes test-3d-shapes)



