#
# Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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


# - find DCMTK libraries
#

#  DCMTK_INCLUDE_DIR   - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source.


FIND_PATH( DCMTK_config_INCLUDE_DIR dcmtk/config/osconfig.h
  ${DCMTK_DIR}/ ${DCMTK_DIR}/include /usr/include
  )

# get version number 
IF (UNIX) 
  find_program(DCMDUM_PROG dcmdump)
  find_program(GREP_PROG grep)
  find_program(AWK_PROG awk)
  
  IF(NOT AWK_PROG OR NOT GREP_PROG) 
    MESSAGE(ERROR "This is supposed to be a UNIX like system but no 'awk' or no 'grep' was found")
  ENDIF(NOT AWK_PROG OR NOT GREP_PROG) 
  
  IF(DCMDUM_PROG)
    execute_process( 
      COMMAND ${DCMDUM_PROG} --version
      COMMAND ${GREP_PROG} dcmtk
      COMMAND ${AWK_PROG} "{ print $3 }"
      OUTPUT_VARIABLE DCMTK_version_string)
    string(REGEX REPLACE "[v.]" "0" DCMTK_version_string2 ${DCMTK_version_string} )
    string(COMPARE LESS ${DCMTK_version_string2} "030600" VERSION_BEFORE_360)
  ENDIF(DCMDUM_PROG) 
ELSE(UNIX)
ENDIF(UNIX) 

FIND_PACKAGE(ZLIB)

IF (ZLIB_FOUND) 
  
  MACRO(FIND_DCMTKLIBS target names)
    FOREACH(n ${names})
      FIND_LIBRARY(t_${n} NAMES "${n}" PATHS
	${DCMTK_DIR}/ofstd/libsrc
	${DCMTK_DIR}/ofstd/libsrc/Release
	${DCMTK_DIR}/ofstd/libsrc/Debug
	${DCMTK_DIR}/ofstd/Release
	${DCMTK_DIR}/ofstd/Debug
	${DCMTK_DIR}/lib
	)
      IF(NOT t_${n}) 
	MESSAGE(STATUS "library ${n} not found")
      ENDIF(NOT t_${n}) 
      SET(files "${files}" "${t_${n}}")
    ENDFOREACH(n)
    SET(${target} "${files}")
  ENDMACRO(FIND_DCMTKLIBS target name)
  
  SET(NAMES dcmdata dcmjpeg dcmimgle ofstd ijg12 ijg16 ijg8)
  FIND_DCMTKLIBS(DCMTK_REQUIRED_LIBS "${NAMES}")
  IF(VERSION_BEFORE_360) 
    SET(DCMTK_LIBS ${DCMTK_REQUIRED_LIBS})
  ELSE(VERSION_BEFORE_360)
    FIND_DCMTKLIBS(DCMTK_NEW_LIBS oflog)
    SET(DCMTK_LIBS ${DCMTK_REQUIRED_LIBS} ${DCMTK_NEW_LIBS})    
  ENDIF(VERSION_BEFORE_360) 
  

  SET(CMAKE_REQUIRED_INCLUDES ${DCMTK_config_INCLUDE_DIR})
  SET(CMAKE_REQUIRED_LIBRARIES ofstd)
  # test if ofstd requires iconv
  CHECK_CXX_SOURCE_COMPILES(
    "#include <dcmtk/ofstd/offname.h>

    int main(int argc, char *args[]) 
  {
	OFFilenameCreator c; 
        return 0; 
  }
" DCMTK_NO_ICONV)

  IF(DCMTK_NO_ICONV)
    MESSAGE(STATUS "DCMTK does not need -liconv")
    SET(DCMTK_LIBRARIES ${DCMTK_LIBS} ${ZLIB_LIBRARIES})
  ELSE()
    FIND_LIBRARY(ICONV_LIBRARY NAMES iconv)
    IF(NOT ICONV_LIBRARY) 
      SET(DCMTK_LIBRARIES ${DCMTK_LIBS} ${ZLIB_LIBRARIES})
    ELSE()
      SET(DCMTK_LIBRARIES ${DCMTK_LIBS} ${ZLIB_LIBRARIES} ${ICONV_LIBRARY})
      MESSAGE(ERROR "DCMTK required iconv, but it couldn't be found")
    ENDIF()
  ENDIF()

  IF( DCMTK_config_INCLUDE_DIR AND DCMTK_LIBS )
    
    SET( DCMTK_FOUND "YES" )
    SET( DCMTK_INCLUDE_DIR
      ${DCMTK_config_INCLUDE_DIR}
      ${ZLIB_INCLUDE_DIR}
      )
 
  ENDIF( DCMTK_config_INCLUDE_DIR AND DCMTK_LIBS )

  IF( NOT DCMTK_FOUND )
    SET( DCMTK_DIR "" CACHE PATH "Root of DCMTK source tree (optional)." )
    MARK_AS_ADVANCED( DCMTK_DIR )
  ENDIF( NOT DCMTK_FOUND )

ENDIF (ZLIB_FOUND)
