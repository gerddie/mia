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


SET(image2dio_path ${PLUGIN_INSTALL_PATH}/2dimage/io)
SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/2dvf/io)

ADD_CUSTOM_TARGET(2d-imageio_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${image2dio_path})
ADD_CUSTOM_TARGET(2d-vfio_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${vf2dio_path})

PLUGIN_PRE("2d-imageio" bmp "${MIA2DLIBS}" "${image2dio_path}")
PLUGIN_PRE("2d-imageio" raw "${MIA2DLIBS}" "${image2dio_path}")

IF(PNG_FOUND)
  SET(PNG_LINK_LIBS ${MIA2DLIBS} ${PNG_LIBRARIES})
  PLUGIN_PRE("2d-imageio" png "${PNG_LINK_LIBS}" "${image2dio_path}")
  INCLUDE_DIRECTORIES(${PNG_PNG_INCLUDE_DIR})
ENDIF(PNG_FOUND)

IF(TIFF_FOUND)
  SET(TIFF_LINK_LIBS ${MIA2DLIBS} ${TIFF_LIBRARIES})
  PLUGIN_PRE("2d-imageio" tiff "${TIFF_LINK_LIBS}" "${image2dio_path}")
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)

IF(JPEG_FOUND)
  SET(JPEG_LINK_LIBS ${MIA2DLIBS} ${JPEG_LIBRARIES})
  PLUGIN_PRE("2d-imageio" jpg "${JPEG_LINK_LIBS}" "${image2dio_path}")
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
ENDIF(JPEG_FOUND)





