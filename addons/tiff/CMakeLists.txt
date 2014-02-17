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

OPTION(WITH_TIFF "enable support for TIFF gray scale images IO and RGB saving" ${SEARCH_LIBS_DEFAULT})

IF(WITH_TIFF)
  IF(STRICT_DEPENDECIES)
    FIND_PACKAGE(TIFF REQUIRED)
  ELSE(STRICT_DEPENDECIES)
    FIND_PACKAGE(TIFF)
  ENDIF(STRICT_DEPENDECIES)
ENDIF(WITH_TIFF)

IF(TIFF_FOUND)
  DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_TIFF_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
  SET(TIFF_LINK_LIBS ${MIA2DLIBS} ${TIFF_LIBRARIES})
  PLUGIN_WITH_PREFIX2("2dimage" "io" tiff "${TIFF_LINK_LIBS}")
  INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
ENDIF(TIFF_FOUND)

