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

OPTION(WITH_MAXFLOW "enable support for maxflow-mincut based filters" ${SEARCH_LIBS_DEFAULT})
IF(WITH_MAXFLOW)

  IF(NOT MAXFLOW_LIBRARIES) 
    PKG_CHECK_MODULES(MAXFLOW maxflow)
  ENDIF(NOT MAXFLOW_LIBRARIES) 

  IF(MAXFLOW_LIBRARIES) 
    DEFINE_PROPERTY(GLOBAL PROPERTY HAVE_MAXFLOW_PROP BRIEF_DOCS "yeah" FULL_DOCS "yeah")
    SET(MAXFLOWLIBS mia2d ${MAXFLOW_LIBRARIES})
    PLUGIN_WITH_TEST_AND_PREFIX2("2dimage" "filter" 2dmaxflow "${MAXFLOWLIBS}")
  ELSE (MAXFLOW_LIBRARIES)
    IF(STRICT_DEPENDECIES)
      MESSAGE(ERROR "Requested library maxflow not found and STRICT_DEPENDECIES given")
    ENDIF(STRICT_DEPENDECIES)
  ENDIF(MAXFLOW_LIBRARIES)
ENDIF(WITH_MAXFLOW)
