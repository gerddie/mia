#
# Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

FIND_PATH(TBB_INCLUDE_DIR tbb/tbb.h 
  /usr/include
  )

find_library(TBB_LIBRARY tbb  
  /usr/lib
  )
find_library(TBB_LIBRARY_MALLOC tbbmalloc  
  /usr/lib
  )

find_library(TBB_LIBRARY_DEBUG tbb_debug  
  /usr/lib
  )
find_library(TBB_LIBRARY_MALLOC_DEBUG tbb_malloc_debug  
  /usr/lib
  )

#Extract path from TBB_LIBRARY name
get_filename_component(TBB_LIBRARY_DIR ${TBB_LIBRARY} PATH)
mark_as_advanced(TBB_LIBRARY TBB_MALLOC_LIBRARY TBB_LIBRARY_DEBUG TBB_MALLOC_LIBRARY_DEBUG)

set (TBB_FOUND "NO")
if (TBB_INCLUDE_DIR)
    if (TBB_LIBRARY)
        set (TBB_FOUND "YES")
        set (TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY} ${TBB_LIBRARIES})
        set (TBB_DEBUG_LIBRARIES ${TBB_LIBRARY_DEBUG} ${TBB_MALLOC_LIBRARY_DEBUG} ${TBB_DEBUG_LIBRARIES})
        set (TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
        set (TBB_LIBRARY_DIRS ${TBB_LIBRARY_DIR} CACHE PATH "TBB library directory" FORCE)
        mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARY_DIRS TBB_LIBRARIES TBB_DEBUG_LIBRARIES)
    endif (TBB_LIBRARY)
endif (TBB_INCLUDE_DIR)

if (NOT TBB_FOUND)
    message("ERROR: Intel TBB NOT found!")
    message(STATUS "Looked for Threading Building Blocks in ${_TBB_INSTALL_DIR}")
    # do only throw fatal, if this pkg is REQUIRED
    if (TBB_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find TBB library.")
    endif (TBB_FIND_REQUIRED)
endif (NOT TBB_FOUND)
