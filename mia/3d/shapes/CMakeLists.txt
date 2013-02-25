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

SET(shape3d 
	    3d-shapes
)


ADD_LIBRARY(3d-shapes MODULE basic_shapes.cc sphere.cc)
TARGET_LINK_LIBRARIES(3d-shapes ${MIA3DLIBS})
SET_TARGET_PROPERTIES(3d-shapes PROPERTIES PREFIX "" SUFFIX ${PLUGSUFFIX})
INSTALL(TARGETS 3d-shapes LIBRARY DESTINATION "${PLUGIN_INSTALL_PATH}/3dimage/shape")

ADD_CUSTOM_TARGET(3d-shapes_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${PLUGIN_INSTALL_PATH}/3dimage/shape)
ADD_CUSTOM_TARGET(3d-shapes_test_link 
  ln -sf "${CMAKE_CURRENT_BINARY_DIR}/3d-shapes.mia" ${PLUGIN_TEST_ROOT}/${PLUGIN_INSTALL_PATH}/3dimage/shape/ 
  DEPENDS 3d-shapes_testdir 3d-shapes)

ADD_DEPENDENCIES(plugin_test_links 3d-shapes_test_link)

