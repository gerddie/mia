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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <mia/core/type_traits.hh>
#include <mia/3d/fifof/regiongrow.hh>

NS_BEGIN(regiongrow_fifof)

NS_MIA_USE;
using namespace std;
using namespace boost;
namespace bfs = boost::filesystem;

static char const *plugin_name = "regiongrow";


C2DRegiongrowFifoFilter::C2DRegiongrowFifoFilter(const CProbabilityVector probmap,
              float low, float seed, int cls, int depth):
       C2DImageFifoFilter(1, 1, depth),
       m_probmap(probmap),
       m_low(low),
       m_seed(seed),
       m_class(cls),
       m_depth(depth),
       m_slice_emls(0)
{
       m_shape = C3DShapePluginHandler::instance().produce("6n");
}

template <typename T, bool is_float>
struct FTransform {
       FTransform(CProbabilityVector::value_type map):
              m_map(map),
              m_maxidx(map.size() - 1)
       {
              assert((size_t)m_maxidx < map.size());
       }

       double operator () (T x) const
       {
              if (x <= 0)
                     return m_map[0];

              if (x <= m_maxidx)
                     return m_map[x];

              return m_map[m_maxidx];
       }
       CProbabilityVector::value_type m_map;
       T m_maxidx;
};

template <typename T>
struct FTransform<T, true> {
       FTransform(CProbabilityVector::value_type /*map*/)
       {
              throw invalid_argument("C2DRegiongrowFifoFilter doesn't support float input images");
       }

       double operator () (T /*x*/) const
       {
              throw invalid_argument("C2DRegiongrowFifoFilter doesn't support float input images");
       }
};


template <typename T>
int C2DRegiongrowFifoFilter::operator ()( const T2DImage<T>& image)
{
       const bool is_float = std::is_floating_point<T>::value;
       transform(image.begin(), image.end(), m_in_buffer.begin(),
                 FTransform<T, is_float>(m_probmap[m_class]));
       transform(m_in_buffer.begin(), m_in_buffer.begin() + image.size(),
                 m_out_buffer.begin(),
                 m_seed);
       return 0;
}

void C2DRegiongrowFifoFilter::do_push(::boost::call_traits<P2DImage>::param_type x)
{
       TRACE("C2DRegiongrowFifoFilter::do_push");
       mia::accumulate(*this, *x);
}

void C2DRegiongrowFifoFilter::do_initialize(::boost::call_traits<P2DImage>::param_type x)
{
       TRACE("C2DRegiongrowFifoFilter::do_initialize");
       m_slice_size = x->get_size();
       m_slice_emls = m_slice_size.x * m_slice_size.y;
       C3DBounds buf_size(m_slice_size.x, m_slice_size.y, m_depth + 1);
       m_in_buffer = C3DDImage(buf_size);
       m_out_buffer = C3DBitImage(buf_size);
}

void C2DRegiongrowFifoFilter::seed_env(const C3DBounds& center, queue<C3DBounds>& seeds) const
{
       C3DShape::const_iterator sb = m_shape->begin();
       C3DShape::const_iterator se = m_shape->end();

       while (sb != se) {
              C3DBounds x(center.x + sb->x, center.y + sb->y, center.z + sb->z);
              ++sb;

              if (x.z >= get_end())
                     continue;

              if ( m_slice_size.y <= x.y || m_slice_size.x <= x.x)
                     continue;

              if (!m_out_buffer(x) && m_in_buffer(x) >= m_low)
                     seeds.push(x);
       }
}

void  C2DRegiongrowFifoFilter::grow()
{
       queue<C3DBounds> seeds;
       C3DBitImage::const_iterator i = m_out_buffer.begin();

       for (size_t z = 0; z < get_end(); ++z)
              for (size_t y = 0; y < m_slice_size.y; ++y)
                     for (size_t x = 0; x < m_slice_size.x; ++x, ++i) {
                            if (*i)
                                   seed_env(C3DBounds(x, y, z), seeds);
                     }

       while (!seeds.empty()) {
              C3DBounds x = seeds.front();
              cvdebug() << "add " << x << "\n";
              seeds.pop();

              if (!m_out_buffer(x)) {
                     m_out_buffer(x) = true;
                     seed_env(x, seeds);
              }
       }
}

P2DImage C2DRegiongrowFifoFilter::do_filter()
{
       TRACE("C2DRegiongrowFifoFilter::do_filter");
       cvdebug() << "Range: [" << get_start() << ", " << get_end() << "]\n";
       grow();
       C2DBitImage *result = new C2DBitImage(m_slice_size);
       copy(m_out_buffer.begin_at(0, 0, get_start()),  m_out_buffer.begin_at(0, 0, get_start() + 1), result->begin());
       return P2DImage(result);
}

void C2DRegiongrowFifoFilter::shift_buffer()
{
       TRACE("C2DRegiongrowFifoFilter::shift_buffer");
       copy_backward(m_in_buffer.begin(), m_in_buffer.end() - m_slice_emls, m_in_buffer.end());
       copy_backward(m_out_buffer.begin(), m_out_buffer.end() - m_slice_emls, m_out_buffer.end());
}

class C2DRegiongrowFifoFilterPlugin : public C2DFifoFilterPlugin
{
public:
       C2DRegiongrowFifoFilterPlugin();
private:
       virtual const string do_get_descr() const;
       virtual C2DImageFifoFilter *do_create()const;

       string m_map;
       float m_low;
       float m_seed;
       int m_class;
       int m_depth;
};


C2DRegiongrowFifoFilterPlugin::C2DRegiongrowFifoFilterPlugin():
       C2DFifoFilterPlugin(plugin_name),
       m_low(0.5f),
       m_seed(0.98),
       m_class(2),
       m_depth(10)
{
       add_parameter("map", new CStringParameter(m_map, CCmdOptionFlags::required_input, "seed class map"));
       add_parameter("low", make_oci_param(m_low, .0f, 1.0f, false,
                                           "low threshold for acceptance probability"));
       add_parameter("seed", make_oci_param(m_seed, .0f, 1.0f, false,
                                            "threshold for seed probability"));
       add_parameter("class", make_lc_param(m_class, 0, false, "class to be segmented"));
       add_parameter("depth", make_lc_param(m_depth, 1, false, "number of slices to keep during processing"));
}


const string C2DRegiongrowFifoFilterPlugin::do_get_descr() const
{
       return "Run a region-growing filter on a stack of images that comprise "
              "a 3D image.  The region growing is based on class probabilities obtained from "
              "a c-means classification of the pixel intensities. One seed thresh is use to initiate "
              "a region, and another (lower) threshold is used to stop the region growing. "
              "By holding a number of 3D slices in the working memory to permit the region to grow \""
              "backwards \" in the stack a Quasi-3D processing is achieved. However, with complex "
              "structures the region growing may not properly be segmented.";
}

C2DImageFifoFilter *C2DRegiongrowFifoFilterPlugin::do_create()const
{
       return  new C2DRegiongrowFifoFilter(CProbabilityVector(m_map),
                                           m_low, m_seed, m_class, m_depth);
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DRegiongrowFifoFilterPlugin();
}
NS_END
