/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef mia_2d_2dimageui_hh
#define mia_2d_2dimageui_hh

#include <set>
#include <vector>
#include <mia/core/ioplugin.hh>
#include <mia/core/iohandler.hh>
#include <mia/core/iohandler.hh>

#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN


/**
   Vector of 2D images to 
 */
class EXPORT_2D C2DImageVector: public C2DImageSeries,
				public CIOData {
public:

	/// \returns a newly created copy of the image vector 
	C2DImageVector *clone() const __attribute__((warn_unused_result));
};


class EXPORT_2D io_2dimage_type {
public:
	typedef  C2DImageVector type;
	static const char *type_descr;
};


typedef TIOPlugin<io_2dimage_type> C2DImageIOPlugin;

class EXPORT_2D C2DImageIOPPH: public TIOPluginHandler<C2DImageIOPlugin> {
public:
	typedef TIOPluginHandler<C2DImageIOPlugin>::CSuffixmap CSuffixmap;
	typedef TIOPluginHandler<C2DImageIOPlugin>::PData PData;
protected:
	C2DImageIOPPH(const std::list< ::boost::filesystem::path>& searchpath);
};

typedef TIOHandlerSingleton< C2DImageIOPPH > C2DImageIOPluginHandler;
typedef C2DImageIOPluginHandler::Instance::DataKey C2DImageDataKey;

typedef C2DImageIOPluginHandler::Instance::PData P2DImageVector;


/**
   Convenience function to create a vector of images wrapping one image
 */
P2DImageVector EXPORT_2D create_image2d_vector(P2DImage image);


/**
   Convenience function to load one 2D image from a file 
   @param filename 
   @returns image 
 */

P2DImage EXPORT_2D  load_image2d(const std::string& filename);

template <>
inline P2DImage load_image<P2DImage>(const std::string& filename)
{
	return load_image2d(filename);
}

/**
   Convenience function to save one 2D image to a file 
   @param filename 
   @param image 
   @returns true if saving was  successfull
 */

bool  EXPORT_2D save_image(const std::string& filename, P2DImage image);


/**
   some DICOM tags that may be used 
 */
extern EXPORT_2D const char * IDModality;
extern EXPORT_2D const char * IDPatientOrientation;
extern EXPORT_2D const char * IDPatientPosition;
extern EXPORT_2D const char * IDAcquisitionDate;
extern EXPORT_2D const char * IDAcquisitionNumber;
extern EXPORT_2D const char * IDImageType;
extern EXPORT_2D const char * IDInstanceNumber;

extern EXPORT_2D const char * IDSeriesNumber;
extern EXPORT_2D const char * IDSliceLocation;
extern EXPORT_2D const char * IDStudyID;
extern EXPORT_2D const char * IDSmallestImagePixelValue;
extern EXPORT_2D const char * IDLargestImagePixelValue;


NS_MIA_END

#endif
