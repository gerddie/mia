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

#ifndef mia_2d_2dimageui_hh
#define mia_2d_2dimageui_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/core/callback.hh>

#include <mia/2d/image.hh>

NS_MIA_BEGIN

/**
   \ingroup Infrastructure
   \brief Vector of 2D images to load and store series of images to one file
 */
class EXPORT_2D C2DImageVector: public C2DImageSeries,
       public CIOData
{
public:

       /// \returns a newly created copy of the image vector
       C2DImageVector *clone() const __attribute__((warn_unused_result));
};


/// @cond INTERNAL
class EXPORT_2D io_2dimage_type
{
public:
       typedef  C2DImageVector type;
       static const char *data_descr;
};
/// @endcond

/// Base type for 2D image IO plugins
typedef TIOPlugin<io_2dimage_type> C2DImageIOPlugin;


/// @cond INTERNAL

extern template class EXPORT_2D TPlugin<io_2dimage_type, io_plugin_type>;
extern template class EXPORT_2D TIOPlugin<io_2dimage_type>;

/**
   \ingroup io
   \brief Handler class for the image IO
 */

class EXPORT_2D C2DImageIOPPH: public TIOPluginHandler<C2DImageIOPlugin>
{
public:
       typedef TIOPluginHandler<C2DImageIOPlugin>::CSuffixmap CSuffixmap;
       typedef TIOPluginHandler<C2DImageIOPlugin>::PData PData;
protected:
       C2DImageIOPPH();
};
/// @endcond


/**
    \ingroup io
    The 2D image IO plugin handler that also provides the interface to
    load and store 2D images.
 */
typedef THandlerSingleton< C2DImageIOPPH > C2DImageIOPluginHandler;

template <>
struct IOHandler_of<C2DImage> {
       typedef C2DImageIOPluginHandler type;
};


/** The type for virtual storage access for images
    \sa CDatapool
 */
typedef C2DImageIOPluginHandler::Instance::DataKey C2DImageDataKey;

/**
    Data type of what is actually loaded by the image plugins handler
 */
typedef C2DImageIOPluginHandler::Instance::PData P2DImageVector;

/**
   \ingroup convenience
   Convenience function to create a vector of images wrapping one image
   \param image
   \returns the vector containing the image
   \todo the interface that requires a vector to be passes may not be the best approach
   in ensures that we can also load all images from a multi-record image file type
   but other then that it is very inconvenient
 */
P2DImageVector EXPORT_2D create_image2d_vector(P2DImage image);


/**
   \ingroup io
   Convenience function to load one 2D image from a file
   @param filename
   @returns image
 */

P2DImage EXPORT_2D  load_image2d(const std::string& filename);

/**
   \ingroup io
   Convenience function to load one 2D image from a file
   @param filename
   @returns image
 */
template <>
inline P2DImage load_image<P2DImage>(const std::string& filename)
{
       return load_image2d(filename);
}

/**
   \ingroup io
   Convenience function to save one 2D image to a file
   @param filename
   @param image
   @returns true if saving was  successfull
 */

bool  EXPORT_2D save_image(const std::string& filename, P2DImage image);

/**
   \ingroup io
   Convenience function to save one 2D image to a file
   @param filename
   @param image
   @returns true if saving was  successfull
 */
bool  EXPORT_2D save_image(const std::string& filename, C2DImage& image);


/**
   \ingroup io
   Convenience function to load a series of images and group them into sets based
   on the acquisition parameters if available.
   If these parameters are not available, then the order of the input files is used
   to sort the files but no grouping takes place.
   \param filenames list of file names
   \param cb optionally provide a call back function that shows loading progress
   \returns grouped set of files.
*/

C2DImageGroupedSeries EXPORT_2D load_image_series(const std::vector<std::string>& filenames,
              CProgressCallback *cb = NULL);



NS_MIA_END

#endif
