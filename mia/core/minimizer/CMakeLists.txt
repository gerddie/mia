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

SET(GSLOPTLIBS ${MIACORE} ${GSL_LIBRARIES})
PLUGIN_WITH_TEST_AND_PREFIX2("minimizer" "singlecost" gsl "${GSLOPTLIBS}")

SET(NEEDED_LIBS miacore "${NEEDED_LIBS}")
PLUGIN_WITH_TEST_AND_PREFIX2("minimizer" "singlecost" gdsq "${NEEDED_LIBS}")

SET(NEEDED_LIBS miacore "${NEEDED_LIBS}")
PLUGIN_WITH_TEST_AND_PREFIX2("minimizer" "singlecost" gdas "${NEEDED_LIBS}")


