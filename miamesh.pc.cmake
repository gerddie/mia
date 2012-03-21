# mia pkg-config source file

prefix=@prefix@
exec_prefix=${prefix}
libdir=${prefix}/@LIBRARY_INSTALL_PATH@
includedir=${prefix}/@INCLUDE_INSTALL_PATH@

pluginroot=@PLUGIN_SEARCH_PATH@
TestLibs= 

Name: miamesh 
Description: A library for 3D triangulat meshes 
Version: @PACKAGE_VERSION@
Conflicts:
Requires: mia3d-@VERSION@
Libs: -lmiamesh-@VERSION@ -L${prefix}/@LIBRARY_INSTALL_PATH@
Cflags: -I${prefix}/@INCLUDE_INSTALL_PATH@ -I${prefix}/@LIB_INCLUDE_INSTALL_PATH@
