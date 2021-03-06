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

OPTION(WITH_DCMTK "enable support for DCMTK DICOM IO" ${SEARCH_LIBS_DEFAULT})


SET(dicom4mia_SOURCES dicom4mia.cc)

SET(image2dio_path ${PLUGIN_INSTALL_PATH}/2dimage/io)
SET(image3dio_path ${PLUGIN_INSTALL_PATH}/3dimage/io)

IF (WITH_DCMTK)
  FIND_PACKAGE(DCMTKnew)
  IF(DCMTK_FOUND)
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_DCMTK_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    SET(dicom4mia_deps mia3d ${DCMTK_LIBRARIES})

   
    MIA_ADD_LIBRARY(dicom4mia "${dicom4mia_SOURCES}" "${dicom4mia_deps}")
    INSTALL_BASE(dicom4mia)

    SET(DCMTK_LINK_LIBS_2D dicom4mia)
    SET(DCMTK_LINK_LIBS_3D dicom4mia)
    NEW_TEST(dicom4mia dicom4mia)

    PLUGIN_WITH_TEST_AND_PREFIX2("2dimage" "io" dcm2d "${DCMTK_LINK_LIBS_2D}")
    PLUGIN_WITH_TEST_AND_PREFIX2("3dimage" "io" dcm3d "${DCMTK_LINK_LIBS_3D}")
    INCLUDE_DIRECTORIES(${DCMTK_INCLUDE_DIR})

  ELSEIF(STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "DCMTK not found and strict dependencies enabled")
  ENDIF(DCMTK_FOUND)
ENDIF(WITH_DCMTK)



