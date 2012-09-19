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

SET(rgbimage2dio_path ${PLUGIN_INSTALL_PATH}/rgbimage/io)

ADD_CUSTOM_TARGET(2d-rgbimageio_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${rgbimage2dio_path})

PLUGIN_PRE("2d-rgbimageio" bmp "${MIA2DLIBS}" "${rgbimage2dio_path}")

IF (FALSE)

IF(PNG_FOUND)
  SET(PNG_LINK_LIBS ${MIA2DLIBS} ${PNG_LIBRARIES})
  PLUGIN_PRE("2d-rgbimageio" png "${PNG_LINK_LIBS}" "${rgbimage2dio_path}")
  INCLUDE_DIRECTORIES(${PNG_PNG_INCLUDE_DIR})
ENDIF(PNG_FOUND)

IF(TIFF_FOUND)
  SET(TIFF_LINK_LIBS ${MIA2DLIBS} ${TIFF_LIBRARIES})
  PLUGIN_PRE("2d-rgbimageio" tiff "${TIFF_LINK_LIBS}" "${rgbimage2dio_path}")
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)

IF(JPEG_FOUND)
  SET(JPEG_LINK_LIBS ${MIA2DLIBS} ${JPEG_LIBRARIES})
  PLUGIN_PRE("2d-rgbimageio" jpg "${JPEG_LINK_LIBS}" "${rgbimage2dio_path}")
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
ENDIF(JPEG_FOUND)

ENDIF(FALSE)


