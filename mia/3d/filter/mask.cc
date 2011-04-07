
/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
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

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/if.hpp>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/filter/mask.hh>


NS_BEGIN(mask_3dimage_filter)
NS_MIA_USE;
using namespace std;
using namespace boost::lambda;

C3DMask::C3DMask(const C3DImageDataKey& image_key):
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
	static P3DImage apply(const T3DImage<T> */*mask*/, const C3DImage& /*data*/) {
		throw invalid_argument("one of the input image must be binary");
	}
};

class C3DMaskDispatch : public TFilter< P3DImage > {
public:
	C3DMaskDispatch(const C3DBitImage *mask):
		m_mask(mask)
		{
		}

	template <typename T>
	C3DMaskDispatch::result_type operator () (const mia::T3DImage<T>& data) const 	{

		T3DImage<T> * result = new T3DImage<T>(data.get_size(), data.get_attribute_list());
		transform(m_mask->begin(), m_mask->end(), data.begin(),  result->begin(),
			  __ifthenelse<T>());
		return C3DMask::result_type(result);
	}
private:
	const C3DBitImage *m_mask;
};


template <>
struct __dispatch_mask<bool> {
	static P3DImage apply(const C3DBitImage *mask, const C3DImage& data) {
		if (data.get_size() != mask->get_size()) {
			throw invalid_argument("Mask: input image and mask must be of same size");
		}

		C3DMaskDispatch m(mask);
		return mia::filter(m, data);
	}
};

template <typename T>
C3DMask::result_type C3DMask::operator () (const T3DImage<T>& data) const
{
	C3DImageIOPlugin::PData in_image_list = m_image_key.get();

	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DMaskImage: no image available in data pool");

	P3DImage image = (*in_image_list)[0];

	if (image->get_pixel_type() == it_bit) {
		const C3DBitImage *mask = dynamic_cast<const C3DBitImage*>(image.get());
		C3DMaskDispatch m(mask);
		return m(data);
	} else {
		return __dispatch_mask<T>::apply(&data, *image);
	}
}

mia::P3DImage C3DMask::do_filter(const mia::C3DImage& image) const
{
	return mia::filter(*this, image);
}

C3DMaskImageFilterFactory::C3DMaskImageFilterFactory():
	C3DFilterPlugin("mask")
{
	add_parameter("input", new CStringParameter(m_mask_filename, true, "second input image file name"));
}

mia::C3DFilterPlugin::ProductPtr C3DMaskImageFilterFactory::do_create()const
{
	C3DImageDataKey mask_data = C3DImageIOPluginHandler::instance().load_to_pool(m_mask_filename);
	return C3DFilterPlugin::ProductPtr(new C3DMask(mask_data));
}

const std::string C3DMaskImageFilterFactory::do_get_descr()const
{
	return "3D masking";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DMaskImageFilterFactory();
}
NS_END
