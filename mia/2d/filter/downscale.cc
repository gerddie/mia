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
#include <sstream>

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/filter/downscale.hh>

NS_BEGIN(downscale_2dimage_filter)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs = ::boost::filesystem;

CDownscale::CDownscale(const C2DBounds& block_size, const string& filter):
       m_block_size(block_size)
{
       stringstream fcompose;
       fcompose << "sepconv:kx=[" << filter << ":w=" << block_size.x / 2
                << "],ky=[" << filter << ":w=" << block_size.y / 2
                << "]";
       cvdebug() << "CDownscale::CDownscale: smoothing kernel '" << fcompose.str() << "'\n";
       m_smoothing = C2DFilterPluginHandler::instance().produce(fcompose.str().c_str());

       if (!m_smoothing)
              throw invalid_argument(string("smoothing filter creation from '") +
                                     fcompose.str() + string("failed"));
       else
              cvdebug() << " Created filter with use count:" << m_smoothing.use_count() << "\n";
}

CDownscale::~CDownscale()
{
       cvdebug() << " Release filter with use count:" << m_smoothing.use_count() << "\n";
}

template <class T>
CDownscale::result_type CDownscale::operator () (const T2DImage<T>& src) const
{
       cvdebug() << "CDownscale::operator () begin:\n";
       T2DImage<T> *fresult = new T2DImage<T>(
              C2DBounds((src.get_size().x + m_block_size.x - 1) / m_block_size.x,
                        (src.get_size().y + m_block_size.y - 1) / m_block_size.y), src);
       CDownscale::result_type Result(fresult);
       typename T2DImage<T>::iterator i = fresult->begin();
       C2DBounds Start(m_block_size.x / 2, m_block_size.y / 2);

       // Put the filtered block values into the target
       for (size_t out_y = 0; out_y < fresult->get_size().y; Start.y += m_block_size.y, ++out_y) {
              Start.x = m_block_size.x / 2;

              for (size_t out_x = 0; out_x < fresult->get_size().x; Start.x += m_block_size.x, ++out_x, ++i) {
                     *i = src(Start);
              }
       }

       C2DFVector pixel_size = src.get_pixel_size();
       pixel_size.x /= m_block_size.x;
       pixel_size.y /= m_block_size.y;
       fresult->set_pixel_size(pixel_size);
       cvdebug() << "CDownscale::operator () end\n";
       return Result;
}

CDownscale::result_type CDownscale::do_filter(const C2DImage& image) const
{
       P2DImage smooth_image = m_smoothing->filter(image);
       return mia::filter(*this, *smooth_image);
}


C2DDownscaleFilterPlugin::C2DDownscaleFilterPlugin():
       C2DFilterPlugin("downscale"),
       m_b(1, 1),
       m_filter("gauss")
{
       add_parameter("bx", make_lc_param(m_b.x, 1u, false, "blocksize in x direction"));
       add_parameter("by", make_lc_param(m_b.y, 1u, false, "blocksize in y direction"));
       add_parameter("b", new C2DBoundsParameter(m_b, false, "blocksize"));
       add_parameter("kernel", new CStringParameter(m_filter, CCmdOptionFlags::none,
                     "smoothing filter kernel to be applied, the "
                     "size of the filter is estimated based on the blocksize.",
                     &C1DSpacialKernelPluginHandler::instance()));
}

C2DFilter *C2DDownscaleFilterPlugin::do_create()const
{
       return new CDownscale(m_b, m_filter);
}

const string C2DDownscaleFilterPlugin::do_get_descr()const
{
       return "Downscale the input image by using a given block size to define the "
              "downscale factor. Prior to scaling the image is filtered by a smoothing "
              "filter to eliminate high frequency data and avoid aliasing artifacts.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DDownscaleFilterPlugin();
}

NS_END
