/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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
#include <mia/3d/3DImage.hh>

NS_MIA_BEGIN


/**
   A class to interface  images with image IO plugins.
*/
class EXPORT_3D C3DImageVector: public std::vector< std::shared_ptr<C3DImage >> ,
		      public CIOData {
public:
	/// returns a dynamically created copy of the image vector 
	C3DImageVector *clone() const __attribute__((warn_unused_result));

};

struct EXPORT_3D io_3dimage_type {
	typedef  C3DImageVector type;
	static const char *type_descr;
};

typedef TIOPlugin<io_3dimage_type> C3DImageIOPlugin;
typedef TIOHandlerSingleton<TIOPluginHandler<C3DImageIOPlugin> > C3DImageIOPluginHandler;
typedef C3DImageIOPluginHandler::Instance::DataKey C3DImageDataKey;
typedef C3DImageIOPluginHandler::Instance::PData P3DImageVector;

/** convenience function to create an image vector from a single image 
    \param image 
    \returns the vector containing the image 
*/ 

P3DImageVector EXPORT_3D create_image3d_vector(P3DImage image);

/**
   convenience function to load a single image from a file.  
   \throws std::invalid_argument if the image could not be loaded 
   \param filename input file 
   \returns the loaded image 
 */

P3DImage  EXPORT_3D load_image3d(const std::string& filename); 


/**
   convenience function to load a single image from a file.  
   \throws std::invalid_argument if the image could not be loaded 
   \param filename input file 
   \returns the loaded image 
*/
template <> 
inline P3DImage  EXPORT_3D load_image<P3DImage>(const std::string& filename)
{
	return load_image3d(filename); 
}

/**
   convenience function to save a single image to a file.  
   \param filename input file 
   \param image image to be saved 
   \returns true if successful and false otherwise 
 */

bool  EXPORT_3D save_image(const std::string& filename, P3DImage image);


NS_MIA_END

#endif
