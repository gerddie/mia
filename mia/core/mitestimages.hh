#ifndef mi_test_images_h
#define mi_test_images_h

#include <mia/core/defines.hh>

NS_MIA_BEGIN

/**
   \ingroup test 
   Simple structure for test images 
 */

struct SImageSize {
	/// image width 
	int width; 
	/// image height 
	int height; 
}; 

extern EXPORT_CORE const SImageSize mi_test_size; 
extern EXPORT_CORE const int moving_init_data[]; 
extern EXPORT_CORE const int reverence_init_data[]; 

NS_MIA_END

#endif
