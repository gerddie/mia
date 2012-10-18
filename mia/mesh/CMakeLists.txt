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

SET(MIAMESH_SRC
  triangularMesh.cc
  filter.cc
) 

SET(MIAMESH_HEADERS
  triangularMesh.hh
  triangulate.hh
  filter.hh
  clist.hh
)

MIA_ADD_LIBRARY(miamesh "${MIAMESH_SRC}" mia3d)

MACRO(TEST_MESH name)
  ADD_EXECUTABLE(test-${name} test_${name}.cc)  
  TARGET_LINK_LIBRARIES(test-${name} miamesh ${BOOST_UNITTEST})
  ADD_TEST(${name} test-${name})
  IF(WIN32)
    SET_TARGET_PROPERTIES(test-${name} PROPERTIES LINKER_FLAGS "/NODEFAULTLIB:MSVCRT")
  ENDIF(WIN32)
ENDMACRO(TEST_MESH name)

SET(INSTALL_MESH miamesh)	
INSTALL_WITH_EXPORT("${INSTALL_MESH}")
INSTALL(FILES ${MIAMESH_HEADERS} DESTINATION ${INCLUDE_INSTALL_PATH}/mia/mesh)

SET(MIAMESHLIBS miamesh)

ADD_SUBDIRECTORY(io)
ADD_SUBDIRECTORY(filter)


TEST_MESH(triangulate)