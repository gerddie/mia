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

#ifndef __mia_internal_convert_hh
#define __mia_internal_convert_hh

#include <mia/core/filter.hh>


NS_MIA_BEGIN

///  @cond DOC_PLUGINS
/*

  \ingroup templates
  \brief Generic pixel conversion for images

  This class implements the image pixel type conversion.
  Various mappings are supported and defined as  EPixelConversion.
  \tparam Image the image type that must provide STL-like iterators.
 */
template <class Image>
class TConvert: public TDataFilter<Image>
{
public:
       typedef typename TDataFilter<Image>::result_type result_type;

       /**
          Create the conversion filter with the parameters
          @param pt output pixel type
          @param ct conversion mapping to be applied
          @param a scale factor for linear mapping
          @param b shift factor for linear mapping
        */
       TConvert(mia::EPixelType pt, EPixelConversion ct, float a, float b);

       /**
          \tparam container type must support STL like forward iterator
          \tparam pixel type, must be a scalar data type
          \param data the data to be converted
          \returns the container with the converted data
          \remark review the template parameters, there seem to be more then necessary
        */
       template <template  <typename> class Data, typename T>
       typename TConvert<Image>::result_type operator () (const Data<T>& data) const;

private:
       template <template  <typename> class Data, typename  S, typename  T>
       typename TConvert<Image>::result_type convert(const Data<S>& src) const;
       typename TConvert::result_type do_filter(const Image& image) const;

       EPixelType m_pt;
       EPixelConversion m_ct;
       float m_a;
       float m_b;
};

/*
  \ingroup templates
  \brief Generic plug-in for pixel conversion image filters
*/
template <class Image>
class TConvertFilterPlugin: public TDataFilterPlugin<Image>
{
public:
       TConvertFilterPlugin();
private:
       virtual TDataFilter<Image> *do_create()const;
       virtual const std::string do_get_descr()const;

       EPixelType m_pixeltype;
       EPixelConversion m_convert;
       float m_a;
       float m_b;

};

/// @endcond DOC_PLUGINS

/**
   \cond INTERNAL
   \ingroup traits
*/

template <typename  T, bool is_float>
struct __mia_round  {
       static T apply(T x)
       {
              return x;
       }
};

template <typename  T>
struct __mia_round<T, false> {
       static T apply(long double x)
       {
              return static_cast<T>(floor(x + 0.5));
       }
};

template <typename T, bool is_float>
struct __dispatch_minmax {
       static std::pair<T, T> apply()
       {
              return std::pair<T, T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
       }
};

template <typename T>
struct __dispatch_minmax<T, true> {
       static std::pair<T, T> apply()
       {
              return std::pair<T, T>(-1.0f, 1.0f);
       }
};

template <typename T>
struct get_minmax {
       static std::pair<T, T> apply()
       {
              return __dispatch_minmax<T, std::is_floating_point<T>::value >::apply();
       }
};

/// @endcond INTERNAL

NS_MIA_END

#endif
