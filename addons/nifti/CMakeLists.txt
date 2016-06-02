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

OPTION(WITH_NIFTI "Use nifti-1 3D image file format" TRUE)

IF (WITH_NIFTI)
  SET(NIFTI_FOUND FALSE)
  
  # zlib is required for nifti to function 
  FIND_PACKAGE(ZLIB)
  IF(ZLIB_FOUND) 
    # check that nifti header is available 
    FIND_PATH( NIFTI_INCLUDE_DIR nifti1_io.h
      /usr/include 
      /usr/include/nifti
      /usr/local/include
      /usr/local/include/nifti 
      )
    
    FIND_LIBRARY(NIFTIIO_LIBRARY
      NAMES
      niftiio 
      ) 

    FIND_LIBRARY(NIFTIZNZ_LIBRARY
      NAMES
      znz 
      ) 
    IF( NIFTIIO_LIBRARY AND NIFTIZNZ_LIBRARY AND NIFTI_INCLUDE_DIR) 
      SET(NIFTI_FOUND TRUE)
      SET(NIFTIIO_LIBRARIES 
        ${NIFTIIO_LIBRARY}
        ${NIFTIZNZ_LIBRARY}
        ${ZLIB_LIBRARIES}
        m)
      MESSAGE(STATUS "libz=${ZLIB_LIBRARIES}")
    ENDIF()
    
  ENDIF(ZLIB_FOUND) 

  if (NOT NIFTI_FOUND AND STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "NIFTI includes and or libraries not found and strict dependencies enabled")
  endif (NOT NIFTI_FOUND AND STRICT_DEPENDECIES)

  INCLUDE_DIRECTORIES(${NIFTI_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})

  SET(imageio_path "${PLUGIN_INSTALL_PATH}/3dimage/io")
  SET(NIFTI_LINK_LIBS_3D ${NIFTIIO_LIBRARIES} mia3d)
  PLUGIN_WITH_TEST_AND_PREFIX2("3dimage" "io" niftiimage "${NIFTI_LINK_LIBS_3D}")
  
ENDIF (WITH_NIFTI)
