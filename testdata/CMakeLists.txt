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

SET(IMAGES  
  c.bmp
  c.png	       
  c_b.bmp
  circle.bmp
  circle.png
  circle_b.bmp
)

IF (NOT WIN32) 
  IF (NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    ADD_CUSTOM_TARGET(file-links ALL)
    FOREACH(f ${IMAGES})
      ADD_CUSTOM_TARGET(link_${f} ln -sf  ${CMAKE_CURRENT_SOURCE_DIR}/${f} ${CMAKE_CURRENT_BINARY_DIR}/${f})
      ADD_DEPENDENCIES(file-links link_${f})
    ENDFOREACH(f)
  ENDIF(NOT ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
ENDIF(NOT WIN32) 
