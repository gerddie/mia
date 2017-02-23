/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/core/export_handler.hh>
#include <mia/2d/rgbimageio.hh>

#include <mia/core/ioplugin.cxx>
#include <mia/core/iohandler.cxx>

#include <mia/core/msgstream.hh>


NS_MIA_BEGIN
using namespace std; 


const char *CRGB2DImage::data_descr = "rgbimage";

CRGB2DImage::CRGB2DImage(const C2DBounds& size):
        m_size(size), 
	m_pixels(size.x * size.y * 3)
{
	cvdebug() << "Allocated buffer of " << size.x * size.y * 3 << " bytes\n"; 
}

const C2DBounds& CRGB2DImage::get_size() const
{
	return m_size; 
}

const unsigned char *CRGB2DImage::pixel() const
{
	return &m_pixels[0]; 
}


unsigned char *CRGB2DImage::pixel()
{
	return &m_pixels[0]; 
}

CRGB2DImage *CRGB2DImage::clone() const
{
	return new CRGB2DImage(*this); 
}

template <> const char *  const 
	TPluginHandler<C2DRGBImageIOPlugin>::m_help =  
       "These plug-ins implement the support for (loading?) and storing 2D RGB images to various file types.";

template class TPlugin<CRGB2DImage, io_plugin_type>; 
template class TIOPlugin<CRGB2DImage>;
template class THandlerSingleton<TIOPluginHandler<C2DRGBImageIOPlugin> >;
template class TIOPluginHandler<C2DRGBImageIOPlugin>;
template class TPluginHandler<C2DRGBImageIOPlugin>;


bool save_image(const std::string& filename, const CRGB2DImage& image)
{
	return C2DRGBImageIOPluginPluginHandler::instance().save(filename, image); 
}

NS_MIA_END
