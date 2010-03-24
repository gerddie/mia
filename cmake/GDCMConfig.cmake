#-----------------------------------------------------------------------------
#
# GDCMConfig.cmake - CMake configuration file for external projects.
#
# This file is configured by GDCM and used by the UseGDCM.cmake
# module to load GDCM's settings for an external project.

# Compute the installation prefix from GDCM_DIR.
SET(GDCM_INSTALL_PREFIX "${GDCM_DIR}")
GET_FILENAME_COMPONENT(GDCM_INSTALL_PREFIX "${GDCM_INSTALL_PREFIX}" PATH)
GET_FILENAME_COMPONENT(GDCM_INSTALL_PREFIX "${GDCM_INSTALL_PREFIX}" PATH)

# The GDCM version number.
SET(GDCM_MAJOR_VERSION "2")
SET(GDCM_MINOR_VERSION "0")
SET(GDCM_BUILD_VERSION "6")

# The libraries.
SET(GDCM_LIBRARIES "")

# The CMake macros dir.
SET(GDCM_CMAKE_DIR "${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/CMake")

# The configuration options.
SET(GDCM_BUILD_SHARED_LIBS "ON")
SET(GDCM_USE_VTK "OFF")

# The C and C++ flags added to the cmake-configured flags.
SET(GDCM_REQUIRED_C_FLAGS
  "")
SET(GDCM_REQUIRED_CXX_FLAGS
  "")
SET(GDCM_REQUIRED_EXE_LINKER_FLAGS
  "")
SET(GDCM_REQUIRED_SHARED_LINKER_FLAGS
  "")
SET(GDCM_REQUIRED_MODULE_LINKER_FLAGS
  "")

# The "use" file.
SET(GDCM_USE_FILE "${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/UseGDCM.cmake")

# The build settings file.
SET(GDCM_BUILD_SETTINGS_FILE "${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/GDCMBuildSettings.cmake")

# The library directories.
SET(GDCM_LIBRARY_DIRS "${GDCM_INSTALL_PREFIX}/lib")

# The runtime directories.
# Note that if GDCM_CONFIGURATION_TYPES is set (see below) then
# these directories will be the parent directories under which there will
# be a directory of runtime binaries for each configuration type.
SET(GDCM_RUNTIME_DIRS "${GDCM_INSTALL_PREFIX}/lib")

# The include directories.
SET(GDCM_INCLUDE_DIRS "${GDCM_INSTALL_PREFIX}/include/gdcm-2.0")

# The library dependencies file.
IF(NOT GDCM_NO_LIBRARY_DEPENDS AND 
    EXISTS "${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/GDCMLibraryDepends.cmake")
  INCLUDE("${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/GDCMLibraryDepends.cmake")
ENDIF(NOT GDCM_NO_LIBRARY_DEPENDS AND 
  EXISTS "${GDCM_INSTALL_PREFIX}/lib/gdcm-2.0/GDCMLibraryDepends.cmake")

# The VTK options.
IF(GDCM_USE_VTK)
  SET(GDCM_VTK_DIR "")
ENDIF(GDCM_USE_VTK)

# An install tree always provides one build configuration.
# A build tree may provide either one or multiple build
# configurations depending on the CMake generator used. Since
# this project can be used either from a build tree or an install tree it
# is useful for outside projects to know the configurations available.
# If this GDCMConfig.cmake is in an install tree
# GDCM_CONFIGURATION_TYPES will be empty and GDCM_BUILD_TYPE
# will be set to the value of CMAKE_BUILD_TYPE used to build
# GDCM. If GDCMConfig.cmake is in a build tree
# then GDCM_CONFIGURATION_TYPES and GDCM_BUILD_TYPE will
# have values matching CMAKE_CONFIGURATION_TYPES and CMAKE_BUILD_TYPE
# for that build tree (only one will ever be set).
SET(GDCM_CONFIGURATION_TYPES )
SET(GDCM_BUILD_TYPE )

