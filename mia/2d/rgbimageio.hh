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

#ifndef mia_2d_rgbimageio_hh
#define mia_2d_rgbimageio_hh

#include <vector>
#include <mia/2d/defines2d.hh>
#include <mia/2d/vector.hh>
#include <mia/core/iodata.hh>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>

NS_MIA_BEGIN

/*
  \brief Quick and dirty class to store RGB unsigned char images

  This class is a very shallow inmplementation of a 2D RGB image that may
  only be used to hold tha data and save it. Main purpoise is its use to create
  some nice and colorfull output.
  This implementation has nothing to do with T2DImage.

*/


class EXPORT_2D CRGB2DImage : public mia::CIOData
{
public:
       /// unifying pointer definition for this class.
       typedef std::shared_ptr<CRGB2DImage> Pointer;

       /// data type description used by the plug-in handler to locate the plug-ins
       static const char *data_descr;

       /// data type for the plug-in handler magic
       typedef CRGB2DImage type;

       /**
          Create an image of the given size
          \param size
        */
       CRGB2DImage(const mia::C2DBounds& size);

       /// \returns the dimensions of the image
       const mia::C2DBounds& get_size() const;

       /// \returns a raw read-only pointer to the pixel data
       const unsigned char *pixel() const;

       /// \returns a raw read-write pointer to the pixel data
       unsigned char *pixel();

       /* provide a cloing mechanism for this image. This is only needed because
          we use the plug-in handler infrastructure to provide the IO for this class
          and the handler needs to be able to clone the data for internal storage.
       */
       CRGB2DImage *clone() const;
private:
       mia::C2DBounds m_size;
       std::vector<unsigned char> m_pixels;

};


/// The shared pointer type of the image
typedef CRGB2DImage::Pointer PRGB2DImage;

/// Base type for 2D RGB image IO plugins
typedef TIOPlugin<CRGB2DImage> C2DRGBImageIOPlugin;

extern template class EXPORT_2D TPlugin<CRGB2DImage, io_plugin_type>;
extern template class EXPORT_2D TIOPlugin<CRGB2DImage>;
/**
   @ingroup io
   \brief The 2D RGB image plugin handler
*/
typedef mia::THandlerSingleton< mia::TIOPluginHandler<C2DRGBImageIOPlugin>> C2DRGBImageIOPluginPluginHandler;

/**
   @ingroup io
   \brief The data key type for the CDatapool storage
*/
typedef C2DRGBImageIOPluginPluginHandler::Instance::DataKey C2DRGBImageDataKey;

/**
   Save an RGB image to the file with the given file name. The type of the image
   file is determined by the filename extension.
   \param filename
   \param image image to be saved
   \returns true if the image was saved successfully. If the saving failes,
   the function may also throw some exception providing an error message.
 */
bool EXPORT_2D save_image(const std::string& filename, const CRGB2DImage& image);

NS_MIA_END

#endif
