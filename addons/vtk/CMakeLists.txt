#
# Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

OPTION(WITH_VTKIO "enable support for VTK files" ${SEARCH_LIBS_DEFAULT})

IF(WITH_VTKIO)
  FIND_PACKAGE(VTK)
  IF(VTK_FOUND)
    INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    SET(meshio_path ${PLUGIN_INSTALL_PATH}/mesh/io)
    SET(vfio_path "${PLUGIN_INSTALL_PATH}/3dvf/io")
    SET(imageio_path "${PLUGIN_INSTALL_PATH}/3dimage/io")
    
    SET(VTK_LINK_LIBS_MESH ${VTK_LIBRARIES} miamesh)
    SET(VTK_LINK_LIBS_3D ${VTK_LIBRARIES} mia3d)

    SINGLEPLUGIN_WITH_TEST_AND_PREFIX("mesh" vtkmesh "${VTK_LINK_LIBS_MESH}" "${meshio_path}")
    SINGLEPLUGIN_WITH_TEST_AND_PREFIX("3d-vfio" vtkvf "${VTK_LINK_LIBS_3D}" "${vfio_path}")

    # the explicit vtk part is added because the build system needs to create unique targets 
    # and the plug-in test directory target is created based on the prefix
    #
    SINGLEPLUGIN_WITH_TEST_AND_PREFIX("3d-imageio-vtk" vtkimage "${VTK_LINK_LIBS_3D}" "${imageio_path}")

  ELSEIF(VTK_FOUND)
    MESSAGE(FATAL_ERROR "VTK not found and strict dependencies enabled")
  ENDIF(VTK_FOUND)
ENDIF(WITH_VTKIO)
