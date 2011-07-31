#ifndef mia_config_h
#define mia_config_h

#ifdef PACKAGE_NAME
#undef PACKAGE_NAME
#endif
#define PACKAGE_NAME "@PACKAGE_NAME@"

#ifdef PACKAGE_VERSION
#undef PACKAGE_VERSION
#endif
#define PACKAGE_VERSION "@PACKAGE_VERSION@"

#ifdef PACKAGE
#undef PACKAGE
#endif
#define PACKAGE "@MIA_NAME@"
#define VENDOR  "@VENDOR@"
#define MIA_SOURCE_ROOT "@SOURCE_ROOT@"
#define MIA_BUILD_ROOT "@BUILD_ROOT@"

#if defined(_MSC_VER)
#  define DO_EXPORT __declspec(dllexport) 
#  define DO_IMPORT __declspec(dllimport) 
#elif  defined(__GNUC__)
#  define DO_EXPORT __attribute__((visibility("default")))
#  define DO_IMPORT __attribute__((visibility("default")))	
#else
#  define DO_EXPORT
#  define DO_IMPORT
#endif

#endif
