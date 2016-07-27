/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <mia/core/msgstream.hh>
#include <mia/2d/filter/tmean.hh>
#include <boost/type_traits/is_floating_point.hpp>

#include <mia/core/parallel.hh>

NS_BEGIN(tmean_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DTmean::C2DTmean(int hw, float thresh):
	m_hw(hw),
        m_thresh(thresh)
{
}


template <typename T, bool value> 
struct __dispatch_filter {
	static T apply(const T2DImage<T>& data, int cx, int cy, int hw, float thresh) {
		double result = 0.0; 
		int n = 0;
		for (int y = cy - hw; y <= cy + hw; ++y) {
			if ( y >= 0 && y < (int)data.get_size().y) 
				for (int x = cx - hw; x <= cx + hw; ++x) {
					if ( x >= 0 && x < (int)data.get_size().x) {
                                                double v = data(x,y);
                                                if (v > thresh) {
                                                        result += v; 
                                                        ++n;
                                                }
					}
				}
		}
		return (n > 0) ? static_cast<T>(rint(result/n)): 0; 
	}
}; 

template <typename T> 
struct __dispatch_filter<T, true> {
	static T apply(const T2DImage<T>& data, int cx, int cy, int  hw, float thresh) {
		double result = 0.0; 
		int n = 0; 
		for (int y = cy - hw; y <= cy + hw; ++y) {
			if ( y >= 0 && y < (int)data.get_size().y) 
				for (int x = cx - hw; x <= cx + hw; ++x) {
					if ( x >= 0 && x < (int)data.get_size().x) {
                                                double v = data(x,y);
                                                if (v > thresh) {
                                                        result += v;
                                                        ++n;
                                                }
					}
				}
		}
		return (n > 0) ? static_cast<T>(result/n): 0; 
	}
}; 

template <typename T> 
C2DTmean::result_type C2DTmean::operator () (const T2DImage<T>& data) const
{
	TRACE_FUNCTION; 
	assert(m_hw >=0); 
	const bool is_floating_point = boost::is_floating_point<T>::value; 

	T2DImage<T> *tresult = new T2DImage<T>(data.get_size(), data);
	P2DImage result(tresult);


        auto run_line  = [this, data, tresult](const C1DParallelRange& range) {
		for (auto y = range.begin(); y !=  range.end(); ++y) {
			auto  i = tresult->begin_at(0, y);
			auto id = data.begin_at(0, y);
			for (size_t x = 0; x < data.get_size().x; ++x, ++i, ++id) {
				if (*id > m_thresh) 
                                *i = __dispatch_filter<T, is_floating_point>::apply(data, x, y, m_hw, m_thresh);
                        else
                                *i = 0;
			}
		}
	};
	pfor(C1DParallelRange(0, data.get_size().y, 1), run_line);

	return result;
}

C2DTmean::result_type C2DTmean::operator () (const mia::T2DImage<bool>& MIA_PARAM_UNUSED(data)) const
{
        throw invalid_argument("tmean: doesn't make sense for binary images, use mean or median filter"); 
}

P2DImage C2DTmean::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DTmeanFilterPlugin::C2DTmeanFilterPlugin():
	C2DFilterPlugin("tmean"), 
	m_hw(1),
        m_thresh(0.0)
{
	add_parameter("w", make_lc_param(m_hw, 1, false, "half filter width"));
        add_parameter("t", make_param(m_thresh, false, "Threshold for pixels not to take into account"));
}

C2DFilter *C2DTmeanFilterPlugin::do_create()const
{
	return new C2DTmean(m_hw, m_thresh);
}

const string C2DTmeanFilterPlugin::do_get_descr()const
{
	return "2D image thresholded tmean filter: The output pixel value is zero if the input pixel "
                "value is below the given threshold, otherwise the pixels in the evaluation windows "
                "are only considered if the input pixel intensity is above the threshold.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DTmeanFilterPlugin();
}

NS_END
