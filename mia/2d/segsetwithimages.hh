/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef mia_2d_SegSetWithImages_hh
#define mia_2d_SegSetWithImages_hh


#include <mia/core/iohandler.hh>

#include <mia/2d/segset.hh>
#include <mia/2d/image.hh>

namespace xmlpp {
	class Document;
};

NS_MIA_BEGIN

/**
   @ingroup perf 
   \brief A set of images and its segmentations, related to heart perfusion analysis  
   
   A set of slices containing segmentationinformation as well as the images. 
 */
class EXPORT_2D CSegSetWithImages: public CSegSet {
public:
	static const char *data_descr;	
	typedef CSegSetWithImages type;	
	typedef std::shared_ptr<CSegSetWithImages> Pointer; 
	
	CSegSetWithImages();

	CSegSetWithImages(int version);


	/**
	   Read the segmentation set and load the images 
	   \param node the root node of the XML segmentation set description
	   \param fileroot is the root location of the set file and it is used as 
	   base path for the images. 
	*/
	CSegSetWithImages(const xmlpp::Document& node, const std::string& fileroot); 
	

	/**
	   Read the segmentation set and load the images 
	   \param filename segmentation set 
	   \param ignore_path if \a true the image reader will ignore the path 
	   assosiated with the images, and use the base directory of the segmentation set.  
	 */
	CSegSetWithImages(const std::string& filename, bool ignore_path);

	/// \returns a vector of the images 
	const C2DImageSeries& get_images()const;

	/// sets the image series @param series 
	void set_images(const C2DImageSeries& series); 
	
	/// save the images to their give file names with the given directory as root @param root 
	void save_images(const std::string& root) const; 


	using CSegSet::add_frame; 
	/**
	   Add a frame ant its correcponding image 
	   \param frame the new frame 
	   \param image the image 
	 */
	void add_frame(const CSegFrame& frame, P2DImage image); 
	
	/** Run acropping on the inout images and correct the segmentation information accordingly 
	    \param start upper left corner of the cropping reagion 
	    \param end lower right corner  of the cropping reagion 
	    \param crop_filename_base new file name base for the cropped images 
	    \returns a new segmentation set with the cropped images and the corrected segmentation information
	*/
	CSegSetWithImages crop(const C2DIVector&  start, const C2DIVector&  end,
			       const std::string& crop_filename_base);

	CSegSetWithImages *clone() const; 
private:
	C2DImageSeries m_images;
};

typedef CSegSetWithImages::Pointer PSegSetWithImages; 

extern template class EXPORT_2DMYOCARD TPlugin<CSegSetWithImages, io_plugin_type>;
extern template class EXPORT_2DMYOCARD TIOPlugin<CSegSetWithImages>; 

typedef TIOPlugin<CSegSetWithImages> CSegSetWithImagesIOPlugin;
typedef THandlerSingleton< TIOPluginHandler<CSegSetWithImagesIOPlugin > > CSegSetWithImagesIOPluginHandler;

template <> 
struct IOHandler_of<CSegSetWithImages> {
	typedef CSegSetWithImagesIOPluginHandler type;
}; 



NS_MIA_END

#endif
