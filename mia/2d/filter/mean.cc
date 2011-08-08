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

/* 
   LatexBeginPluginDescription{2D image filters}
   
   \subsection{Mean filter}
   \label{filter2d:mean}
   
   \begin{description}
   
   \item [Plugin:] median
   \item [Description:] Applies the mean filter. 
   \item [Input:] Abitrary gray scale or binary image 
   \item [Output:] The filtered image of the same pixel type and dimension 
   
   \plugtabstart
   w &  int & filter width parameter & 1  \\
   \plugtabend
   
   \end{description}

   LatexEnd  
 */

#include <mia/core/msgstream.hh>
#include <mia/2d/filter/mean.hh>
#include <boost/type_traits/is_floating_point.hpp>

NS_BEGIN(mean_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DMean::C2DMean(int hw):
	m_hw(hw)
{
}


template <typename T, bool value> 
struct __dispatch_filter {
	static T apply(const T2DImage<T>& data, int cx, int cy, int hw) {
		double result = 0.0; 
		int n = 0; 
		for (int y = cy - hw; y <= cy + hw; ++y) {
			if ( y >= 0 && y < data.get_size().y) 
				for (int x = cx - hw; x <= cx + hw; ++x) {
					if ( x >= 0 && x < data.get_size().x) {
						result += data(x,y); 
						++n; 
					}
				}
		}
		return static_cast<T>(rint(result/n)); 
	}
}; 

template <typename T> 
struct __dispatch_filter<T, true> {
	static T apply(const T2DImage<T>& data, int cx, int cy, int hw) {
		double result = 0.0; 
		int n = 0; 
		for (int y = cy - hw; y <= cy + hw; ++y) {
			if ( y >= 0 && y < data.get_size().y) 
				for (int x = cx - hw; x <= cx + hw; ++x) {
					if ( x >= 0 && x < data.get_size().x) {
						result += data(x,y); 
						++n; 
					}
				}
		}
		return static_cast<T>(result/n); 
	}
}; 

// special case bool - if the number of pixels counted is even and the 
// the number of trues and falses equal return the original value
template <> 
struct __dispatch_filter<bool, false> {
	static bool apply(const T2DImage<bool>& data, int cx, int cy, int hw) {
		int balance = 0; 
		for (int y = cy - hw; y <= cy + hw; ++y) {
			if ( y >= 0 && y < data.get_size().y) 
				for (int x = cx - hw; x <= cx + hw; ++x) {
					if ( x >= 0 && x < data.get_size().x) {
						balance += data(x,y) ? 1 : -1; 
					}
				}
		}
		return (balance > 0) ? true : 
			((balance < 0) ? false : data(cx,cy)); 
	}
}; 

template <typename T> 
C2DMean::result_type C2DMean::operator () (const T2DImage<T>& data) const
{
	TRACE_FUNCTION; 
	const bool is_floating_point = boost::is_floating_point<T>::value; 

	T2DImage<T> *tresult = new T2DImage<T>(data.get_size(), data);
	P2DImage result(tresult);

	typename T2DImage<T>::iterator i = tresult->begin();

	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++i)
			*i = __dispatch_filter<T, is_floating_point>::apply(data, x, y, m_hw);

	return result;
}

P2DImage C2DMean::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMeanFilterPlugin::C2DMeanFilterPlugin():
	C2DFilterPlugin("mean")
{
	add_parameter("w", new CIntParameter(m_hw, 0, numeric_limits<int>::max(), false, "half filter width"));
}

C2DFilter *C2DMeanFilterPlugin::do_create()const
{
	return new C2DMean(m_hw);
}

const string C2DMeanFilterPlugin::do_get_descr()const
{
	return "2D image mean filter";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMeanFilterPlugin();
}

NS_END

