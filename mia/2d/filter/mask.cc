/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

static char const * plugin_name = "mask";

template <class Data2D>
typename C2DMask::result_type C2DMask::operator () (const Data2D& data) const
{
	if (data.get_size() != m_mask.get_size())
		throw runtime_error("Input image and mask differ in size");

	const typename Data2D::value_type zero = typename Data2D::value_type();

	Data2D *result = new Data2D(data);

	C2DBitImage::const_iterator im = m_mask.begin();
	typename Data2D::iterator id = result->begin();
	typename Data2D::iterator ie = result->end();

	while (id != ie) {
		if (!*im)
			*id = zero;
		++im;
		++id;
	}

	return P2DImage(result);
}


P2DImage C2DMask::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMaskImageFilterFactory::C2DMaskImageFilterFactory():
	C2DFilterPlugin(plugin_name),
	m_invert(false)
{
	add_parameter("img", new CStringParameter(m_mask_name, true,
						  "mask image (must be in bit representation)"));
	add_parameter("inv", new TParameter<bool>(m_invert, false,
						  "whether the mask should be inverted"));
}

C2DFilter *C2DMaskImageFilterFactory::do_create()const
{
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();
	C2DImageIOPluginHandler::Instance::PData inImage_list = imageio.load(m_mask_name);

	if (!inImage_list.get() || !inImage_list->size() ) {
		string not_found = string("No image data found in ") + m_mask_name;
		throw runtime_error(not_found);
	}

	C2DImageIOPluginHandler::Instance::Data::iterator mask = inImage_list->begin();

	if ((*mask)->get_pixel_type() != it_bit)
		throw runtime_error("Binary image needed for mask ");

	C2DBitImage *image = dynamic_cast<C2DBitImage *>(mask->get());

	assert(image);
	if (m_invert)
		transform(image->begin(), image->end(), image->begin(), logical_not<bool>());

	return new C2DMask(*image);
}

const string C2DMaskImageFilterFactory::do_get_descr()const
{
	return "2D image mask filter";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMaskImageFilterFactory();
}
} // end namespace mask_2dimage_filter
