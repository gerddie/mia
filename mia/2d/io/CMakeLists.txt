#
# This file is part of MIA - a toolbox for medical image analysis 
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


SET(image2dio_path ${PLUGIN_INSTALL_PATH}/2dimage/io)
SET(vf2dio_path ${PLUGIN_INSTALL_PATH}/2dvf/io)

ADD_CUSTOM_TARGET(2d-imageio_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${image2dio_path})
ADD_CUSTOM_TARGET(2d-vfio_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${vf2dio_path})

PLUGIN_WITH_PREFIX2("2dimage" "io" bmp "${MIA2DLIBS}")
PLUGIN_WITH_PREFIX2("2dimage" "io" raw "${MIA2DLIBS}")


SET(segset2dio
  xml)

PLUGINGROUP_WITH_TEST_AND_PREFIX2("2dmyocardsegset" "io" "${segset2dio}" mia2dmyocardperf TESTLIBS  mia2dtest)







