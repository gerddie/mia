/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_3d_3dimageui_hh
#define mia_3d_3dimageui_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/3d/image.hh>

NS_MIA_BEGIN


/**
   @ingroup io 
   @brief A class to interface  images with image IO plugins.
*/
class EXPORT_3D C3DImageVector: public std::vector< std::shared_ptr<C3DImage >> ,
		      public CIOData {
public:
	/// returns a dynamically created copy of the image vector 
	C3DImageVector *clone() const __attribute__((warn_unused_result));

};

struct EXPORT_3D io_3dimage_data {
	typedef  C3DImageVector type;
	static const char *data_descr;
};
/**
   @ingroup io 
   @brief The 3D image io plugin base class 
*/
typedef TIOPlugin<io_3dimage_data> C3DImageIOPlugin;

/**
   @ingroup io
   @brief Handler for the plug-ins that store and load 3D images. 
*/
typedef THandlerSingleton<TIOPluginHandler<C3DImageIOPlugin> > C3DImageIOPluginHandler;

/**
   @ingroup io
   @brief Data key type used to load and store to the CDatapool 
*/
typedef C3DImageIOPluginHandler::Instance::DataKey C3DImageDataKey;

/**
   @ingroup io
   @brief pointer type to the images 
*/
typedef C3DImageIOPluginHandler::Instance::PData P3DImageVector;

/** 
    @cond INTERNAL 
    @ingroup test 
    @brief class to initialize the plug-in path for tests on the uninstalled library 
*/
struct EXPORT_3D C3DImageIOPluginHandlerTestPath {
	C3DImageIOPluginHandlerTestPath(); 
}; 
/// @endcond

/** 
    @ingroup convenience 
    convenience function to create an image vector from a single image 
    \param image 
    \returns the vector containing the image 
*/ 
P3DImageVector EXPORT_3D create_image3d_vector(P3DImage image);

/**
   \ingroup io
   convenience function to load a single image from a file.  
   \throws std::invalid_argument if the image could not be loaded 
   \param filename input file 
   \returns the loaded image 
 */

P3DImage  EXPORT_3D load_image3d(const std::string& filename); 


/**
   \ingroup io
   convenience function to load a single image from a file.  
   \throws std::invalid_argument if the image could not be loaded 
   \param filename input file 
   \returns the loaded image 
*/
template <> 
inline P3DImage  EXPORT_3D load_image<P3DImage>(const std::string& filename)
{
	cvdebug() << "Load image " << filename << "\n"; 
	return load_image3d(filename); 
}

/**
   \ingroup io
   convenience function to save a single image to a file.  
   \param filename input file 
   \param image image to be saved 
   \returns true if successful and false otherwise 
 */

bool  EXPORT_3D save_image(const std::string& filename, P3DImage image);


/**
   \ingroup io
   Convenience function to save one 3D image to a file 
   @param filename 
   @param image 
   @returns true if saving was  successfull
 */
bool  EXPORT_3D save_image(const std::string& filename, C3DImage& image);


NS_MIA_END

#endif
