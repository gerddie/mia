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

SET(isosurface_SOURCES 
  iso.cc  iso_backend.cc  mesh_convert.cc
  )

OPTION(WITH_GTS "Use the GNU Triangulated Surfaces Library for some mesh realted features" ${SEARCH_LIBS_DEFAULT})

IF(WITH_GTS) 
  IF(STRICT_DEPENDECIES) 
    pkg_check_modules(GTS gts REQUIRED)
  ELSE(STRICT_DEPENDECIES) 
    pkg_check_modules(GTS gts REQUIRED)
  ENDIF(STRICT_DEPENDECIES) 
ENDIF(WITH_GTS) 

IF(GTS_FOUND) 

  # add the ISO-surface program 
  ADD_EXECUTABLE(mia-3dextract-isosurface ${isosurface_SOURCES})
  CREATE_EXE_DOCU(mia-3dextract-isosurface)
  INCLUDE_DIRECTORIES(${GTS_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(mia-3dextract-isosurface miamesh ${GTS_LIBRARIES})
  INSTALL(TARGETS mia-3dextract-isosurface RUNTIME DESTINATION "bin")

  # add GTS IO plug-in
  
  
ENDIF(GTS_FOUND) 