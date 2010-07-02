/* -*- mona-c++  -*-
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


#include <mia/2d/cstkernel.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

typedef TCST2DKernel<C2DFVectorfield> CCST2DVectorKernel;
typedef TCST2DKernel<C2DFImage>       CCST2DImageKernel;

const char* cst2d_vector_kernel::type_descr = "cst2dvectorkernel";
const char* cst2d_image_kernel::type_descr = "cst2dimagekernel";

template <typename T>
TCST2DKernel<T>::TCST2DKernel(fftwf_r2r_kind forward):_M_forward(forward)
{
}

template <typename T>
TCST2DKernel<T>::~TCST2DKernel()
{
}

template <typename T>
void TCST2DKernel<T>::apply(const T& in, T& out) const
{
	assert(_M_plan.get());
	assert(in.get_size() == out.get_size());
	assert(2 == _M_plan->get_size().size() &&
	       _M_plan->get_size()[0] == (int)out.get_size().x &&
	       _M_plan->get_size()[1] == (int)out.get_size().y
		);


	_M_plan->execute(in, out);
}

template <typename T>
void TCST2DKernel<T>::prepare(const C2DBounds& s)
{
	if (_M_plan.get() &&
	    _M_plan->get_size().size() == 2 &&
	    _M_plan->get_size()[0] == (int)s.x   &&
	    _M_plan->get_size()[1] == (int)s.y )
		return;

	std::vector<int> size(2);
	size[0] = s.x;
	size[1] = s.y;


	cvdebug() << "size = " << s.x << ", " << s.y << "\n";
	_M_plan.reset(do_prepare(_M_forward, size));
}



template class TCST2DKernel<C2DFVectorfield>;
template class TCST2DKernel<C2DFImage>;

template class TPlugin<cst2d_vector_kernel, kernel_plugin_type>;
template class TFactory<CCST2DVectorKernel, cst2d_vector_kernel, kernel_plugin_type> ;
template class TPluginHandler<CCST2DVectorKernelPlugin>;
template class TFactoryPluginHandler<CCST2DVectorKernelPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<CCST2DVectorKernelPlugin> > ;

template class TPlugin<cst2d_image_kernel, kernel_plugin_type>;
template class TFactory<CCST2DImageKernel, cst2d_image_kernel, kernel_plugin_type> ;
template class TPluginHandler<CCST2DImgKernelPlugin>;
template class TFactoryPluginHandler<CCST2DImgKernelPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<CCST2DImgKernelPlugin> > ;

NS_MIA_END
