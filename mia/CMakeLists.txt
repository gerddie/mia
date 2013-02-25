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

SET(MIA_HEADERS core.hh 2d.hh 3d.hh mesh.hh)

IF (WIN32)
ADD_SUBDIRECTORY(core ${OUTPUT_ROOT})
ADD_SUBDIRECTORY(2d   ${OUTPUT_ROOT})
ADD_SUBDIRECTORY(3d   ${OUTPUT_ROOT})
ADD_SUBDIRECTORY(mesh ${OUTPUT_ROOT})
ELSE (WIN32)
ADD_SUBDIRECTORY(core)
ADD_SUBDIRECTORY(2d  )
ADD_SUBDIRECTORY(3d  )
ADD_SUBDIRECTORY(mesh)
ADD_SUBDIRECTORY(template)
ENDIF(WIN32)


INSTALL(FILES ${MIA_HEADERS}
		DESTINATION "${INCLUDE_INSTALL_PATH}/mia")

INSTALL(DIRECTORY test internal DESTINATION "${INCLUDE_INSTALL_PATH}/mia" PATTERN "*.hh" )

