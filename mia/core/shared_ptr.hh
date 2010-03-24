
#include <mia/core/defines.hh>

#ifndef mia_core_shared_ptr_hh
#define mia_core_shared_ptr_hh

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <boost/shared_ptr.hpp>
#define SHARED_PTR(type) ::boost::shared_ptr< type >
#define unique_ptr auto_ptr 
#else
#include <memory>
#define SHARED_PTR(type) ::std::shared_ptr< type >
#endif



#endif
