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

SET(MINIMIZER_PLUGIN_PATH "${PLUGIN_INSTALL_PATH}/minimizer/singlecost")

ADD_CUSTOM_TARGET(min_testdir mkdir -p ${PLUGIN_TEST_ROOT}/${MINIMIZER_PLUGIN_PATH})

SET(GSLOPTLIBS ${MIACORE} ${GSL_LIBRARIES})
PLUGIN_WITH_TEST_AND_PREFIX(min gsl "${GSLOPTLIBS}" ${MINIMIZER_PLUGIN_PATH})


OPTION(WITH_NLOPT "enable support for nlopt nonlinear optimizers" ${SEARCH_LIBS_DEFAULT})

IF(WITH_NLOPT)

  IF(NOT NLOPT_LIBRARIES) 
    PKG_CHECK_MODULES(NLOPT nlopt)
  ENDIF(NOT NLOPT_LIBRARIES) 

  IF(NLOPT_LIBRARIES) 
    SET(NLOPTLIBS ${MIACORE} ${NLOPT_LIBRARIES})
    PLUGIN_WITH_TEST_AND_PREFIX(min nlopt "${NLOPTLIBS}" ${MINIMIZER_PLUGIN_PATH})
  ELSE (NLOPT_LIBRARIES)
    IF(STRICT_DEPENDECIES)
      MESSAGE(ERROR "Requested library nlopt not found and STRICT_DEPENDECIES given")
    ENDIF(STRICT_DEPENDECIES)
  ENDIF(NLOPT_LIBRARIES)
ENDIF(WITH_NLOPT)

SET(NEEDED_LIBS miacore "${NEEDED_LIBS}")
PLUGIN_WITH_TEST_AND_PREFIX(min gdsq "${NEEDED_LIBS}" ${MINIMIZER_PLUGIN_PATH})



