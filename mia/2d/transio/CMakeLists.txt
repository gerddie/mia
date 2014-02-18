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

SET(io2dtransform_serialized
  bbs 
  xml
)

SET(SERLIBS mia2d ${Boost_SERIALIZATION_LIBRARY})

PLUGINGROUP_WITH_PREFIX2(
  "2dtransform" "io" 
  "${io2dtransform_serialized}" 
  "${SERLIBS}" 
)

IF(FALSE)
SET(PORT_SERLIBS mia2d portable_binary_archive ${Boost_SERIALIZATION_LIBRARY} ${Boost_SYSTEM})
PLUGINGROUP_WITH_TEST_AND_PREFIX2(
  "2dtransform" "io" 
  "2d" 
  "pbs" 
  "${PORT_SERLIBS}" 
)
ENDIF(FALSE)
