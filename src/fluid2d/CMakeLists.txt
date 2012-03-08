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

SET(fluid2d_SOURCES main.cc vfluid.cc elast.cc helpers.cc)

ADD_EXECUTABLE(mia-fluid2d ${fluid2d_SOURCES})
CREATE_EXE_DOCU(fluid2d)
TARGET_LINK_LIBRARIES(mia-fluid2d mia2d)
INSTALL(TARGETS mia-fluid2d RUNTIME DESTINATION "bin")

