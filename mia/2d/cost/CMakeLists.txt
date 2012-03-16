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

SET(costplugins2dNew ssd mi ngf lsd)
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-cost "${costplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/2dimage/cost"
  )


SET(fatcostplugins2dNew fatngf fatssd) 
PLUGINGROUP_WITH_TEST_AND_PREFIX(2d-fatcost "${fatcostplugins2dNew}" 
  "${MIA2DLIBS}" "${PLUGIN_INSTALL_PATH}/2dimage/fatcost"
  )

