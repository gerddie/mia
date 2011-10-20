# mia pkg-config source file

prefix=@prefix@
exec_prefix=${prefix}
libdir=${prefix}/@LIBRARY_INSTALL_PATH@
includedir=${prefix}/@INCLUDE_INSTALL_PATH@

pluginroot=@PLUGIN_SEARCH_PATH@
TestLibs= 

Name: mia2dperfusion
Description: A library for 2D perfusion series processing 
Version: @PACKAGE_VERSION@
Conflicts:
Requires: mia2d-@VERSION@
Libs: -lmia2dmyocardperf-@VERSION@ -L${prefix}/@LIBRARY_INSTALL_PATH@
Cflags: -I${prefix}/@INCLUDE_INSTALL_PATH@ -I${prefix}/@LIB_INCLUDE_INSTALL_PATH@
