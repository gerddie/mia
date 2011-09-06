# mia pkg-config source file

prefix=@prefix@
exec_prefix=${prefix}
libdir=${prefix}/@LIBRARY_INSTALL_PATH@
includedir=${prefix}/@INCLUDE_INSTALL_PATH@

pluginroot=@PLUGIN_SEARCH_PATH@
TestLibs= 

Name: mia
Description: A library for 2D/3D grayscale image processing 
Version: @PACKAGE_VERSION@
Conflicts:
Requires: @PKG_CONFIG_DEPS@ 
Libs: -lmia3d-@VERSION@ -lmia2d-@VERSION@ -lmiacore-@VERSION@ @MIA_DEPEND_LIBRARIES@ -L${prefix}/@LIBRARY_INSTALL_PATH@
Cflags: -I${prefix}/@INCLUDE_INSTALL_PATH@ -I${prefix}/@LIB_INCLUDE_INSTALL_PATH@
