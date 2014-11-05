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

SET(fluid3d_SOURCES 
  eqn_solver.cc
  main.cc
  sor_solver.cc
  vfluid.cc
  )
ADD_DEFINITIONS(-DVSTREAM_DOMAIN=\"fluid3d\")

ADD_EXECUTABLE(mia-3dfluid ${fluid3d_SOURCES})
TARGET_LINK_LIBRARIES(mia-3dfluid mia3d)
INSTALL(TARGETS mia-3dfluid RUNTIME DESTINATION "bin")

MIA_EXE_CREATE_DOCU_AND_INTERFACE(3dfluid)




