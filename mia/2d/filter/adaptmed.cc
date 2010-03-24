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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
   This implementation of an adaptive median filter works like follows:
   - filter width is w=2*n+1
   - start with n=1
     * if the resulting filtered value is equal to the min or max of the covered
       area (max or min) then increase n and repeat filtering
     * repeat above algorithm until n=n_max or resulting value is not equal to an
       extreme value
   - if the original intensity value of the image is equal to one of the extremes,
     then keep the value, otherwise replace it by the filter result
   for bit valued images this is just the normal median filter.
*/

#include <limits>
#include <mia/2d/filter/adaptmed.hh>


NS_BEGIN(adaptmed_2dimage_filter)

NS_MIA_USE;
using namespace std;


static char const * plugin_name = "adaptmed";

C2DAdaptMedian::C2DAdaptMedian(int hw):
	_M_hw(hw)
{
}

template < class Data2D>
struct __dispatch_filter {
	static typename Data2D::value_type  apply(const Data2D& data, int x, int y, int max_width,
						  vector<typename Data2D::value_type>& target_vector) {
		int width = 1;

		typename Data2D::value_type median;
		typename Data2D::value_type zmin = numeric_limits<typename Data2D::value_type>::max();
		typename Data2D::value_type zmax = numeric_limits<typename Data2D::value_type>::min();


		do  {
			typename vector<typename Data2D::value_type>::iterator tend = target_vector.begin();
			int idx = 0;
			// sub-optimal, eigentlich braucht man nur "auﬂenrum" gehen
			for (int iy = max(0, y - width);
			     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
				for (int ix = max(0, x - width);
				     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
					typename Data2D::value_type val = data(ix,iy);
					if (val < zmin)
						zmin = val;
					if (val > zmax)
						zmax = val;
					*tend = val;
					++tend;
					++idx;
				}

			typename vector<typename Data2D::value_type>::iterator mid = target_vector.begin();
			advance(mid, idx/2);
			nth_element(target_vector.begin(), mid, tend);
			median = *mid;
			if ( !(idx & 1) ) {
				--mid;
				nth_element(target_vector.begin(), mid, tend);
				median = typename Data2D::value_type(0.5 * (median + *mid));
			}
			++width;

		} while (width <= max_width && (median == zmin || median == zmax));

		typename Data2D::value_type me = data(x,y);
		if (me > zmin && me < zmax)
			return median;
		else
			return data(x,y);
	}
};

template <>
struct __dispatch_filter<C2DBitImage> {
	static C2DBitImage::value_type apply(const C2DBitImage& data, int x, int y, int width,
					     vector<C2DBitImage::value_type>& /*target_vector*/) {
		int trues = 0;
		int falses = 0;
		for (int iy = max(0, y - width);
		     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
			for (int ix = max(0, x - width);
			     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
				if (data(ix, iy))
					++trues;
				else
					++falses;
			}
		return trues > falses;
	}
};


template <class Data2D>
typename C2DAdaptMedian::result_type C2DAdaptMedian::operator () (const Data2D& data) const
{
	cvdebug() << "C2DAdaptMedian::operator () begin\n";

	Data2D *result = new Data2D(data.get_size());

	vector<typename Data2D::value_type> target_vector((2 * _M_hw + 1) *
							       (2 * _M_hw + 1));

	typename Data2D::iterator i = result->begin();

	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
			*i = __dispatch_filter<Data2D>::apply(data, x, y, _M_hw, target_vector);

	cvdebug() << "C2DAdaptMedian::operator () end\n";
	return P2DImage(result);
}

C2DAdaptMedian::result_type C2DAdaptMedian::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DAdaptMedianImageFilterFactory::C2DAdaptMedianImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_parameter("w", new CIntParameter(_M_hw, 0, numeric_limits<int>::max(),
					     false, "half filter width"));
}

C2DFilterPlugin::ProductPtr C2DAdaptMedianImageFilterFactory::do_create()const
{
	return C2DFilterPlugin::ProductPtr(new C2DAdaptMedian(_M_hw));
}

const string C2DAdaptMedianImageFilterFactory::do_get_descr()const
{
	return "2D image adaptive median filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DAdaptMedianImageFilterFactory();
}

NS_END
