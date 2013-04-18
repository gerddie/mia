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

SET(REG3DLIBS mia3d)

SET(models3d 
  navier
  naviera
)

SET(SSE_MODELS 
  navierasse navierpsse
)

IF (NOT MSVC AND (BUILD_HAS_ALTIVEC_ENABLED OR BUILD_HAS_SSE_ENABLED))
  SET(models3d ${models3d} ${SSE_MODELS})
ENDIF (NOT MSVC AND (BUILD_HAS_ALTIVEC_ENABLED OR BUILD_HAS_SSE_ENABLED))

PLUGINGROUP_WITH_PREFIX2("3dimage" "model" "${models3d}" "${REG3DLIBS}")

SET(timesteps3d
  fluid
  direct
)

PLUGINGROUP_WITH_PREFIX2("3dimage" "timestep" "${timesteps3d}" "${REG3DLIBS}")


