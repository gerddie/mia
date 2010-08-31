/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "SPACIAL KERNEL"

#include <mia/core/export_handler.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN
using namespace boost;

const char *spacial_kernel_data::type_descr = "spacial";
const char *kernel_plugin_type::value = "kernel";


C1DFilterKernel::C1DFilterKernel(int fsize):
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


template class TPlugin<spacial_kernel_data, kernel_plugin_type>;
template class TFactory<C1DFoldingKernel, spacial_kernel_data, kernel_plugin_type>;
template class TPluginHandler<C1DSpacialKernelPlugin>;
template class TFactoryPluginHandler<C1DSpacialKernelPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<C1DSpacialKernelPlugin> >;



NS_MIA_END
