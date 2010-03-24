#ifndef mia_3d_defines_hh
#define mia_3d_defines_hh

#ifdef WIN32
#  include <cstdlib>
   inline double drand48() {
	return double(rand()) / RAND_MAX; 
   }
#endif

#ifdef WIN32
#  ifdef mia3d_EXPORTS
#    define EXPORT_3D __declspec(dllexport) 
#  else
#    define EXPORT_3D __declspec(dllimport) 
#  endif
#  ifdef mia3dtest_EXPORTS
#    define EXPORT_3DTEST __declspec(dllexport) 
#  else
#    define EXPORT_3DTEST __declspec(dllimport) 
#  endif
#else
#  define EXPORT_3D
#  define EXPORT_3DTEST
#endif

#ifdef mia3d_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM  
#  endif 
#  define VSTREAM "MIA3D"
#endif

#ifdef mia3dtest_EXPORTS
#  ifdef VSTREAM 
#    undef VSTREAM  
#  endif 
#  define VSTREAM "MIA3DTEST"
#endif

#endif
