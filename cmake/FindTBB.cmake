# Locate Intel Threading Building Blocks include paths and libraries
# TBB can be found at http://www.threadingbuildingblocks.org/ 
# Written by Hannes Hofmann, hannes.hofmann _at_ informatik.uni-erlangen.de
# Adapted by Gino van den Bergen gino _at_ dtecta.com
# Licensed under MIT license 

# GvdB: This module uses the environment variable TBB_ARCH_PLATFORM which defines architecture and compiler.
#   e.g. "ia32/vc8" or "em64t/cc4.1.0_libc2.4_kernel2.6.16.21"
#   TBB_ARCH_PLATFORM is set by the build script tbbvars[.bat|.sh|.csh], which can be found
#   in the TBB installation directory (TBB_INSTALL_DIR).
#
# For backwards compatibility, you may explicitely set the CMake variables TBB_ARCHITECTURE and TBB_COMPILER.
# TBB_ARCHITECTURE     [ ia32 | em64t | itanium ]
#   which architecture to use
# TBB_COMPILER         e.g. vc9 or cc3.2.3_libc2.3.2_kernel2.4.21 or cc4.0.1_os10.4.9
#   which compiler to use (detected automatically on Windows)

# This module respects
# TBB_INSTALL_DIR or $ENV{TBB21_INSTALL_DIR} or $ENV{TBB_INSTALL_DIR}

# This module defines
# TBB_INCLUDE_DIRS, where to find task_scheduler_init.h, etc.
# TBB_LIBRARY_DIRS, where to find libtbb, libtbbmalloc
# TBB_INSTALL_DIR, the base TBB install directory
# TBB_LIBRARIES, the libraries to link against to use TBB.
# TBB_DEBUG_LIBRARIES, the libraries to link against to use TBB with debug symbols.
# TBB_FOUND, If false, don't try to use TBB.


#-- Clear the public variables
set (TBB_FOUND "NO")


#-- Look for include directory and set ${TBB_INCLUDE_DIR}
find_path(TBB_INCLUDE_DIR
    tbb/task_scheduler_init.h
    PATHS 
    /usr/include
    /opt/intel/tbb/include
    /usr/local/include
)
mark_as_advanced(TBB_INCLUDE_DIR)


find_library(TBB_LIBRARY tbb 
  /usr/lib
  /opt/intel/tbb/lib
  /usr/local/lib
  )
find_library(TBB_MALLOC_LIBRARY tbbmalloc 
  /usr/lib
  /opt/intel/tbb/lib
  /usr/local/lib
  )

find_library(TBB_LIBRARY_DEBUG tbb_debug 
  /usr/lib
  /opt/intel/tbb/lib
  /usr/local/lib
  )
find_library(TBB_MALLOC_LIBRARY_DEBUG tbbmalloc_debug
  /usr/lib
  /opt/intel/tbb/lib
  /usr/local/lib
  )


#Extract path from TBB_LIBRARY name
get_filename_component(TBB_LIBRARY_DIR ${TBB_LIBRARY} PATH)

mark_as_advanced(TBB_LIBRARY TBB_MALLOC_LIBRARY TBB_LIBRARY_DEBUG TBB_MALLOC_LIBRARY_DEBUG)


if (TBB_INCLUDE_DIR)
  if (TBB_LIBRARY)
    set (TBB_FOUND "YES")
    set (TBB_LIBRARIES ${TBB_LIBRARY} ${TBB_MALLOC_LIBRARY})
    set (TBB_DEBUG_LIBRARIES ${TBB_LIBRARY_DEBUG} ${TBB_MALLOC_LIBRARY_DEBUG})
    set (TBB_INCLUDE_DIRS ${TBB_INCLUDE_DIR} CACHE PATH "TBB include directory" FORCE)
    set (TBB_LIBRARY_DIRS ${TBB_LIBRARY_DIR} CACHE PATH "TBB library directory" FORCE)
    mark_as_advanced(TBB_INCLUDE_DIRS TBB_LIBRARY_DIRS TBB_LIBRARIES TBB_DEBUG_LIBRARIES)
    message(STATUS "Found Intel TBB")
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

