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

INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")

ADD_SUBDIRECTORY(dicom   )
ADD_SUBDIRECTORY(openexr )
ADD_SUBDIRECTORY(vtk   )
ADD_SUBDIRECTORY(vistaio )
