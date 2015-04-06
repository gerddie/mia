/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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
#include <mia/3d/filter/mean.hh>
#include <mia/core/utils.hh>
#include <mia/core/threadedmsg.hh>


#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


NS_BEGIN(mean_3dimage_filter)
using namespace mia; 
using std::unique_ptr; 
using std::pair;
using std::make_pair; 

C3DMeanFilter::C3DMeanFilter(int hwidth):
      m_hwidth(hwidth)
{
}

inline pair<C3DBounds, C3DBounds> prepare_range(const C3DBounds& size, int cx, int cy, int cz, int hw) 
{
	int zb = cz - hw;
	if (zb < 0) zb = 0; 
	unsigned ze = cz + hw + 1; 
	if (ze > size.z) ze = size.z; 
	
	int yb = cy - hw;
	if (yb < 0) yb = 0; 
	unsigned ye = cy + hw + 1; 
	if (ye > size.y) ye = size.y; 
	
	int xb = cx - hw;
	if (xb < 0) xb = 0; 
	unsigned xe = cx + hw + 1; 
	if (xe > size.x) xe = size.x; 
	
	return make_pair(C3DBounds(xb,yb,zb), C3DBounds(xe,ye,ze)); 
}


template <typename T, bool value> 
struct __dispatch_filter {
	static T apply(const T3DImage<T>& data, int cx, int cy, int cz, int hw, int freedom) {
		double result = 0.0; 
		int n = freedom;
		// 
		// Coverty complains about this: 1128688, 1128687 
		// 
		// hw >= 1, cy >= 0 && cy < data.get_size().y
		// therefore n>=8
		// 
		auto range = prepare_range(data.get_size(), cx, cy, cz, hw); 
		auto rb = data.begin_range(range.first,range.second); 
		auto re = data.end_range(range.first,range.second); 
		
		while (rb != re) {
			result += *rb; 
			++rb;
			++n; 
		}
		return mia_round_clamped<T>(rint(result/n)); 
	}
}; 

template <typename T> 
struct __dispatch_filter<T, true> {
	static T apply(const T3DImage<T>& data, int cx, int cy, int cz, int  hw, int freedom) {
		double result = 0.0; 
		int n = freedom; 
		// 
		// see above. Coverty  1128688, 1128687
		// 
		auto range = prepare_range(data.get_size(), cx, cy, cz, hw); 
		auto rb = data.begin_range(range.first,range.second); 
		auto re = data.end_range(range.first,range.second); 
		
		while (rb != re) {
			result += *rb; 
			++rb; 
			++n; 
		}
		return static_cast<T>(result/n); 
	}
}; 

// special case bool - if the number of pixels counted is even and the 
// the number of trues and falses equal return the original value
template <> 
struct __dispatch_filter<bool, false> {
	static bool apply(const T3DImage<bool>& data, int cx, int cy, int cz, int hw, int MIA_PARAM_UNUSED(freedom)) {
		int balance = 0; 
		
		auto range = prepare_range(data.get_size(), cx, cy, cz, hw); 
		auto rb = data.begin_range(range.first,range.second); 
		auto re = data.end_range(range.first,range.second); 
		
		while (rb != re) {
			balance += *rb  ? 1 : -1; 
			++rb; 
		}

		return (balance > 0) ? true : 
			((balance < 0) ? false : data(cx,cy,cz)); 
	}
};

template <class T>
mia::T3DImage<T> *C3DMeanFilter::apply(const mia::T3DImage<T>& data) const
{
        const bool is_floating_point = std::is_floating_point<T>::value; 
        T3DImage<T> * result = new T3DImage<T>(data.get_size(), data);
        const int hw = m_hwidth; 

        auto run_slice  = [hw, data, result](const tbb::blocked_range<size_t>& range) {
                for (auto z = range.begin(); z != range.end(); ++z) {
                        auto ir = result->begin_at(0,0,z);  
                        for (size_t y = 0; y < data.get_size().y; ++y)
                                for (size_t x = 0; x < data.get_size().x; ++x, ++ir) {
                                        *ir = __dispatch_filter<T, is_floating_point>::apply(data, x,y,z, hw, 0); 
                                }
                }
        }; 
        parallel_for(tbb::blocked_range<size_t>(0, data.get_size().z, 1), run_slice);
        return result;
}


template <class T>
mia::P3DImage C3DMeanFilter::operator () (const T3DImage<T>& data) const 
{
        return P3DImage(apply(data)); 
}

mia::P3DImage C3DMeanFilter::do_filter(const mia::C3DImage& image) const
{
        return mia::filter(*this, image); 
}



C3DMeanFilterPlugin::C3DMeanFilterPlugin():
        C3DFilterPlugin("mean"), 
        m_hw(1)
{
        add_parameter("w", make_lc_param(m_hw, 1, false, "half filter width"));
}
                       
C3DFilter *C3DMeanFilterPlugin::do_create()const
{
        return new C3DMeanFilter(m_hw);
}

const std::string  C3DMeanFilterPlugin::do_get_descr() const
{
        return "3D image mean filter";
}



C3DVarianceFilter::C3DVarianceFilter(int hwidth):
m_hwidth(hwidth), 
        m_mean(hwidth)
{
        
}

template <class T>
P3DImage C3DVarianceFilter::operator () (const mia::T3DImage<T>& data) const
{
        unique_ptr<mia::T3DImage<T>> mean(m_mean.apply(data)); 
        
        transform(data.begin(), data.end(), mean->begin(), mean->begin(), 
                  [](T x, T y) -> T { T xy = x - y; return xy*xy; }); 

        const bool is_floating_point = std::is_floating_point<T>::value; 
        T3DImage<T> * result = new T3DImage<T>(data.get_size(), data);
        const int hw = m_hwidth; 

        auto run_slice  = [hw, &mean, result](const tbb::blocked_range<size_t>& range) {
                for (auto z = range.begin(); z != range.end(); ++z) {
                        auto ir = result->begin_at(0,0,z);  
                        for (size_t y = 0; y < mean->get_size().y; ++y)
                                for (size_t x = 0; x < mean->get_size().x; ++x, ++ir) {
                                        *ir = sqrt(__dispatch_filter<T, is_floating_point>::apply(*mean, x,y,z, hw, -1)); 
                                        
                                }
                }
        }; 
        parallel_for(tbb::blocked_range<size_t>(0, data.get_size().z, 1), run_slice);
        return P3DImage(result);
 }

mia::P3DImage C3DVarianceFilter::operator () (const mia::C3DBitImage& data) const
{
	cvwarn() << "This filter does not really evaluate the pixel wise variance\n";  
        C3DFImage fimage(data.get_size(), data); 
        copy(data.begin(), data.end(), fimage.begin()); 
        return this->operator()(fimage);
}

P3DImage C3DVarianceFilter::do_filter(const mia::C3DImage& image) const
{
        return mia::filter(*this, image); 
}


C3DVarianceFilterPlugin::C3DVarianceFilterPlugin():
C3DFilterPlugin("variance"), 
        m_hw(1)
{
        add_parameter("w", make_lc_param(m_hw, 1, false, "half filter width"));        
}
        
mia::C3DFilter *C3DVarianceFilterPlugin::do_create()const
{
        return new C3DVarianceFilter(m_hw); 
}

const std::string  C3DVarianceFilterPlugin::do_get_descr() const
{
        return "3D image variance filter"; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
        auto chain = new C3DMeanFilterPlugin();
        chain->append_interface(new C3DVarianceFilterPlugin()); 
	return chain; 
}


NS_END

