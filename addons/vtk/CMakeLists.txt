#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

OPTION(WITH_VTKIO "enable support for VTK files" ${SEARCH_LIBS_DEFAULT})

IF(WITH_VTKIO)
  if (STRICT_DEPENDECIES)
    FIND_PACKAGE(VTK 6.0 REQUIRED)
  else (STRICT_DEPENDECIES)
    FIND_PACKAGE(VTK 6.0 QUIET)
  endif (STRICT_DEPENDECIES)
  IF(VTK_FOUND)
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_VTK_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    SET(meshio_path ${PLUGIN_INSTALL_PATH}/mesh/io)
    SET(vfio_path "${PLUGIN_INSTALL_PATH}/3dvf/io")
    SET(imageio_path "${PLUGIN_INSTALL_PATH}/3dimage/io")
    
    SET(VTK_LINK_LIBS_MESH ${VTK_LIBRARIES} miamesh)
    SET(VTK_LINK_LIBS_3D ${VTK_LIBRARIES} mia3d)

    PLUGIN_WITH_TEST_AND_PREFIX2("mesh" "io" vtkmesh "${VTK_LINK_LIBS_MESH}")
    PLUGIN_WITH_TEST_AND_PREFIX2("3dvf" "io" vtkvf "${VTK_LINK_LIBS_3D}")
    PLUGIN_WITH_TEST_AND_PREFIX2("3dimage" "io" vtkimage "${VTK_LINK_LIBS_3D}")

  ELSEIF(VTK_FOUND)
    MESSAGE(MESSAGE "VTK not found, disabled")
  ENDIF(VTK_FOUND)
ENDIF(WITH_VTKIO)
