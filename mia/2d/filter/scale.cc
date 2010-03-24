/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
  A image scaling filter
  \author Gert Wollny <gert at die.upm.es>
*/

#include <limits>
#include <sstream>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filter/scale.hh>


NS_BEGIN(scale_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs= ::boost::filesystem;

CScale::CScale(const C2DBounds& size, const string& filter):
	_M_size(size),
	_M_ipf(create_2dinterpolation_factory(GInterpolatorTable.get_value(filter.c_str())))
{

}

CScale::~CScale()
{
}

template <class T>
CScale::result_type CScale::operator () (const T2DImage<T>& src) const
{
	if (src.get_size() == _M_size)
		return CScale::result_type(new T2DImage<T>(src));




	auto_ptr<T2DInterpolator<T> > s(_M_ipf->create(src.data()));

	T2DImage<T> *result = new T2DImage<T>(_M_size);
	typename T2DImage<T>::iterator i = result->begin();
	C2DFVector factor(float(src.get_size().x / float(_M_size.x) ),
			  float(src.get_size().y / float(_M_size.y) ));


	C2DFVector l(0.0, 0.0);
	for (size_t y = 0; y < _M_size.y; ++y, l.y += factor.y) {
		l.x = 0.0;
		for (size_t x = 0; x < _M_size.x; ++x, l.x += factor.x, ++i) {
			*i = (*s)(l);
		}
	}

	result->set_pixel_size(src.get_pixel_size() / factor);
	return CScale::result_type(result);
}

CScale::result_type CScale::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DScaleFilterPlugin::C2DScaleFilterPlugin():
	C2DFilterPlugin("scale"),
	_M_sx(128),
	_M_sy(128),
	_M_interp("bspline3")
{
	add_parameter("sx", new CIntParameter(_M_sx, 1,
					      numeric_limits<int>::max(), true,
					      "target size in x direction"));

	add_parameter("sy", new CIntParameter(_M_sy, 1,
					      numeric_limits<int>::max(), true,
					      "target size in y direction"));

	add_parameter("interp", new CStringParameter(_M_interp, false,
						     "interpolation method to be used "));
}


C2DScaleFilterPlugin::ProductPtr C2DScaleFilterPlugin::do_create()const
{
	return C2DScaleFilterPlugin::ProductPtr(new CScale(C2DBounds(_M_sx, _M_sy), _M_interp));
}

const string C2DScaleFilterPlugin::do_get_descr()const
{
	return "2D image downscaler filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DScaleFilterPlugin();
}

NS_END
