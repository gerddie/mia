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


## FFTW can be compiled and subsequently linked against
## various data types.
## There is a single set of include files, and then muttiple libraries,
## One for each type.  I.e. libfftw.a-->double, libfftwf.a-->float

## The following logic belongs in the individual package
## MARK_AS_ADVANCED(USE_FFTWD)
## OPTION(USE_FFTWD "Use double precision FFTW if found" ON)
## MARK_AS_ADVANCED(USE_FFTWF)
## OPTION(USE_FFTWF "Use single precision FFTW if found" ON)


SET(FFTW_INC_SEARCHPATH
  /sw/include
  /usr/include
  /usr/local/include
  /usr/include/fftw
  /usr/local/include/fftw
  )

FIND_PATH(FFTW_INCLUDE_PATH fftw3.h ${FFTW_INC_SEARCHPATH})

IF(FFTW_INCLUDE_PATH)
  SET(FFTW_INCLUDE ${FFTW_INCLUDE_PATH})
ENDIF (FFTW_INCLUDE_PATH)

IF(FFTW_INCLUDE)
  INCLUDE_DIRECTORIES( ${FFTW_INCLUDE})
ENDIF(FFTW_INCLUDE)

GET_FILENAME_COMPONENT(FFTW_INSTALL_BASE_PATH ${FFTW_INCLUDE_PATH} PATH)

SET(FFTW_LIB_SEARCHPATH
  ${FFTW_INSTALL_BASE_PATH}/lib
  /usr/lib/fftw
  /usr/local/lib/fftw
  )

IF(WITH_FFTWD)
  MARK_AS_ADVANCED(FFTWD_LIB)
  #   OPTION(FFTWD_LIB "The full path to the fftw3 library (including the library)" )
  FIND_LIBRARY(FFTWD_LIB fftw3 ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib
  FIND_LIBRARY(FFTWD_THREADS_LIB fftw3_threads ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib only if compiled with threads support
  
  IF(FFTWD_LIB)
    SET(FFTWD_FOUND 1)
    IF(FFTWD_THREADS_LIB)
      SET(FFTWD_LIB ${FFTWD_LIB} ${FFTWD_THREADS_LIB} )
    ENDIF(FFTWD_THREADS_LIB)
  ENDIF(FFTWD_LIB)
ENDIF(WITH_FFTWD)

IF(WITH_FFTWF)
  MARK_AS_ADVANCED(FFTWF_LIB)
  #   OPTION(FFTWF_LIB "The full path to the fftw3f library (including the library)" )
  FIND_LIBRARY(FFTWF_LIB fftw3f ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib
  FIND_LIBRARY(FFTWF_THREADS_LIB fftw3f_threads ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib only if compiled with threads support
  
  IF(FFTWF_LIB)
    SET(FFTWF_FOUND 1)
    IF(FFTWF_THREADS_LIB)
      SET(FFTWF_LIB ${FFTWF_LIB} ${FFTWF_THREADS_LIB} )
    ENDIF(FFTWF_THREADS_LIB)
  ENDIF(FFTWF_LIB)
ENDIF(WITH_FFTWF)


