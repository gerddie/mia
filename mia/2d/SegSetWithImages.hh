/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_2d_SegSetWithImages_hh
#define mia_2d_SegSetWithImages_hh

#include <mia/2d/SegSet.hh>
#include <mia/2d/2DImage.hh>

namespace xmlpp {
	class Document;
};

NS_MIA_BEGIN

/**
   A set of slices containing segmentationinformation as well as the images. 
 */


class EXPORT_2D CSegSetWithImages: public CSegSet {
public:
	
	CSegSetWithImages();
	/**
	   Read the segmentation set and load the images 
	   \param filename segmentation set 
	   \param ignore_path if \a true the image reader will ignore the path 
	   assosiated with the images, and use the base directory of the segmentation set.  
	 */

	CSegSetWithImages(const std::string& filename, bool ignore_path);

	/// \returns a vector of the images 
	const C2DImageSeries& get_images()const;

	/** Run acropping on the inout images and correct the segmentation information accordingly 
	    \param start upper left corner of the cropping reagion 
	    \param end lower right corner  of the cropping reagion 
	    \param crop_filename_base new file name base for the cropped images 
	    \returns a new segmentation set with the cropped images and the corrected segmentation information
	*/
	CSegSetWithImages crop(const C2DIVector&  start, const C2DIVector&  end,
			       const std::string& crop_filename_base);
private:
	C2DImageSeries _M_images;
};

NS_MIA_END

#endif
