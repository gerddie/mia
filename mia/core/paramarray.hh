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

#ifndef mia_core_paramarray_hh
#define mia_core_paramarray_hh

#include <mia/core/cmdlineparser.hh>

#include <cassert>

namespace mia
{

template <typename T>
class TPerLevelScalarParam
{
public:
       TPerLevelScalarParam(T default_value);

       PCmdOption create_level_params_option(const char *long_name,
                                             char short_name,
                                             EParameterBounds flags,
                                             const std::vector<T>& boundaries,
                                             const char *help);

       PCmdOption create_level_params_option(const char *long_name,
                                             char short_name,
                                             const char *help);


       T operator [](unsigned l)const;
private:
       std::vector<T> m_params;
       T m_default_value;
};

template <typename T>
TPerLevelScalarParam<T>::TPerLevelScalarParam(T default_value):
       m_default_value(default_value)
{
}

template <typename T>
PCmdOption TPerLevelScalarParam<T>::create_level_params_option(const char *long_opt,
              char short_opt,
              EParameterBounds bflags,
              const std::vector<T>& boundaries,
              const char *help)
{
       return PCmdOption(new CParamOption( short_opt, long_opt,
                                           new TBoundedParameter<std::vector<T>>(m_params, bflags,
                                                         boundaries, false, help)));
}

template <typename T>
PCmdOption TPerLevelScalarParam<T>::create_level_params_option(const char *long_opt,
              char short_opt,
              const char *help)
{
       return PCmdOption(new CParamOption( short_opt, long_opt,
                                           new CTParameter<std::vector<T>>(m_params, false, help)));
}

template <typename T>
T TPerLevelScalarParam<T>::operator [](unsigned l) const
{
       if (m_params.empty())
              return m_default_value;

       return l < m_params.size() ? m_params[l] : m_params[m_params.size() - 1];
}


} // namespace mia

#endif
