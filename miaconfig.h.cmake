#cmakedefine HAVE_UNISTD_H 1
#cmakedefine WORDS_BIGENDIAN 1
#cmakedefine HAVE_LIBXMLPP 1
#cmakedefine ICA_ALWAYS_USE_SYMM 1 

#define SOURCE_ROOT "@SOURCE_ROOT@"
#define PLUGIN_SEARCH_PATH  "@PLUGIN_SEARCH_PATH@"
#define PLUGIN_INSTALL_PATH  "@PLUGIN_INSTALL_PATH@"


@HAVE_INT64@
@LONG_IS_INT64@
#ifdef HAVE_INT64
typedef signed @INT64@ mia_int64;
typedef unsigned @INT64@ mia_uint64;
#endif

#cmakedefine ICA_ALWAYS_USE_SYMM 0