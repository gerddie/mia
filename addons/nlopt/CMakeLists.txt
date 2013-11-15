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

OPTION(WITH_NLOPT "enable support for nlopt nonlinear optimizers" ${SEARCH_LIBS_DEFAULT})
IF(WITH_NLOPT)

  IF(NOT NLOPT_LIBRARIES) 
    PKG_CHECK_MODULES(NLOPT nlopt)
  ENDIF(NOT NLOPT_LIBRARIES) 

  IF(NLOPT_LIBRARIES) 
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_NLOPT_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    SET(NLOPTLIBS miacore ${NLOPT_LIBRARIES})
    PLUGIN_WITH_TEST_AND_PREFIX2("minimizer" "singlecost" nlopt "${NLOPTLIBS}")
  ELSE (NLOPT_LIBRARIES)
    IF(STRICT_DEPENDECIES)
      MESSAGE(ERROR "Requested library nlopt not found and STRICT_DEPENDECIES given")
    ENDIF(STRICT_DEPENDECIES)
  ENDIF(NLOPT_LIBRARIES)
ENDIF(WITH_NLOPT)
