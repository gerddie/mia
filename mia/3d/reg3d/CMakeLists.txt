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

# this is not very nice, we assume if it's not 64 bit it doesn't support SSE 
IF (NOT MSVC AND LONG_64BIT)
  SET(models3d ${models3d} ${SSE_MODELS})
ENDIF (NOT MSVC AND LONG_64BIT)

PLUGIN_GROUP_PRE("3d-model" "${models3d}" "${REG3DLIBS}" "${PLUGIN_INSTALL_PATH}/3dimage/model")

SET(timesteps3d
  fluid
  direct
)

PLUGIN_GROUP_PRE("3d-timestep" "${timesteps3d}" "${REG3DLIBS}" "${PLUGIN_INSTALL_PATH}/3dimage/timestep")

