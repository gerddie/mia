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

#include <limits>

#include <mia/core/msgstream.hh>
#include <mia/2d/filter/median.hh>

NS_BEGIN(median_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DMedian::C2DMedian(int hw):
	m_hw(hw)
{
}

template < typename T>
struct __dispatch_filter {
	static T  apply(const T2DImage<T>& data, int x, int y, int width, vector<T>& target_vector) {
		int idx = 0;

		typename vector<T>::iterator tend = target_vector.begin();

		for (int iy = max(0, y - width);
		     iy < min(y + width + 1, (int)data.get_size().y);  ++iy)
			for (int ix = max(0, x - width);
			     ix < min(x + width + 1, (int)data.get_size().x);  ++ix) {
				++tend;
				target_vector[idx++] = data(ix,iy);
			}
			sort(target_vector.begin(), tend);
			if (idx & 1)
				return target_vector[idx / 2];
			else
				return T(0.5 * ( target_vector[idx / 2] + target_vector[idx / 2 - 1]));
	}
};

template <>
struct __dispatch_filter<bool> {
	static C2DBitImage::value_type apply(const C2DBitImage& data, int x, int y, int width,
					     vector<bool>& /*target_vector*/) {
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


template <typename T>
typename C2DMedian::result_type C2DMedian::operator () (const T2DImage<T>& data) const
{
	cvdebug() << "C2DMedian::operator () begin\n";

	T2DImage<T> *tresult = new T2DImage<T>(data.get_size());
	P2DImage result(tresult);

	vector<T> target_vector((2 * m_hw + 1) * (2 * m_hw + 1));

	typename T2DImage<T>::iterator i = tresult->begin();

	for (int y = 0; y < (int)data.get_size().y; ++y)
		for (int x = 0; x < (int)data.get_size().x; ++x, ++i)
			*i = __dispatch_filter<T>::apply(data, x, y, m_hw, target_vector);

	cvdebug() << "C2DMedian::operator () end\n";
	return result;
}

P2DImage C2DMedian::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DMedianFilterPluginFactory::C2DMedianFilterPluginFactory():
	C2DFilterPlugin("median")
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "half filter width"));
}

C2DFilter *C2DMedianFilterPluginFactory::do_create()const
{
	return new C2DMedian(m_hw);
}

const string C2DMedianFilterPluginFactory::do_get_descr()const
{
	return "2D image median filter";
}



C2DSaltAndPepperFilter::C2DSaltAndPepperFilter(int hwidth, float thresh):
	m_width(hwidth),
	m_thresh(thresh)
{
}

template <class T>
P2DImage C2DSaltAndPepperFilter::operator () (const T2DImage<T>& data) const
{
	T2DImage<T> *result = new T2DImage<T>(data);
	typename T2DImage<T>::const_iterator inp = data.begin();

	typename T2DImage<T>::iterator i = result->begin();

	vector<T> target_vector((2 * m_width + 1) * (2 * m_width + 1));


	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++i, ++inp) {
			T res = __dispatch_filter<T>::apply(data, x, y, m_width, target_vector);
			float delta = ::fabs((double)(res - *inp));
			if (delta > m_thresh)
				*i = res;
		}
	return P2DImage(result);
}


P2DImage C2DSaltAndPepperFilter::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}


C2DSaltAndPepperFilterFactory::C2DSaltAndPepperFilterFactory():
	C2DFilterPlugin("sandp"),
	m_hw(1),
	m_thresh(100)
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "filter width parameter"));
	add_parameter("thresh", new CFloatParameter(m_thresh, 0, numeric_limits<float>::max(), false, "thresh value"));
}

C2DFilter *C2DSaltAndPepperFilterFactory::do_create()const
{
	return new C2DSaltAndPepperFilter(m_hw, m_thresh);
}
const string  C2DSaltAndPepperFilterFactory::do_get_descr() const
{
	return "salt and pepper  3d filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	CPluginBase *median = new  C2DMedianFilterPluginFactory();
	median->append_interface(new C2DSaltAndPepperFilterFactory());
	return median;
}

NS_END
