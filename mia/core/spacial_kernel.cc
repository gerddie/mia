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

#define VSTREAM_DOMAIN "SPACIAL KERNEL"

#include <mia/core/export_handler.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace boost;
using namespace std;

const char *spacial_kernel_data::data_descr = "1d";
const char *kernel_plugin_type::type_descr = "spacialkernel";


C1DFilterKernel::C1DFilterKernel(unsigned fsize):
       m_fsize(fsize)
{
}



int C1DFilterKernel::get_fsize()const
{
       return m_fsize;
}

size_t C1DFilterKernel::size()const
{
       return do_size();
}

void C1DFilterKernel::apply_inplace(std::vector<double>& data) const
{
       vector<double> tmp = apply(data);
       copy(tmp.begin(), tmp.end(), data.begin());
}

vector<double> C1DFilterKernel::apply(const vector<double>& data) const
{
       return do_apply(data);
}

C1DFilterKernel::~C1DFilterKernel()
{
}

C1DFoldingKernel::C1DFoldingKernel(int fsize):
       C1DFilterKernel(fsize),
       m_mask(2 * fsize + 1),
       m_derivative(2 * fsize + 1)
{
}

C1DFoldingKernel::const_iterator C1DFoldingKernel::begin()const
{
       return m_mask.begin();
}

C1DFoldingKernel::const_iterator C1DFoldingKernel::end()const
{
       return m_mask.end();
}

C1DFoldingKernel::const_iterator C1DFoldingKernel::dbegin()const
{
       return m_derivative.begin();
}

C1DFoldingKernel::const_iterator C1DFoldingKernel::dend()const
{
       return m_derivative.end();
}

C1DFoldingKernel::iterator C1DFoldingKernel::begin()
{
       return m_mask.begin();
}

C1DFoldingKernel::iterator C1DFoldingKernel::end()
{
       return m_mask.end();
}

C1DFoldingKernel::iterator C1DFoldingKernel::dbegin()
{
       return m_derivative.begin();
}

C1DFoldingKernel::iterator C1DFoldingKernel::dend()
{
       return m_derivative.end();
}

size_t C1DFoldingKernel::do_size()const
{
       return m_mask.size();
}



template<>  const char *const
TPluginHandler<TFactory<C1DFoldingKernel>>::m_help =
              "These plug-ins provide folding kernel(s) for spacial separable filtering.";

EXPLICIT_INSTANCE_HANDLER(C1DFoldingKernel);

NS_MIA_END
