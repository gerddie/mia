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

OPTION(WITH_JPEG "enable support for JPEG gray scale images IO and RGB saving" ${SEARCH_LIBS_DEFAULT})

IF(WITH_JPEG)
  IF(STRICT_DEPENDECIES)
    FIND_PACKAGE(JPEG REQUIRED)
  ELSE(STRICT_DEPENDECIES)
    FIND_PACKAGE(JPEG)
  ENDIF(STRICT_DEPENDECIES)
ENDIF(WITH_JPEG)

IF(JPEG_FOUND)
  DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_JPG_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
  SET(JPEG_LINK_LIBS ${MIA2DLIBS} ${JPEG_LIBRARIES})
  INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})

  PLUGIN_WITH_PREFIX2("2dimage" "io" jpg-gray "${JPEG_LINK_LIBS}")
  PLUGIN_WITH_PREFIX2("rgbimage" "io" jpg-rgb "${JPEG_LINK_LIBS}" "${rgbimage2dio_path}")
ENDIF(JPEG_FOUND)
