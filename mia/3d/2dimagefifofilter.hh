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


#ifndef __mia_2dfifofilterstack_hh
#define __mia_2dfifofilterstack_hh

#include <mia/2d/2DImage.hh>
#include <mia/3d/3DImage.hh>
#include <mia/core/factory.hh>
#include <mia/core/fifofilter.hh>
#include <mia/core/filter.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

/** @cond INTERNAL
    \brief type for combiner plug-in path handling 
 */

struct EXPORT_3D image2d_stack {
	static const char *data_descr;
};

/**
   \ingroup traits
   \brief an instanciation of the __copy_create_ptr for C2DImage 
   
   This trait is used to copy a C2DImage and return it as a P2DImage shared pointer.  
*/
template <>
struct __copy_create_ptr<C2DImage> {
	static P2DImage apply(const C2DImage& x){
		return P2DImage(x.clone());
	}
};
/// @endcond 

/**
   \ingroup filtering 
   \brief The class to provide filtering of series of 2D images as if they where 3D images 

   This class is the base class for filters that run on series of 2D images in a 3D manner 
   offering out-of-core processing. Specifically, each derived filter should only hold 
   as much data as abolutely neccessary to filter a block of 2D images in a 3D manner. 
   Therefore, only spacial, non-iterative filter can be implementes to be used in this 
   FIFO filtering framework.
   This base class takes care of the data handling needed for out-of-core processing. 
*/


class EXPORT_3D C2DImageFifoFilter: public TFifoFilter<P2DImage>, public CProductBase  {
public:
	typedef image2d_stack plugin_data; 
	typedef filter_type plugin_type; 

	/**
	   Constructor of the FIFO filter. The given parameters describe what amount of memory is needed 
	   for the filter to work properly. Thes parameters are passed directly to the parent class. 
	   \param filter_width actual filter width in Z-direction (i.e. the number of slices used 
	   to evaluate the filter at a certain point. 
	   \param min_fill minimum number of slices required before the filter does actual processing 
	   \param read_start number of slices that have to be pushed into the filter before first results can be read. 
	*/
	C2DImageFifoFilter(size_t filter_width, size_t min_fill, size_t read_start);
protected:

	/**
	   Create the 3D buffer that is needed to hold the amount of data required for the filter 
	   \param slice_sice size of the 2D images to be puhsed down the pipeline 
	   \param depth number of slices to be hold for processing (this may be different from filter width 
	   \param type pixel type to be used for the buffer
	*/
	C3DImage *create_buffer(const C2DBounds& slice_sice, size_t depth, EPixelType type);

	/**
	   @cond INTERNAL 
	   \ingroup Filtering 
	   \brief helper class to dispatch the shift of the slices in the FIFO  buffer based on the pixel type
	*/
	struct CShiftSlices: public TFilter<int> {
		template <typename T>
		int operator ()(T3DImage<T>& input) const
		{
			copy_backward(input.begin(), input.end() - input.get_size().x
				      * input.get_size().y , input.end());
			return 0;
		}
	};
	///@endcond 
};

/**
   \ingroup filtering
   \brief The plug-in base class for the FIFO filters 
*/
class EXPORT_3D C2DFifoFilterPlugin:
	public TFactory<C2DImageFifoFilter> {
protected:
	/**
	   Plugin constructor
	   \param name Name of the filter 
	 */
	C2DFifoFilterPlugin(char const * const  name);

	/// helper class to store a set of images 
	typedef std::vector<P2DImage> CImageStack;

	/**
	   Helper function to run tests on stack filters
	   \param in_data the input data 
	   \param test_data the expected filter result 
	   \param filter the filter to be run (usually the same as handled by the plug-in 
	   \returns true if the filter provided the expected results, false otherwiese  
	   \remark this should be a stand-alone function that is implemented in the test library 
	*/
	bool call_test(const CImageStack& in_data, const CImageStack& test_data,
		       C2DImageFifoFilter& filter)const ;
};

/**
   \ingroup filtering
   \brief Pointer type for the FIFO filter 
*/
typedef TFifoFilter<P2DImage>::Pointer P2DImageFifoFilter;

/**
   \ingroup filtering
   \brief Plugin handler for the FIFO filters 
*/
typedef THandlerSingleton<TFactoryPluginHandler<C2DFifoFilterPlugin> > C2DFifoFilterPluginHandler;

NS_MIA_END

#endif
