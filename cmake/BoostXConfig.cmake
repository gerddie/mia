# - Find the Boost includes and libraries.
# The following variables are set if Boost is found.  If Boost is not
# found, Boost_FOUND is set to false.
#  Boost_FOUND                  - True when the Boost include directory is found.
#  Boost_INCLUDE_DIRS           - the path to where the boost include files are.
#  Boost_LIBRARY_DIRS           - The path to where the boost library files are.
#  Boost_LIB_DIAGNOSTIC_DEFINITIONS - Only set if using Windows.
#  Boost_<library>_FOUND        - True if the Boost <library> is found.
#  Boost_<library>_INCLUDE_DIRS - The include path for Boost <library>.
#  Boost_<library>_LIBRARIES    - The libraries to link to to use Boost <library>.
#  Boost_LIBRARIES              - The libraries to link to to use all Boost libraries.
#
# The following variables can be set to configure how Boost is found:
#  Boost_LIB_PREFIX             - Look for Boost libraries prefixed with this, e.g. "lib"
#  Boost_LIB_SUFFIX             - Look for Boost libraries ending with this, e.g. "vc80-mt"
#  Boost_LIB_SUFFIX_DEBUG       - As for Boost_LIB_SUFFIX but for debug builds, e.g. "vs80-mt-gd"

# ----------------------------------------------------------------------------
# If you have installed Boost in a non-standard location or you have
# just staged the boost files using bjam then you have three
# options. In the following comments, it is assumed that <Your Path>
# points to the root directory of the include directory of Boost. e.g
# If you have put boost in C:\development\Boost then <Your Path> is
# "C:/development/Boost" and in this directory there will be two
# directories called "include" and "lib".
# 1) After CMake runs, set Boost_INCLUDE_DIR to <Your Path>/include/boost<-version>
# 2) Use CMAKE_INCLUDE_PATH to set a path to <Your Path>/include. This will allow FIND_PATH()
#    to locate Boost_INCLUDE_DIR by utilizing the PATH_SUFFIXES option. e.g.
#    SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "<Your Path>/include")
# 3) Set an environment variable called ${BOOST_ROOT} that points to the root of where you have
#    installed Boost, e.g. <Your Path>. It is assumed that there is at least a subdirectory called
#    include in this path.
#
# Note:
#  1) If you are just using the boost headers, then you do not need to use
#     Boost_LIBRARY_DIRS in your CMakeLists.txt file.
#  2) If Boost has not been installed, then when setting Boost_LIBRARY_DIRS
#     the script will look for /lib first and, if this fails, then for /stage/lib.
#
# Usage:
# In your CMakeLists.txt file do something like this:
# ...
# # Boost
# FIND_PACKAGE(Boost)
# ...
# INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
# LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
#
# In Windows, we make the assumption that, if the Boost files are installed, the default directory
# will be C:\boost.

#
# TODO:
#
# 1) Automatically find the Boost library files and eliminate the need
#    to use Link Directories.
#

SET(BOOST_INCLUDE_PATH_DESCRIPTION "directory containing the boost include files. E.g /usr/local/include/boost-1_34_1 or c:\\boost\\include\\boost-1_34_1")

IF(WIN32)
  # In windows, automatic linking is performed, so you do not have to specify the libraries.
  # If you are linking to a dynamic runtime, then you can choose to link to either a static or a
  # dynamic Boost library, the default is to do a static link.  You can alter this for a specific
  # library "whatever" by defining BOOST_WHATEVER_DYN_LINK to force Boost library "whatever" to
  # be linked dynamically.  Alternatively you can force all Boost libraries to dynamic link by
  # defining BOOST_ALL_DYN_LINK.

  # This feature can be disabled for Boost library "whatever" by defining BOOST_WHATEVER_NO_LIB,
  # or for all of Boost by defining BOOST_ALL_NO_LIB.

  # If you want to observe which libraries are being linked against then defining
  # BOOST_LIB_DIAGNOSTIC will cause the auto-linking code to emit a #pragma message each time
  # a library is selected for linking.
  SET(Boost_LIB_DIAGNOSTIC_DEFINITIONS "-DBOOST_LIB_DIAGNOSTIC")
ENDIF(WIN32)

SET(BOOST_DIR_MESSAGE "Set the Boost_INCLUDE_DIR cmake cache entry to the ${BOOST_INCLUDE_PATH_DESCRIPTION}")


IF(WIN32)
  #SET(BOOST_VERSIONS 1.34.1 1.34.2 1.34.3 1.35 1.35.0 1.35.1)
  #FOREACH(ver ${BOOST_VERSIONS})
	GET_FILENAME_COMPONENT(BOOST_ROOT [HKEY_LOCAL_MACHINE\\SOFTWARE\\Boost-Consulting.com\\1.34.1;InstallRoot] ABSOLUTE CACHE)
    SET(BOOST_DIR_SEARCH ${BOOST_DIR_SEARCH} ${BOOST_ROOT})
  #ENDFOREACH(ver)
  MESSAGE(STATUS ${BOOST_DIR_SEARCH})
  FIND_PATH(Boost_INCLUDE_DIR NAMES boost/config.hpp 
      PATHS ${BOOST_DIR_SEARCH}
      # Help the user find it if we cannot.
      DOC "The ${BOOST_INCLUDE_PATH_DESCRIPTION}"
   )
ELSE(WIN32)
  SET(BOOST_DIR_SEARCH $ENV{BOOST_ROOT})
  IF(BOOST_DIR_SEARCH)
     FILE(TO_CMAKE_PATH ${BOOST_DIR_SEARCH} BOOST_DIR_SEARCH)
     SET(BOOST_DIR_SEARCH ${BOOST_DIR_SEARCH}/include)
   ENDIF(BOOST_DIR_SEARCH)

  # Add in some path suffixes. These will have to be updated whenever a new Boost version comes out.
  SET(SUFFIX_FOR_PATH
    boost-1_35_1
    boost-1_35_0
    boost-1_35
    boost-1_34_1
  )

  #
  # Look for an installation.
  #
  FIND_PATH(Boost_INCLUDE_DIR NAMES boost/config.hpp PATH_SUFFIXES ${SUFFIX_FOR_PATH} 
	PATHS /usr/include /usr/local/include 
	
     # Help the user find it if we cannot.
     DOC "The ${BOOST_INCLUDE_PATH_DESCRIPTION}"
   )
ENDIF(WIN32)

IF (MINGW OR MSYS)
  SET(Boost_LIB_SUFFIX "mgw-mt")
  SET(Boost_LIB_SUFFIX_DEBUG "mgw-mt-d")
ELSEIF (APPLE)
  SET(Boost_LIB_SUFFIX "")
  SET(Boost_LIB_SUFFIX_DEBUG "d")
ELSEIF (UNIX)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(Boost_LIB_SUFFIX "mt")
    SET(Boost_LIB_SUFFIX_DEBUG "mt-d")
  ELSE(CMAKE_COMPILER_IS_GNUCXX)
    SET(Boost_LIB_SUFFIX "il")
    SET(Boost_LIB_SUFFIX_DEBUG "il-d")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ELSEIF (MSVC80)
  SET(Boost_LIB_SUFFIX "vc80-mt")
  SET(Boost_LIB_SUFFIX_DEBUG "vc80-mt-gd")
ENDIF (MINGW OR MSYS)



# Assume we didn't find it.
SET(Boost_FOUND 0)

# Now try to get the include and library path.
IF(Boost_INCLUDE_DIR)

  # Look for the boost library path.
  # Note that the user may not have installed any libraries
  # so it is quite possible the Boost_LIBRARY_PATH may not exist.
  SET(Boost_LIBRARY_DIR ${Boost_INCLUDE_DIR})

  IF("${Boost_LIBRARY_DIR}" MATCHES "boost-[0-9]+")
    GET_FILENAME_COMPONENT(Boost_LIBRARY_DIR ${Boost_LIBRARY_DIR} PATH)
  ENDIF ("${Boost_LIBRARY_DIR}" MATCHES "boost-[0-9]+")

  IF("${Boost_LIBRARY_DIR}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Boost_LIBRARY_DIR ${Boost_LIBRARY_DIR} PATH)
  ENDIF("${Boost_LIBRARY_DIR}" MATCHES "/include$")

  IF(EXISTS "${Boost_LIBRARY_DIR}/lib")
    SET (Boost_LIBRARY_DIR ${Boost_LIBRARY_DIR}/lib)
    MESSAGE(STATUS "BOOST lib dir = ${Boost_LIBRARY_DIR}")
  ELSE(EXISTS "${Boost_LIBRARY_DIR}/lib")
    IF(EXISTS "${Boost_LIBRARY_DIR}/stage/lib")
      SET(Boost_LIBRARY_DIR ${Boost_LIBRARY_DIR}/stage/lib)
    ELSE(EXISTS "${Boost_LIBRARY_DIR}/stage/lib")
      SET(Boost_LIBRARY_DIR "")
    ENDIF(EXISTS "${Boost_LIBRARY_DIR}/stage/lib")
  ENDIF(EXISTS "${Boost_LIBRARY_DIR}/lib")

  IF(EXISTS "${Boost_INCLUDE_DIR}")
    SET(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
    # We have found boost. It is possible that the user has not
    # compiled any libraries so we set Boost_FOUND to be true here.
    SET(Boost_FOUND 1)
    MARK_AS_ADVANCED(Boost_INCLUDE_DIR)

  ENDIF(EXISTS "${Boost_INCLUDE_DIR}")

  IF(Boost_LIBRARY_DIR AND EXISTS "${Boost_LIBRARY_DIR}")
    SET(Boost_LIBRARY_DIRS ${Boost_LIBRARY_DIR})
  ENDIF(Boost_LIBRARY_DIR AND EXISTS "${Boost_LIBRARY_DIR}")

  #get the BOOST library suffix
  IF("${Boost_INCLUDE_DIR}" MATCHES "[0-9]+[0-9_]*$")
    STRING(REGEX MATCH "[0-9]+[0-9_]*$" LIB_VERSION_SUFFIX "${Boost_INCLUDE_DIR}")
  ENDIF("${Boost_INCLUDE_DIR}" MATCHES "[0-9]+[0-9_]*$")

ENDIF(Boost_INCLUDE_DIR)

#
# Find boost libraries
#

# List of library suffixes to search, e.g. libboost_date_time-gcc
SET(BOOST_SUFFIX_SEARCH 
  mt
  gcc41-mt
  gcc-mt 
  gcc
  il
  mgw
)

SET(SUFFIX_FOR_NAME
 1_35_1
 1_35_0
 1_34_1
)

# List of all boost libraries
SET(BOOST_ALL_LIBRARIES 
  date_time
  filesystem
  graph
  iostreams
  program_options
  python
  regex
  serialization
  signals
#  test
  thread
  unit_test_framework    
  wave
)

SET(GCC_VERSIONS 
  gcc32
  gcc33
  gcc34
  gcc40
  gcc41
  gcc42
)

# Macro to find boost library called name
MACRO(BOOST_FIND_LIBRARY name)

  # User can specify a particular build variant via the variables:
  #   Boost_LIB_PREFIX, Boost_LIB_SUFFIX, Boost_LIB_SUFFIX_DEBUG
  # otherwise we'll search the BOOST_SUFFIX_SEARCH list

  IF (Boost_LIB_SUFFIX)
    SET(BOOST_LIB_NAMES ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX})
    IF (LIB_VERSION_SUFFIX)
      SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX}-${LIB_VERSION_SUFFIX})
    ELSE(LIB_VERSION_SUFFIX)
      FOREACH(ver ${SUFFIX_FOR_NAME})
	SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX}-${ver})
      ENDFOREACH(ver)
    ENDIF(LIB_VERSION_SUFFIX)

    IF(CMAKE_COMPILER_IS_GNUCXX)
    
    FOREACH(gccver ${GCC_VERSIONS}) 
      SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${gccver}-${Boost_LIB_SUFFIX})
      IF (LIB_VERSION_SUFFIX)
        SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${gccver}-${Boost_LIB_SUFFIX}-${LIB_VERSION_SUFFIX})
      ELSE(LIB_VERSION_SUFFIX)
        FOREACH(ver ${SUFFIX_FOR_NAME})
  	  SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${gccver}-${Boost_LIB_SUFFIX}-${ver})
        ENDFOREACH(ver)
      ENDIF(LIB_VERSION_SUFFIX)
    ENDFOREACH(gccver)
    ENDIF(CMAKE_COMPILER_IS_GNUCXX)

  ELSE (Boost_LIB_SUFFIX)
    SET(BOOST_LIB_NAMES ${Boost_LIB_PREFIX}boost_${name})
    FOREACH(suffix ${BOOST_SUFFIX_SEARCH})
      SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${suffix})
      IF (LIB_VERSION_SUFFIX)
	SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX}-${LIB_VERSION_SUFFIX})
      ELSE(LIB_VERSION_SUFFIX)
	FOREACH(ver ${SUFFIX_FOR_NAME})
	  SET(BOOST_LIB_NAMES ${BOOST_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX}-${ver})
	ENDFOREACH(ver)
      ENDIF(LIB_VERSION_SUFFIX)
    ENDFOREACH(suffix)
  ENDIF(Boost_LIB_SUFFIX)
 
  # MESSAGE(STATUS "Search library '${BOOST_LIB_NAMES}' in '${Boost_LIBRARY_DIRS}'")
  # Find the library in the Boost_LIBRARY_DIRS
  FIND_LIBRARY(Boost_${name}_LIBRARY 
    NAMES ${BOOST_LIB_NAMES}
    PATHS ${Boost_LIBRARY_DIRS})
  
  SET(BOOST_DEBUG_LIB_NAMES ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX_DEBUG})
  IF (LIB_VERSION_SUFFIX)
    SET(BOOST_DEBUG_LIB_NAMES ${BOOST_DEBUG_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX_DEBUG}-${LIB_VERSION_SUFFIX})
  ELSE(LIB_VERSION_SUFFIX)
    FOREACH(ver ${SUFFIX_FOR_NAME})
      SET(BOOST_DEBUG_LIB_NAMES ${BOOST_DEBUG_LIB_NAMES} ${Boost_LIB_PREFIX}boost_${name}-${Boost_LIB_SUFFIX_DEBUG}-${ver})
    ENDFOREACH(ver)
  ENDIF(LIB_VERSION_SUFFIX)
    
  # For MSVC builds find debug library
  FIND_LIBRARY(Boost_${name}_LIBRARY_DEBUG 
    NAMES ${BOOST_DEBUG_LIB_NAMES}
    PATHS ${Boost_LIBRARY_DIRS})
  
  IF (Boost_${name}_LIBRARY AND Boost_${name}_LIBRARY_DEBUG)
    SET(Boost_${name}_LIBRARIES debug ${Boost_${name}_LIBRARY_DEBUG} optimized ${Boost_${name}_LIBRARY})
  ELSEIF (Boost_${name}_LIBRARY)
    SET(Boost_${name}_LIBRARIES ${Boost_${name}_LIBRARY})
  ELSEIF(Boost_${name}_LIBRARY_DEBUG)
    SET(Boost_${name}_LIBRARIES ${Boost_${name}_LIBRARY_DEBUG})
  ENDIF (Boost_${name}_LIBRARY AND Boost_${name}_LIBRARY_DEBUG)

  # If we've got it setup appropriate variables or issue error message
  IF(Boost_${name}_LIBRARIES)
    SET(Boost_${name}_FOUND 1)
    SET(Boost_${name}_INCLUDE_DIRS ${Boost_INCLUDE_DIR})
    MARK_AS_ADVANCED(Boost_${name}_LIBRARY Boost_${name}_LIBRARY_DEBUG)
  ELSE(Boost_${name}_LIBRARIES)
    IF(NOT Boost_FIND_QUIETLY)
      MESSAGE(STATUS "Boost ${name} library was not found.")
    ELSE(NOT Boost_FIND_QUIETLY)
      IF(Boost_FIND_REQUIRED_${name})
        MESSAGE(FATAL_ERROR "Could NOT find required Boost ${name} library.")
      ENDIF(Boost_FIND_REQUIRED_${name})
    ENDIF(NOT Boost_FIND_QUIETLY)  
  ENDIF(Boost_${name}_LIBRARIES)
ENDMACRO(BOOST_FIND_LIBRARY)

IF(Boost_LIBRARY_DIRS)

  # If the user specified required components e.g. via 
  # FIND_PACKAGE(Boost REQUIRED date_time regex)
  # find (just) those libraries. Otherwise find all libraries.
  IF(Boost_FIND_COMPONENTS)
    SET(Boost_FIND_LIBRARIES ${Boost_FIND_COMPONENTS})
  ELSE(Boost_FIND_COMPONENTS)
    SET(Boost_FIND_LIBRARIES ${BOOST_ALL_LIBRARIES})
  ENDIF(Boost_FIND_COMPONENTS)

  SET(Boost_LIBRARIES)
  FOREACH(library ${Boost_FIND_LIBRARIES})
    BOOST_FIND_LIBRARY(${library})
    IF(Boost_${library}_FOUND)
      SET(Boost_LIBRARIES ${Boost_LIBRARIES} ${Boost_${library}_LIBRARIES})
    ENDIF(Boost_${library}_FOUND)
  ENDFOREACH(library)
ENDIF(Boost_LIBRARY_DIRS)

IF(NOT Boost_FOUND)
  IF(NOT Boost_FIND_QUIETLY)
    MESSAGE(STATUS "Boost was not found. ${BOOST_DIR_MESSAGE}")
  ELSE(NOT Boost_FIND_QUIETLY)
    IF(Boost_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Boost was not found. ${BOOST_DIR_MESSAGE}")
    ENDIF(Boost_FIND_REQUIRED)
  ENDIF(NOT Boost_FIND_QUIETLY)
ENDIF(NOT Boost_FOUND)
