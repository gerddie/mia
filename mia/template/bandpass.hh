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

#ifndef __mia_internal_bandpass_hh
#define __mia_internal_bandpass_hh

#include <limits>
#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

///  @cond DOC_PLUGINS

template <class Image>
class TBandPass: public TDataFilter<Image>
{
public:
       typedef typename TDataFilter<Image>::result_type result_type;
       TBandPass(float min, float max);

       template <class  T>
       typename TBandPass::result_type operator () (const T& data) const;

private:
       typename TBandPass::result_type do_filter(const Image& image) const;

       float m_min;
       float m_max;
};

template <class Image>
class TBandPassFilterPlugin: public TDataFilterPlugin<Image>
{
public:
       TBandPassFilterPlugin();
       virtual TDataFilter<Image> *do_create()const;
       virtual const std::string do_get_descr()const;
private:
       float m_min;
       float m_max;
};

NS_MIA_END

///  @endcond DOC_PLUGINS

#endif
