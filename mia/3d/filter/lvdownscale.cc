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

#include <mia/3d/filter/lvdownscale.hh>

NS_BEGIN(lvdownscale_3dimage_filter)

using namespace std; 
using namespace mia; 

C3DLVDownscale::C3DLVDownscale(const C3DBounds& block_size):m_block_size(block_size) 
{
}



inline pair<unsigned, unsigned> clamp_coord(unsigned i, unsigned di, unsigned ni) 
{
	unsigned first = i * di; 
	unsigned second = first +  di; 
	return make_pair(first, second > ni? ni: second); 
}; 


template <typename  T>
C3DFilter::result_type C3DLVDownscale::operator () (const T3DImage<T>& data) const
{
	
	auto get_max_represented = [] (const vector<T>& buffer) -> T {
		T max_elm = T(); 
		if (buffer.empty()) 
			return max_elm; 
		unsigned int max_num = 1; 
		unsigned int cur_num = 1; 

		max_elm = buffer[0]; 
		
		T cur_elm = max_elm; 

		for (auto ib = buffer.begin() + 1; ib != buffer.end() ; ++ib) {
			if (cur_elm == *ib) {
				++cur_num; 
			}else{
				if (cur_num > max_num) {
					max_num = cur_num; 
					max_elm = cur_elm; 
				}
				cur_num = 1; 
				cur_elm = *ib; 
			}
		}
		if (cur_num > max_num) {
			max_elm = cur_elm; 
		}
		return max_elm; 
	}; 

	C3DBounds rsize ( data.get_size() / m_block_size); 
	if (rsize.x < 1) rsize.x = 1; 
	if (rsize.y < 1) rsize.y = 1; 
	if (rsize.z < 1) rsize.z = 1; 

	T3DImage<T> *result = new T3DImage<T>(rsize, data); 


	vector<T> buffer; 
	const size_t buffer_size = m_block_size.product(); 

	auto ir = result->begin(); 
	for (unsigned z = 0; z < rsize.z; ++z) {
		auto rangez = clamp_coord(z, m_block_size.z, data.get_size().z); 
		for (unsigned y = 0; y < rsize.y; ++y) {
			auto rangey = clamp_coord(y, m_block_size.y, data.get_size().y); 
			for (unsigned x = 0; x < rsize.x; ++x, ++ir) {
				auto rangex = clamp_coord(x, m_block_size.x, data.get_size().x);
				C3DBounds ibegin(rangex.first, rangey.first, rangez.first); 
				C3DBounds iend(rangex.second, rangey.second, rangez.second);
								
				buffer.clear(); 
				buffer.reserve(buffer_size); 
				for_each(data.begin_range(ibegin, iend), data.end_range(ibegin, iend), 
					 [&buffer](T pixel) {
						 if (pixel)
							 buffer.push_back(pixel); 
					 }); 
				sort(buffer.begin(), buffer.end()); 
				*ir = get_max_represented(buffer); 
				
			}
		}
	}
	
	C3DFVector pixel_size = data.get_voxel_size();
	pixel_size.x *= m_block_size.x;
	pixel_size.y *= m_block_size.y;
	pixel_size.z *= m_block_size.z;
	result->set_voxel_size(pixel_size);
	
	return P3DImage(result); 
}


C3DFilter::result_type C3DLVDownscale::do_filter(const C3DImage& image) const
{
	return mia::filter(*this, image); 
}



C3DLVDownscaleFilterPlugin::C3DLVDownscaleFilterPlugin():
	C3DFilterPlugin("lvdownscale"),
	m_b(1,1,1)
{
	add_parameter("b", new C3DBoundsParameter(m_b, false, "blocksize for the downscaling. Each block will be represented "
						  "by one pixel in the target image."));

}


C3DFilter *C3DLVDownscaleFilterPlugin::do_create()const
{
	return new C3DLVDownscale(m_b); 
}

const string C3DLVDownscaleFilterPlugin::do_get_descr()const
{
	return "This is a label voting downscale filter. It adownscales a 3D image by blocks. "
		"For each block the (non-zero) label that appears most times in the block is "
		"issued as output pixel in the target image. If two labels appear the same number "
		"of times, the one with the lower absolute value wins."; 
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLVDownscaleFilterPlugin();
}


NS_END

