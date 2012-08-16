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


/*
  LatexBeginPluginSection{2D image stack filters}
  \label{sec:fifof}
  
  These filters work on a stack of 2D images in a 3D manner by interpreting the images of the stack 
   as slices of a 3D image.
  For spacial filters with a fixed filter width that is small compared to the number of slices 
  the filter is applied in a true 3D manner. 
  Other filters, that would normaly require to work on the whole image domain at once, the 
  image filter is only applied to a subset of the images at a time 
  which results in a quasi-3D processing. 
  Only filters are supported that can be run in a single pass. 
  
  Filters are created by the C2DFifoFilterPluginHandler singleton.

  LatexEnd
*/


#include <mia/core/export_handler.hh>


#include <mia/core/plugin_base.hh>
#include <mia/core/handler.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

#include <mia/3d/2dimagefifofilter.hh>
#include <mia/core/fifofilter.cxx>

NS_MIA_BEGIN

const char *image2d_stack::data_descr = "2dstack";

C2DImageFifoFilter::C2DImageFifoFilter(size_t fill, size_t min_fill, size_t read_start):
	TFifoFilter<P2DImage>(fill, min_fill, read_start)
{
}

C3DImage *C2DImageFifoFilter::create_buffer(const C2DBounds& slice_sice, size_t depth, EPixelType type)
{
	const C3DBounds size(slice_sice.x, slice_sice.y, depth);
	switch (type) {
	case it_bit:   return new T3DImage<bool>(size);
	case it_sbyte: return new T3DImage<signed char>(size);
	case it_ubyte: return new T3DImage<unsigned char>(size);
	case it_sshort:return new T3DImage<signed short>(size);
	case it_ushort:return new T3DImage<unsigned short>(size);
	case it_sint:  return new T3DImage<signed int>(size);
	case it_uint:  return new T3DImage<unsigned int>(size);
#ifdef HAVE_INT64
	case it_slong: return new T3DImage<signed long>(size);
	case it_ulong: return new T3DImage<unsigned long>(size);
#endif
	case it_float: return new T3DImage<float>(size);
	case it_double:return new T3DImage<double>(size);
	default:
		assert("unkown image type requested");
		return NULL;
	}
}

typedef TFifoFilterSink<P2DImage> C2DImageFifoFilterSink;
template class TFifoFilterSink<P2DImage>;

C2DFifoFilterPlugin::C2DFifoFilterPlugin(char const * const  name):
	TFactory<C2DImageFifoFilter>(name)
{
}

bool C2DFifoFilterPlugin::call_test(const CImageStack& in_data, const CImageStack& test_data,
				    C2DImageFifoFilter& filter) const
{
	TRACE("C2DFifoFilterPlugin::call_test");



	C2DImageFifoFilterSink::Pointer sink(new C2DImageFifoFilterSink());
	filter.append_filter(sink);

	for (CImageStack::const_iterator i = in_data.begin();
	     i != in_data.end(); ++i)
		filter.push(*i);

	filter.finalize();
	C2DImageFifoFilterSink::result_type r = sink->result();

	if (r.size() != test_data.size()) {
		cvfail() << "Expected "<< test_data.size()  <<" slices, got " << r.size() << "\n";
		return false;
	}

	bool result = true;
	for (size_t i = 0; i < r.size(); ++i) {
		if (r[i]->get_pixel_type() != test_data[i]->get_pixel_type()) {
			cvfail() << "Pixel type\n";
			return false;
		}
		if (r[i]->get_size() != test_data[i]->get_size()) {
			cvfail() << "Size\n";
			return false;
		}
		if (*r[i] != *test_data[i]) {
			cvfail() << "Data\n";

			cvdebug() << "Failed slice: " << i << "\n";
			CImageComparePrinter printer;
			mia::filter(printer, *r[i], *test_data[i]);

			result = false;
		}
	}
	return result;
}


template <> const char *  const 
TPluginHandler<C2DFifoFilterPlugin>::m_help =  
   "These filters work on a stack of 2D images in a 3D manner by interpreting the images of the stack "
   "as slices of a 3D image. For spacial filters with a fixed filter width that is small compared to "
   "the number of slices the filter is applied in a true 3D manner. "
   "For other filters, that would normaly require to work on the whole image domain at once, the "
   "image filter is only applied to a subset of the images at a time which results in a quasi-3D processing " 
  " that may require additional post-processing.";


template class TPlugin<image2d_stack, filter_type>;
template class TFifoFilter<P2DImage>;
template class TFactory<C2DImageFifoFilter>;
template class TFactoryPluginHandler<C2DFifoFilterPlugin>;
//template class TPluginHandler<TFactory<C2DImageFifoFilter, image2d_stack, filter_type> >;
template class TPluginHandler<C2DFifoFilterPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<C2DFifoFilterPlugin> >;

NS_MIA_END
