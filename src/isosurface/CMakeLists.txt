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

SET(miagts_SOURCES 
  iso_backend.cc  mesh_convert.cc
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
  INCLUDE_DIRECTORIES(${GTS_INCLUDE_DIRS})
  set( miagts_DEPS miamesh ${GTS_LIBRARIES})
  MIA_ADD_LIBRARY(miagts "${miagts_SOURCES}" "${miagts_DEPS}")
  INSTALL_WITH_EXPORT(miagts)

  # add the ISO-surface programs 
  ADD_EXECUTABLE(mia-3disosurface-from-volume iso.cc)
  ADD_DEPENDENCIES(mia-3disosurface-from-volume plugin_test_links)
  MIA_EXE_CREATE_DOCU_AND_INTERFACE(mia 3disosurface-from-volume)
  TARGET_LINK_LIBRARIES(mia-3disosurface-from-volume miagts)
  INSTALL(TARGETS mia-3disosurface-from-volume RUNTIME DESTINATION "bin")

  ADD_EXECUTABLE(mia-3disosurface-from-stack iso_from_slices.cc)
  ADD_DEPENDENCIES(3disosurface-from-stack plugin_test_links)
  MIA_EXE_CREATE_DOCU_AND_INTERFACE(mia 3disosurface-from-stack)
  TARGET_LINK_LIBRARIES(mia-3disosurface-from-stack miagts)
  INSTALL(TARGETS mia-3disosurface-from-stack RUNTIME DESTINATION "bin")

  # add GTS IO plug-in
  
  
ENDIF(GTS_FOUND) 