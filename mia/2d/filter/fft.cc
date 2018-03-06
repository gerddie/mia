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
#include <mia/core/msgstream.hh>
#include <mia/2d/filter/fft.hh>

NS_BEGIN(fft_2dimage_filter)
NS_MIA_USE;
using namespace std;

C2DFft::C2DFft(const PFFT2DKernel&  kernel):
       m_kernel(kernel)
{
}

template <typename T, bool is_integral>
struct FBackConvert {
       FBackConvert(float scale):
              m_scale(scale)
       {
              cvdebug() << "scale = " << m_scale << "\n";
       }

       T operator ()(float x)
       {
              return T(x * m_scale);
       }
private:
       float m_scale;
};

template <typename T>
struct FBackConvert<T, true> {
       FBackConvert(float scale):
              m_scale(scale)
       {
              cvdebug() << "scale = " << m_scale << "\n";
       }

       T operator ()(float x)
       {
              float xc = x * m_scale;
              return xc < numeric_limits<T>::min() ? numeric_limits<T>::min() :
                     ( xc < numeric_limits<T>::max() ?  T(xc) : numeric_limits<T>::max());
       }
private:
       float m_scale;
};


template <typename T>
typename C2DFft::result_type C2DFft::operator () (const T2DImage<T>& image) const
{
       cvdebug() << "C2DFFT::operator() begin\n";
       float *buffer = m_kernel->prepare(image.get_size());
       size_t realsize_x = image.get_size().x;

       // fill buffer with image data
       for (size_t y = 0; y < image.get_size().y; ++y)
              if (y & 1)
                     transform(image.begin_at(0, y), image.begin_at(0, y) + image.get_size().x, &buffer[y * realsize_x],
                               bind2nd(multiplies<float>(), -1));
              else
                     copy(image.begin_at(0, y), image.begin_at(0, y) + image.get_size().x, &buffer[y *  realsize_x]);

       m_kernel->apply();
       T2DImage<T> *result = new T2DImage<T>(image.get_size(), image);
       const bool is_integral = ::boost::is_integral<T>::value;
       float scale = 1.0 / image.size();
       FBackConvert<T, is_integral> convertp(scale);
       FBackConvert<T, is_integral> convertm(-scale);

       for (size_t y = 0; y < image.get_size().y; ++y)
              if (y & 1)
                     transform(&buffer[y * realsize_x], &buffer[y * realsize_x + image.get_size().x], result->begin_at(0, y), convertm);
              else
                     transform(&buffer[y * realsize_x], &buffer[y * realsize_x + image.get_size().x], result->begin_at(0, y), convertp);

       return P2DImage(result);
}

P2DImage C2DFft::do_filter(const C2DImage& image) const
{
       return mia::filter(*this, image);
}


C2DFilterPluginFactory::C2DFilterPluginFactory():
       C2DFilterPlugin("fft")
{
       add_parameter("k", mape_param(m_kernel, "", true, "filter kernel"));
}

C2DFilter *C2DFilterPluginFactory::do_create()const
{
       return new C2DFft(m_kernel);
}

const string C2DFilterPluginFactory::do_get_descr()const
{
       return "Run a filter in the frequency domain by applying a forward real2complex fft, "
              "running the filter, and applying the backward complex2real fft. ";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C2DFilterPluginFactory();
}

NS_END
