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

SET(PYTHON_SRC mia_python.cc)

#INSTALL(FILES ${SCRIPTS}
#  DESTINATION "bin"
#  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
#    )


FIND_PACKAGE( Boost 1.36.1 COMPONENTS python)
FIND_PACKAGE( PythonLibs )

if ( PYTHONLIBS_FOUND )
  FIND_PACKAGE(PythonInterp)

  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  ADD_LIBRARY(mia MODULE ${PYTHON_SRC})
  SET_TARGET_PROPERTIES(mia PROPERTIES PREFIX "")
  TARGET_LINK_LIBRARIES(mia ${mia3d}  ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES} )

#  if (PYTHONINTERP_FOUND)
#    ADD_TEST(mia-python ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_mia.py)
#  endif(PYTHONINTERP_FOUND)
endif ( PYTHONLIBS_FOUND )
