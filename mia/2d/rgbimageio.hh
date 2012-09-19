/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#ifndef mia_2d_rgbimageio_hh
#define mia_2drgbimageio_hh

#include <vector>
#include <mia/2d/2DVector.hh>
#include <mia/core/iodata.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>

NS_MIA_BEGIN

class CRGB2DImage : public mia::CIOData{
public: 
	typedef std::shared_ptr<CRGB2DImage> Pointer; 

	static const char *data_descr;
	typedef CRGB2DImage type; 


	CRGB2DImage(const mia::C2DBounds& size); 
	
	const mia::C2DBounds& get_size() const; 

	const unsigned char *pixel() const; 
	
	unsigned char *pixel(); 

	CRGB2DImage *clone() const; 
private:
	mia::C2DBounds m_size; 
	std::vector<unsigned char> m_pixels; 
	
};

typedef CRGB2DImage::Pointer PRGB2DImage; 

/// Base type for 2D RGB image IO plugins 
typedef mia::TIOPlugin<CRGB2DImage> C2DRGBImageIOPlugin;


/**
   @ingroup io 
   \brief The 2D RGB image plugin handler 
*/
typedef mia::TIOHandlerSingleton< mia::TIOPluginHandler<C2DRGBImageIOPlugin> > C2DRGBImageIOPluginPluginHandler;

/**
   @ingroup io 
   \brief The data key type for the CDatapool storage 
*/
typedef C2DRGBImageIOPluginPluginHandler::Instance::DataKey C2DRGBImageDataKey;

bool save_image(const std::string& filename, const CRGB2DImage& image); 

NS_MIA_END

#endif
