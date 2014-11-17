# mia pkg-config source file

prefix=@prefix@
exec_prefix=${prefix}
libdir=${prefix}/@LIBRARY_INSTALL_PATH@
includedir=${prefix}/@INCLUDE_INSTALL_PATH@

pluginroot=@PLUGIN_SEARCH_PATH@
TestLibs= 

doctools=@MIA_DOCTOOLS_ROOT@

Name: miacore
Description: A library for 2D/3D grayscale image processing 
Version: @PACKAGE_VERSION@
Conflicts:
Requires: libxml++-2.6
Requires.private: @PKG_CONFIG_DEPS@ 
Libs: -lmiacore-@VERSION@  -L${prefix}/@LIBRARY_INSTALL_PATH@ -lboost_system 
Cflags: -I${prefix}/@INCLUDE_INSTALL_PATH@ -I@LIB_INCLUDE_INSTALL_PATH@
