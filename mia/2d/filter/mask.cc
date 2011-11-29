/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <mia/2d/2dimageio.hh>
#include <mia/2d/filter/mask.hh>

NS_BEGIN(mask_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DMask::C2DMask(const C2DImageDataKey& image_key):
	m_image_key(image_key)
{
}

template <typename T>
struct __ifthenelse {
	T operator() ( bool yes, T value) const {
		return yes ? value : T();
	}
};

template <typename T>
struct __dispatch_mask {
	static P2DImage apply(const T2DImage<T> */*mask*/, const C2DImage& /*data*/) {
		throw invalid_argument("one of the input image must be binary");
	}
};

class C2DMaskDispatch : public TFilter< P2DImage > {
public:
	C2DMaskDispatch(const C2DBitImage *mask):
		m_mask(mask)
		{
		}

	template <typename T>
	C2DMaskDispatch::result_type operator () (const mia::T2DImage<T>& data) const 	{

		T2DImage<T> * result = new T2DImage<T>(data.get_size(), data);
		transform(m_mask->begin(), m_mask->end(), data.begin(),  result->begin(),
			  __ifthenelse<T>());
		return C2DMask::result_type(result);
	}
private:
	const C2DBitImage *m_mask;
};


template <>
struct __dispatch_mask<bool> {
	static P2DImage apply(const C2DBitImage *mask, const C2DImage& data) {
		if (data.get_size() != mask->get_size()) {
			throw invalid_argument("Mask: input image and mask must be of same size");
		}

		C2DMaskDispatch m(mask);
		return mia::filter(m, data);
	}
};

template <typename T>
C2DMask::result_type C2DMask::operator () (const T2DImage<T>& data) const
{
	C2DImageIOPlugin::PData in_image_list = m_image_key.get();

	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C2DMaskImage: no image available in data pool");

	P2DImage image = (*in_image_list)[0];

	if (image->get_pixel_type() == it_bit) {
		const C2DBitImage *mask = dynamic_cast<const C2DBitImage*>(image.get());
		C2DMaskDispatch m(mask);
		return m(data);
	} else {
		return __dispatch_mask<T>::apply(&data, *image);
	}
}

mia::P2DImage C2DMask::do_filter(const mia::C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMaskImageFilterFactory::C2DMaskImageFilterFactory():
	C2DFilterPlugin("mask")
{
	add_parameter("input", new CStringParameter(m_mask_filename, true, "second input image file name"));
}

C2DFilter *C2DMaskImageFilterFactory::do_create()const
{
	C2DImageDataKey mask_data = C2DImageIOPluginHandler::instance().load_to_pool(m_mask_filename);
	return new C2DMask(mask_data);
}

const std::string C2DMaskImageFilterFactory::do_get_descr()const
{
	return "2D masking, one of the two input images must by of type bit.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMaskImageFilterFactory();
}
} // end namespace mask_2dimage_filter
