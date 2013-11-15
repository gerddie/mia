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

OPTION(WITH_PNG "enable support for PNG gray scale images IO and RGB saving" ${SEARCH_LIBS_DEFAULT})

IF (WITH_PNG)
  IF(STRICT_DEPENDECIES)
    FIND_PACKAGE(PNG REQUIRED)
  ELSE(STRICT_DEPENDECIES)
    FIND_PACKAGE(PNG)
  ENDIF(STRICT_DEPENDECIES)
ENDIF(WITH_PNG)


IF(PNG_FOUND)

  DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_PNG_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")

  SET(PNG_LINK_LIBS ${MIA2DLIBS} ${PNG_LIBRARIES})
  INCLUDE_DIRECTORIES(${PNG_PNG_INCLUDE_DIR})

  PLUGIN_WITH_PREFIX2("2dimage" "io" png-gray "${PNG_LINK_LIBS}")
  PLUGIN_WITH_PREFIX2("rgbimage" "io" png-rgb "${PNG_LINK_LIBS}" )


ENDIF(PNG_FOUND)
