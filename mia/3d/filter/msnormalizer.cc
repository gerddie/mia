/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>

#include <mia/3d/filter/msnormalizer.hh>
extern "C" {
#include <cblas.h>
}


NS_BEGIN(msnormalizer_3dimage_filter)

using namespace mia; 
using std::pair; 
using std::make_pair; 
using std::string; 
using std::vector; 

C3DMSNormalizerFilter::C3DMSNormalizerFilter(int hwidth):
m_hwidth(hwidth)
{
}

pair<int, int> get_range(int i, int n)
{
        if (i <= 0) 
                return make_pair(0, n+i);
        else 
                return make_pair(i, n-i+1);
}

int n_elements( int i, int n, int w) 
{
	int wl = i >= w ? w : i; 
	int wr = i + w  < n ? w : n - i - 1; 
	return wl + wr + 1; 
}

template <class T>
void  C3DMSNormalizerFilter::add(C3DFImage& mean, C3DFImage& variance, const mia::T3DImage<T>& data, 
          const C3DBounds& bi, const C3DBounds& bo, const C3DBounds& ei) const
{

        int x_length = ei.x - bi.x; 
	
	auto sum_slice = [&data, &mean, &variance, bi, bo, ei, x_length](const tbb::blocked_range<int>& range) {
		
		vector <float> in_buffer(x_length);
		for(auto z = range.begin(); z != range.end(); ++z)
			for(unsigned y = bi.y, oy = 0; y != ei.y; ++y, ++oy) {
				
				auto in_start = data.begin_at(bi.x,y,bi.z + z); 
				copy(in_start, in_start + x_length, in_buffer.begin());
				
				cblas_saxpy(x_length, 1.0f, &in_buffer[0],  1, &mean(bo.x, bo.y + oy, bo.z + z), 1);
				
				transform(in_buffer.begin(), in_buffer.end(), 
					  in_buffer.begin(), [](float x){ return x*x;});
				cblas_saxpy(x_length, 1.0f, &in_buffer[0],  1, &variance(bo.x,bo.y + oy, bo.z + z), 1); 
			}
	}; 

	parallel_for(tbb::blocked_range<int>(0, ei.z - bi.z, 1), sum_slice);


}

template <class T>
mia::P3DImage C3DMSNormalizerFilter::operator () (const mia::T3DImage<T>& data) const
{
        // prepare buffers
        C3DFImage *mean = new C3DFImage(data.get_size(), data);
        C3DFImage variance(data.get_size());

        C3DBounds bo; 

        // evaluate sum 
        for (int z = -m_hwidth; z <= m_hwidth; ++z) {
                auto z_range = get_range(z, data.get_size().z);
                bo.z = z >= 0 ? 0 : -z;  
                for (int y = -m_hwidth; y <= m_hwidth; ++y) {
                        auto y_range = get_range(y, data.get_size().y);
                        bo.y = y >= 0 ? 0 : -y; 
                        for (int x = -m_hwidth; x <= m_hwidth; ++x) {
                                auto x_range = get_range(x, data.get_size().x);
                                bo.x = x >= 0 ? 0 : -x; 
                                C3DBounds bi(x_range.first, y_range.first, z_range.first);
                                C3DBounds ei(x_range.second, y_range.second, z_range.second);
				cvdebug() << "bi(" << bi << ")->bo(" << bo << ")\n"; 
                                add(*mean, variance, data, bi, bo, ei);
                        }
                }
        }

	
	auto evaluate_result = [data, this, &mean, &variance](const tbb::blocked_range<unsigned>& range) {
		for (auto z = range.begin(); z != range.end(); ++z) {
			auto im = mean->begin_at(0,0,z); 
			auto iv = variance.begin_at(0,0,z);
			auto ii = data.begin_at(0,0,z); 
			
			const int nz = n_elements(z, data.get_size().z, m_hwidth); 
			for (unsigned y = 0; y != data.get_size().y; ++y) {
				const int ny = n_elements(y, data.get_size().y, m_hwidth); 
				for (unsigned x = 0; x != data.get_size().x; ++x, ++im, ++iv, ++ii) {
					const int nx = n_elements(x, data.get_size().x, m_hwidth); 
					int n = nx * ny * nz; 
					float mean = *im / n;
					float var = (n > 1.0) ? (*iv - *im * mean) / (n - 1.0f) : 0.0f;
					*im = var > 0 ? (*ii - mean) / sqrt(var) : 0.0f; 
				}
			}
		}
	}; 
	
	parallel_for(tbb::blocked_range<unsigned>(0, data.get_size().z, 1), evaluate_result);
	
        return P3DImage(mean); 
}


mia::P3DImage C3DMSNormalizerFilter::do_filter(const mia::C3DImage& image) const
{
        return mia::filter(*this, image); 
}

C3DMSNormalizerFilterPlugin::C3DMSNormalizerFilterPlugin():
C3DFilterPlugin("msnormalizer"), 
        m_hw(1)
{
        add_parameter("w", new CIntParameter(m_hw, 1, std::numeric_limits<int>::max(), false, "half filter width"));        
}

mia::C3DFilter *C3DMSNormalizerFilterPlugin::do_create()const
{
        return new C3DMSNormalizerFilter(m_hw); 
}

const std::string  C3DMSNormalizerFilterPlugin::do_get_descr() const
{
        return "3D image mean-sigma normalizing filter"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
        return new C3DMSNormalizerFilterPlugin();
}

NS_END
