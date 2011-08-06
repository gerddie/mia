#
# Copyright (c) Leipzig, Madrid 2011
#
# BIT, ETSI Telecomunicacion, UPM
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

IF (FALSE)
IF(NOT HAVE_BOOST_PORTABLE_BINARY_ARCHIVE)
  SET(PORTABLE_BINARY_ARCHIVE_SRC 
    portable_binary_iarchive.cpp  
    portable_binary_oarchive.cpp)
  
  ADD_LIBRARY(portable_binary_archive STATIC ${PORTABLE_BINARY_ARCHIVE_SRC})
  set_source_files_properties(${PORTABLE_BINARY_ARCHIVE_SRC}  
    PROPERTIES COMPILE_FLAGS "-fPIC")
  
  TARGET_LINK_LIBRARIES(portable_binary_archive ${Boost_SERIALIZATION})
ENDIF(NOT HAVE_BOOST_PORTABLE_BINARY_ARCHIVE)
ENDIF(FALSE)
