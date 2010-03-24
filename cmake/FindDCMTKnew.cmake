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
#
# Written for VXL by Amitha Perera.
# 

FIND_PATH( DCMTK_config_INCLUDE_DIR dcmtk/config/osconfig.h
  ${DCMTK_DIR}/ ${DCMTK_DIR}/include /usr/include
  )

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
      SET(files "${files}" "${t_${n}}")
    ENDFOREACH(n)
    SET(${target} "${files}")
  ENDMACRO(FIND_DCMTKLIBS target name)

  
  SET(NAMES ofstd dcmdata dcmimgle dcmjpeg ijg12 ijg16 ijg8)
  FIND_DCMTKLIBS(DCMTK_LIBS "${NAMES}")
  SET(DCMTK_LIBRARIES ${DCMTK_LIBS} ${ZLIB_LIBRARIES})

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
