#
# Copyright (c) Madrid 2010
# BIT, ETSI Telecomunicacion, UPM
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
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

SET(GSLPP_SOURCE 
  matrix.cc
  multimin.cc
  vector.cc
  )

SET(GSLPP_HEADERS 
  matrix.hh
  multimin.hh
  vector.hh
  vector_template.hh
  gsldefines.hh
  )

ADD_LIBRARY(miagsl SHARED ${GSLPP_SOURCE})
TARGET_LINK_LIBRARIES(miagsl ${GSL_LIBRARIES})
SET_TARGET_PROPERTIES(miagsl PROPERTIES  VERSION ${LIBRARY_VERSION_INFO})

INSTALL_WITH_EXPORT(miagsl)

INSTALL(FILES ${GSLPP_HEADERS} DESTINATION "${INCLUDE_INSTALL_PATH}/gsl++")

MACRO(GSL_TEST name)
  ADD_EXECUTABLE(test-${name} test_${name}.cc)
  TARGET_LINK_LIBRARIES(test-${name} miagsl ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
  ADD_TEST(${name} test-${name})
ENDMACRO(GSL_TEST name)

IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  GSL_TEST(vector)
  GSL_TEST(matrix)
  GSL_TEST(multimin)
ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)


