#define PACKAGE_NAME "@PACKAGE_NAME@"
#define PACKAGE_VERSION "@PACKAGE_VERSION@"
#define PACKAGE "@MIA_NAME@"
#define VENDOR  "@VENDOR@"
#define MIA_SOURCE_ROOT "@SOURCE_ROOT@"

#if defined(MSVC)
#  define DO_EXPORT __declspec(dllexport) 
#  define DO_IMPORT __declspec(dllimport) 
#elif  defined(__GNUC__)
#  define DO_EXPORT __attribute__((visibility("default")))
#  define DO_IMPORT	
#else
#  define DO_EXPORT
#  define DO_IMPORT
#endif

