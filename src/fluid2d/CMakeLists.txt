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

SET(fluid2d_SOURCES main.cc vfluid.cc elast.cc helpers.cc)

ADD_EXECUTABLE(mia-2dfluid ${fluid2d_SOURCES})
TARGET_LINK_LIBRARIES(mia-2dfluid mia2d)
CREATE_EXE_DOCU(2dfluid)
INSTALL(TARGETS mia-2dfluid RUNTIME DESTINATION "bin")

