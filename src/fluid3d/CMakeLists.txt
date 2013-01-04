#
# Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

SET(fluid3d_SOURCES 
  eqn_solver.cc
  main.cc
  sor_solver.cc
  vfluid.cc
  )

ADD_EXECUTABLE(mia-3dfluid ${fluid3d_SOURCES})
TARGET_LINK_LIBRARIES(mia-3dfluid mia3d)
CREATE_EXE_DOCU(3dfluid)
INSTALL(TARGETS mia-3dfluid RUNTIME DESTINATION "bin")




