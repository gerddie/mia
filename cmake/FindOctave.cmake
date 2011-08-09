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


FIND_PROGRAM(MKOCTFILE mkoctfile)
IF(MKOCTFILE_FOUND)

  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p INCFLAGS    RESULT_VARIABLE R1 OUTPUT_VARIABLE OCT_INCLUDES)
  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p LDFLAGS     RESULT_VARIABLE R2 OUTPUT_VARIABLE OCT_FDFLAGS)
  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p OCTAVE_LIBS RESULT_VARIABLE R3 OUTPUT_VARIABLE OCTAVE_LIBS)
  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p BLAS_LIBS   RESULT_VARIABLE R4 OUTPUT_VARIABLE BLAS_LIBS) 
  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p FFTW_LIBS   RESULT_VARIABLE R5 OUTPUT_VARIABLE FFTW_LIBS)
  EXECUTE_PROCESS(COMMAND ${MKOCTFILE} -p LIBS        RESULT_VARIABLE R6 OUTPUT_VARIABLE LIBS)
  
  IF (R1 AND R2 AND R3 AND R4 AND R5 AND R6)
    SET(OCTAVE_FOUND TRUE)
    SET(OCTAVE_INCLUDES ${OCT_INCLUDES})
    SET(OCTAVE_LIBS ${OCT_FDFLAGS} ${OCTAVE_LIBS} ${BLAS_LIBS} ${FFTW_LIBS} ${LIBS})
    SET(OCTAVE_SUFFIX "oct")
    SET(OCTAVE_PREFIX "")
  ENDIF(R1 AND R2 AND R3 AND R4 AND R5 AND R6 AND R7)

ENDIF(MKOCTFILE_FOUND)
