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
#include <mia/2d/filter/labelscale.hh>

NS_BEGIN( labelscale_2dimage_filter)

using namespace mia; 
using std::vector; 
using std::pair; 
using std::make_pair; 
using std::invalid_argument; 

C2DLabelscale::C2DLabelscale(const C2DBounds& out_size):
        m_out_size(out_size)
{
}

inline pair<unsigned, unsigned> clamp_coord(unsigned i, unsigned di, unsigned ni) 
{
	unsigned first = i * di; 
	unsigned second = first +  di; 
	return make_pair(first, second > ni? ni: second); 
}; 


template <typename  T>
T get_max_represented(const vector<T>& buffer)
{
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

template <typename  T>
typename C2DLabelscale::result_type C2DLabelscale::operator () (const T2DImage<T>& data) const
{
        T2DImage<T> *result = new T2DImage<T>(m_out_size, data); 

        C2DBounds block_size = C2DBounds::_1; 
        C2DFVector coordinate_scale = C2DFVector::_1; 

        if (m_out_size.x  < data.get_size().x ){
                block_size.x = (data.get_size().x + m_out_size.x - 1) / m_out_size.x; 
        }else{
                coordinate_scale.x = float(data.get_size().x) / float(m_out_size.x); 
        }

                
        if (m_out_size.y  < data.get_size().y ) {
                block_size.y = (data.get_size().y + m_out_size.y - 1) / m_out_size.y;
        }else{
                coordinate_scale.y = float(data.get_size().y) / float(m_out_size.y); 
        }
        
        vector<T> buffer; 
        buffer.reserve(block_size.product());
        auto ir = result->begin(); 

        for (unsigned y = 0; y < m_out_size.y; ++y) {
                unsigned iy = static_cast<unsigned>(floor(y * coordinate_scale.y)); 
                auto rangey = clamp_coord(iy, block_size.y, data.get_size().y); 
                for (unsigned x = 0; x < m_out_size.x; ++x, ++ir) {
                        unsigned ix = static_cast<unsigned>(floor(x * coordinate_scale.x)); 
                        auto rangex = clamp_coord(ix, block_size.x, data.get_size().x);
                        C2DBounds ibegin(rangex.first, rangey.first); 
                        C2DBounds iend(rangex.second, rangey.second);
			
                        buffer.clear(); 
                        for_each(data.begin_range(ibegin, iend), data.end_range(ibegin, iend), 
                                 [&buffer](T pixel) {
                                         buffer.push_back(pixel); 
                                 });
                        
                        sort(buffer.begin(), buffer.end()); 
                        *ir = get_max_represented(buffer);
                }
        }
        
        C2DFVector pixel_size = data.get_pixel_size();
        C2DFVector factor(float(data.get_size().x / float(m_out_size.x) ),
			  float(data.get_size().y / float(m_out_size.y) ));

	result->set_pixel_size(pixel_size * factor);
	
	return P2DImage(result); 


}

P2DImage C2DLabelscale::do_filter(const C2DImage& image) const
{
        return mia::filter(*this, image); 
}

C2DLabelscaleFilterPluginFactory::C2DLabelscaleFilterPluginFactory():
C2DFilterPlugin("labelscale")
{
        add_parameter("out-size", new C2DBoundsParameter(m_out_size, true, "target size given as two coma separated values"));
}

C2DFilter *C2DLabelscaleFilterPluginFactory::do_create()const
{
        if (m_out_size.x < 1 || m_out_size.y < 1) {
                throw create_exception<invalid_argument>("labelscale: Output sizes must be positive (got", 
                                                         m_out_size, ")"); 
        }
        return new C2DLabelscale(m_out_size); 
}

const std::string C2DLabelscaleFilterPluginFactory::do_get_descr()const
{
        return "A filter that only creates output voxels that are already created in "
                "the input image. Scaling is done by using a voting algorithms that "
                "selects the target pixel value based on the highest pixel count of a "
                "certain label in the corresponding source region. If the region "
                "comprises two labels with the same count, the one with the lower number "
                "wins.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLabelscaleFilterPluginFactory();
}


NS_END
