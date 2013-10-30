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

OPTION(USE_HDF5 "use hdf5 for data UI" TRUE)


IF (USE_HDF5)
  if (STRICT_DEPENDECIES)
    find_package(HDF5 REQUIRED)
  else (STRICT_DEPENDECIES)
    find_package(HDF5)
  endif (STRICT_DEPENDECIES)
  
  IF(HDF5_FOUND)
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_HDF5_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
    LINK_DIRECTORIES(${HDF5_LIBRARY_DIRS})
    SET(HDF5IO_SRC hdf5mia.cc hdf5a_mia.cc)
    SET(HDF5MIALIBS miacore ${HDF5_LIBRARIES})
    MIA_ADD_LIBRARY(miahdf5 "${HDF5IO_SRC}" "${HDF5MIALIBS}")
    SET(INSTALL_TARGETS miahdf5)	
    INSTALL_BASE("${INSTALL_TARGETS}")


    NEW_TEST(hdf5mia miahdf5)
    SET(DEPS3D miahdf5 mia3d)

    PLUGIN_WITH_TEST_AND_PREFIX2("3dimage" "io" hdf5_3dimage  "${DEPS3D}")

  ENDIF(HDF5_FOUND)
  
ENDIF(USE_HDF5)
