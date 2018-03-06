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

#include <stdexcept>
#include <mia/2d/filter/crop.hh>


NS_BEGIN(crop_2dimage_filter)
NS_MIA_USE;
using namespace std;


static const char *plugin_name = "crop";


template <class Data2D>
typename C2DCrop::result_type C2DCrop::operator () (const Data2D& data) const
{
       TRACE("C2DCrop::operator ()");
       C2DSize end = m_end;

       if (end.x == -1)
              end.x = data.get_size().x;

       if (end.y == -1)
              end.y = data.get_size().y;

       if (m_start.x >= end.x || m_start.y >= end.y) {
              stringstream errmsg;
              errmsg << "C2DCrop::operator (): start vs. end = (" << m_start.x << ", " << m_start.y << ") vs. ("
                     <<  end.x << ", " << end.y << ")";
              throw invalid_argument(errmsg.str());
       }

       C2DBounds s(end.x - m_start.x, end.y - m_start.y);
       cvdebug() << "new size = (" << s.x << ", " << s.y << ")\n";
       Data2D *result = new Data2D(s, data);

       if (!result) {
              throw runtime_error("crop2d: unable to allocate image");
       }

       result->set_origin(C2DFVector(m_start) * data.get_pixel_size());
       size_t read_start_x = m_start.x < 0 ? 0 : m_start.x;
       size_t write_start_x = m_start.x < 0 ? - m_start.x : 0;
       size_t length_x = result->get_size().x - write_start_x;

       if (data.get_size().x - read_start_x < length_x )
              length_x = data.get_size().x - read_start_x;

       for (size_t iy = m_start.y < 0 ? 0 : m_start.y,
            oy = m_start.y < 0 ? -m_start.y : 0;
            iy < data.get_size().y && oy < result->get_size().y; ++iy, ++oy) {
              typename Data2D::const_iterator s = data.begin_at(read_start_x, iy);
              copy(s, s + length_x, result->begin_at(write_start_x, oy));
       }

       return P2DImage(result);
}

P2DImage C2DCrop::do_filter(const C2DImage& image) const
{
       return mia::filter(*this, image);
}

C2DCropImageFilterFactory::C2DCropImageFilterFactory():
       C2DFilterPlugin(plugin_name),
       m_start(0, 0),
       m_end(-1, -1)
{
       add_parameter("start", new TParameter<C2DCrop::C2DSize>(m_start, false, "start of crop region"));
       add_parameter("end",   new TParameter<C2DCrop::C2DSize>(m_end, false, "end of crop region"));
}

C2DFilter *C2DCropImageFilterFactory::do_create()const
{
       return new C2DCrop(m_start, m_end);
}

const string C2DCropImageFilterFactory::do_get_descr()const
{
       return "Crop a region of an image, the region is always clamped to the original image size.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DCropImageFilterFactory();
}
} // end namespace crop_2dimage_filter
