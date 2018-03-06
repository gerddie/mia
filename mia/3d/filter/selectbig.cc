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
#include <stdexcept>
// boost type trains are needed to check whether we are dealing with an integer pixel valued image
#include <boost/type_traits.hpp>

// this is for the definition of the 3D image plugin base classes
#include <mia/3d/filter/selectbig.hh>


// start a namespace to avoid naming conflicts during runtime

NS_BEGIN(selectbig_3dimage_filter)

// don't need to write all the names ...
NS_MIA_USE;
using namespace std;

C3DSelectBig::C3DSelectBig()
{
}

template <typename T, bool is_float>
struct __get_big_index {
       static T apply(const T3DImage<T>& /*data*/)
       {
              throw invalid_argument("C3DselectBig: only support integral input types");
       }
};

template <typename T>
struct __get_big_index<T, true> {
       static T apply(const T3DImage<T>& data)
       {
              map<T, long> values;

              for (auto i = data.begin(); i != data.end(); ++i) {
                     auto k = values.find(*i);

                     if (k == values.end())
                            values[*i] = 1;
                     else
                            ++k->second;
              }

              values[0] = 0;
              auto vi = values.begin();
              T max_idx = vi->first;
              long max_cnt = vi->second;
              ++vi;

              while (vi != values.end()) {
                     if (max_cnt < vi->second) {
                            max_cnt = vi->second;
                            max_idx = vi->first;
                     }

                     ++vi;
              }

              return max_idx;
       }
};

template <typename T>
struct FEquals {
       FEquals(T val): m_testval(val) {}
       bool operator() (T value) const
       {
              return value == m_testval;
       };
private:
       T m_testval;
};

/* This is the work horse operator of the filter. */
template <typename T>
C3DSelectBig::result_type C3DSelectBig::operator () (const mia::T3DImage<T>& data) const
{
       const bool is_integral = ::boost::is_integral<T>::value;
       T idx = __get_big_index<T, is_integral>::apply(data);
       C3DBitImage *result = new C3DBitImage(data.get_size(), data);

       if (!result) {
              stringstream err;
              err << "selectbig: unable to allocate image of size " << data.get_size().x << "x" << data.get_size().y;
              throw runtime_error(err.str());
       }

       transform(data.begin(), data.end(), result->begin(),
                 FEquals<T>(idx));
       return P3DImage(result);
}


/* The actual filter dispatch function calls the filter by selecting the right pixel type through wrap_filter */
P3DImage C3DSelectBig::do_filter(const C3DImage& image) const
{
       return mia::filter(*this, image);
}

/* The factory constructor initialises the plugin name, and takes care that the plugin help will show its parameters */
C3DSelectBigImageFilterFactory::C3DSelectBigImageFilterFactory():
       C3DFilterPlugin("selectbig")
{
}

/* The factory create function creates and returns the filter with the given options*/
C3DFilter *C3DSelectBigImageFilterFactory::do_create()const
{
       return new C3DSelectBig;
}

/* This function sreturns a short description of the filter */
const string C3DSelectBigImageFilterFactory::do_get_descr()const
{
       return "A filter that creats a binary mask representing the intensity with the highest pixel count."
              "The pixel value 0 will be ignored, and if two intensities have the same pixel count, "
              "then the result is undefined. The input pixel must have an integral pixel type.";
}

/*
  Finally the C-interface function that returns the filter factory. It's linking style is "C"
  to avoid the name-mangling of C++. A clean name is needed, because this function is aquired
  by its "real"(compiled) name.
*/
extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C3DSelectBigImageFilterFactory();
}

NS_END
