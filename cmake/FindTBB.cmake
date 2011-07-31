


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
