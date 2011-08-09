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

OPTION(WITH_PNG "enable support for PNG" ${SEARCH_LIBS_DEFAULT})
OPTION(WITH_TIFF "enable support for TIFF" ${SEARCH_LIBS_DEFAULT})

SET(image2dio_path ${PLUGIN_INSTALL_PATH}/io/2dimage)
SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/io/2dvf)

PLUGIN_PRE("2d" bmp "${MIA2DLIBS}" "${image2dio_path}")
PLUGIN_PRE("2d" raw "${MIA2DLIBS}" "${image2dio_path}")

IF (WITH_PNG)
  FIND_PACKAGE(PNG)
  IF(PNG_FOUND)
    SET(PNG_LINK_LIBS ${MIA2DLIBS} ${PNG_LIBRARIES})
    PLUGIN_PRE("2d" png "${PNG_LINK_LIBS}" "${image2dio_path}")
    INCLUDE_DIRECTORIES(${PNG_PNG_INCLUDE_DIR})
  ELSEIF(STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "PNG not found and strict dependencies enabled")
  ENDIF(PNG_FOUND)
ENDIF(WITH_PNG)

IF(WITH_TIFF)
  FIND_PACKAGE(TIFF)
  IF(TIFF_FOUND)
    SET(TIFF_LINK_LIBS ${MIA2DLIBS} ${TIFF_LIBRARIES})
    PLUGIN_PRE("2d" tiff "${TIFF_LINK_LIBS}" "${image2dio_path}")
    INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
  ELSEIF(STRICT_DEPENDECIES)
    MESSAGE(FATAL_ERROR "TIFF not found and strict dependencies enabled")
  ENDIF(TIFF_FOUND)
ENDIF(WITH_TIFF)




