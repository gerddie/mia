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

#include <mia/2d/imageio.hh>
#include <mia/2d/filter/mask.hh>

NS_BEGIN(mask_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DMask::C2DMask(const C2DImageDataKey& image_key, EFill fill, bool inverse):
	m_image_key(image_key), 
	m_fill(fill), 
	m_inverse(inverse)
{
}

const TDictMap<C2DMask::EFill>::Table FillStyleTable[] = {
	{"min", C2DMask::f_min, "set values outside the mask to the minimum value found in the image."}, 
	{"zero", C2DMask::f_zero, "set the values outside the mask to zero."}, 
	{"max", C2DMask::f_max, "set values outside the mask to the maximum value found in the image.."}, 
	{NULL, C2DMask::f_unknown, NULL}
}; 

template <typename T>
struct __dispatch_mask {
	static P2DImage apply(const T2DImage<T> */*mask*/, const C2DImage& /*data*/, C2DMask::EFill /*fill*/, bool /*inverse*/) {
		throw invalid_argument("one of the input image must be binary");
	}
};

class C2DMaskDispatch : public TFilter< P2DImage > {
public:
	C2DMaskDispatch(const C2DBitImage *mask, C2DMask::EFill fill, bool inverse, bool use_data_attr):
		m_mask(mask), 
		m_fill(fill), 
		m_inverse(inverse), 
		m_use_data_attr(use_data_attr)
		{
		}

	template <typename T>
	C2DMaskDispatch::result_type operator () (const mia::T2DImage<T>& data) const 	{

		T fill_value; 
		switch (m_fill) {
		case C2DMask::f_min: fill_value = *min_element(data.begin(), data.end()); break;
		case C2DMask::f_max: fill_value = *min_element(data.begin(), data.end()); break;
		case C2DMask::f_zero: fill_value = T(); break;
		default: 
			throw invalid_argument("C2DMask: unknown fill value selected"); 
		}
				
		T2DImage<T> * result; 
		if (m_use_data_attr) 
			result = new T2DImage<T>(data.get_size(), data);
		else 
			result = new T2DImage<T>(data.get_size(), *m_mask);
		
		if (m_inverse) {
			transform(m_mask->begin(), m_mask->end(), data.begin(),  result->begin(),
				  [&fill_value](bool m, T value){ return m ? fill_value : value; }); 
		
		}else{
			transform(m_mask->begin(), m_mask->end(), data.begin(),  result->begin(),
				  [&fill_value](bool m, T value){ return m ? value : fill_value; }); 
		}
		return C2DMask::result_type(result);
	}
private:
	const C2DBitImage *m_mask;
	C2DMask::EFill m_fill; 
	bool m_inverse; 
	bool m_use_data_attr; 
};


template <>
struct __dispatch_mask<bool> {
	static P2DImage apply(const C2DBitImage *mask, const C2DImage& data, C2DMask::EFill fill, bool inverse) {
		if (data.get_size() != mask->get_size()) {
			throw invalid_argument("Mask: input image and mask must be of same size");
		}

		C2DMaskDispatch m(mask, fill, inverse, false);
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
		// use static cast because type is already tested. 
		const C2DBitImage *mask = static_cast<const C2DBitImage*>(image.get());
		C2DMaskDispatch m(mask, m_fill, m_inverse, true);
		return m(data);
	} else {
		return __dispatch_mask<T>::apply(&data, *image, m_fill, m_inverse);
	}
}

mia::P2DImage C2DMask::do_filter(const mia::C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMaskImageFilterFactory::C2DMaskImageFilterFactory():
	C2DFilterPlugin("mask"), 
	m_fill(C2DMask::f_min), 
	m_inverse(false)
{
	add_parameter("input", new CStringParameter(m_mask_filename, CCmdOptionFlags::required_input, 
						    "second input image file name", 
						    &C2DImageIOPluginHandler::instance()));
	add_parameter("fill", new CDictParameter<C2DMask::EFill>(m_fill, TDictMap<C2DMask::EFill>( FillStyleTable), 
								 "fill style for pixels outside of the mask"));
	add_parameter("inverse", new CBoolParameter(m_inverse, false, "set to true to use the inverse of the mask for masking"));

}

C2DFilter *C2DMaskImageFilterFactory::do_create()const
{
	C2DImageDataKey mask_data = C2DImageIOPluginHandler::instance().load_to_pool(m_mask_filename);
	return new C2DMask(mask_data, m_fill, m_inverse);
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
