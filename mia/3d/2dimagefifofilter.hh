/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

// $Id: filter2dstack.hh,v 1.21 2006-09-18 21:40:12 gerddie Exp $

/*! \brief basic type of a plugin handler

A basis for filter plugins

\file filter_plugin.hh
\author Gert Wollny <wollny at cbs.mpg.de>

*/

#ifndef __mia_2dfifofilterstack_hh
#define __mia_2dfifofilterstack_hh


#include <mia/core/shared_ptr.hh>

#include <mia/2d/2DImage.hh>
#include <mia/3d/3DImage.hh>
#include <mia/core/factory.hh>
#include <mia/core/fifofilter.hh>
#include <mia/core/filter.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

struct EXPORT_3D image2d_stack {
	static const char *type_descr;
};

template <>
struct __copy_create_ptr<C2DImage> {
	static P2DImage apply(const C2DImage& x){
		return P2DImage(x.clone());
	}
};


class EXPORT_3D C2DImageFifoFilter: public TFifoFilter<P2DImage>, public CProductBase  {
public:
	C2DImageFifoFilter(size_t filter_width, size_t min_fill, size_t read_start);
protected:
	C3DImage *create_buffer(const C2DBounds& slice_sice, size_t depth, EPixelType type);

	struct CShiftSlices: public TFilter<int> {
		template <typename T>
		int operator ()(T3DImage<T>& input) const
		{
			copy_backward(input.begin(), input.end() - input.get_size().x
				      * input.get_size().y , input.end());
			return 0;
		}
	};

};

class EXPORT_3D C2DFifoFilterPlugin:
	public TFactory<C2DImageFifoFilter, image2d_stack, filter_type> {
protected:
	C2DFifoFilterPlugin(char const * const  name);
	typedef std::vector<P2DImage> CImageStack;
	bool call_test(const CImageStack& in_data, const CImageStack& test_data,
		       C2DImageFifoFilter& filter)const ;
};

typedef TFifoFilter<P2DImage>::Pointer P2DImageFifoFilter;
typedef THandlerSingleton<TFactoryPluginHandler<C2DFifoFilterPlugin> > C2DFifoFilterPluginHandler;

NS_MIA_END

#endif
