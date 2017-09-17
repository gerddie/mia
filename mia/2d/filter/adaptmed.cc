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

#include <limits>
#include <mia/2d/filter/adaptmed.hh>

NS_BEGIN(adaptmed_2dimage_filter)

NS_MIA_USE;
using namespace std;


static char const * plugin_name = "adaptmed";

C2DAdaptMedian::C2DAdaptMedian(int hw):
	m_hw(hw)
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
			auto tend = target_vector.begin();
			int idx = 0;
			// sub-optimal, eigentlich braucht man nur "auﬂenrum" gehen
			for (int iy = max(0, y - width);
			     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
				for (int ix = max(0, x - width);
				     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
					auto val = data(ix,iy);
					if (val < zmin)
						zmin = val;
					if (val > zmax)
						zmax = val;
					*tend = val;
					++tend;
					++idx;
				}

			auto  mid = target_vector.begin();
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

	Data2D *result = new Data2D(data.get_size(), data);

	vector<typename Data2D::value_type> target_vector((2 * m_hw + 1) *
							       (2 * m_hw + 1));

	typename Data2D::iterator i = result->begin();

	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
			*i = __dispatch_filter<Data2D>::apply(data, x, y, m_hw, target_vector);

	cvdebug() << "C2DAdaptMedian::operator () end\n";
	return P2DImage(result);
}

C2DAdaptMedian::result_type C2DAdaptMedian::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DAdaptMedianImageFilterFactory::C2DAdaptMedianImageFilterFactory():
	C2DFilterPlugin(plugin_name), 
	m_hw(2)
{
	add_parameter("w", make_lc_param(m_hw, 1, false, "half filter width"));
}

C2DFilter *C2DAdaptMedianImageFilterFactory::do_create()const
{
	return new C2DAdaptMedian(m_hw);
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
