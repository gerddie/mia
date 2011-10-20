# mia pkg-config source file

prefix=@prefix@
exec_prefix=${prefix}
libdir=${prefix}/@LIBRARY_INSTALL_PATH@
includedir=${prefix}/@INCLUDE_INSTALL_PATH@

pluginroot=@PLUGIN_SEARCH_PATH@
TestLibs= 

Name: mia2d
Description: A library for 2D grayscale image processing 
Version: @PACKAGE_VERSION@
Conflicts:
Requires: miacore-@VERSION@
Libs: -lmia2d-@VERSION@ -L${prefix}/@LIBRARY_INSTALL_PATH@
Cflags: -I${prefix}/@INCLUDE_INSTALL_PATH@ -I${prefix}/@LIB_INCLUDE_INSTALL_PATH@
