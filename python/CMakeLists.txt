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


FIND_PACKAGE( PythonLibs )

if ( PYTHONLIBS_FOUND )
  FIND_PACKAGE(PythonInterp)

   
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  ADD_LIBRARY(mia MODULE ${PYTHON_SRC})
  SET_TARGET_PROPERTIES(mia PROPERTIES PREFIX "")

  # not sure  is this is really needed now, 
  # better interface will be with numpy
  FIND_PACKAGE( Boost 1.46.1 COMPONENTS python)
  TARGET_LINK_LIBRARIES(mia ${mia3d}  ${Boost_PYTHON_LIBRARY} ${PYTHON_LIBRARIES} )

  #
  #
  #
  EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import numpy; print numpy.get_include()"
    OUTPUT_VARIABLE NUMPY_INCLUDE_PATH
    RESULT_VARIABLE NUMPY_ERR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  IF(NUMPY_ERR) 
    MESSAGE(ERROR "This code requires numpy to be installed")
  ENDIF(NUMPY_ERR)
  INCLUDE_DIRECTORIES(${NUMPY_INCLUDE_PATH})
  MESSAGE(STATUS "numpy include path ${NUMPY_INCLUDE_PATH}")
endif ( PYTHONLIBS_FOUND )
