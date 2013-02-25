/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/2d/cstkernel.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

typedef TCST2DKernel<C2DFVectorfield> CCST2DVectorKernel;
typedef TCST2DKernel<C2DFImage>       CCST2DImageKernel;

const char* cst2d_kernel::type_descr = "cst2d-kernel";

template <typename T>
TCST2DKernel<T>::TCST2DKernel(fftwf_r2r_kind forward):m_forward(forward)
{
}

template <typename T>
TCST2DKernel<T>::~TCST2DKernel()
{
}

template <typename T>
void TCST2DKernel<T>::apply(const T& in, T& out) const
{
	assert(m_plan.get());
	assert(in.get_size() == out.get_size());
	assert(2 == m_plan->get_size().size() &&
	       m_plan->get_size()[0] == (int)out.get_size().x &&
	       m_plan->get_size()[1] == (int)out.get_size().y
		);


	m_plan->execute(in, out);
}

template <typename T>
void TCST2DKernel<T>::prepare(const C2DBounds& s)
{
	if (m_plan.get() &&
	    m_plan->get_size().size() == 2 &&
	    m_plan->get_size()[0] == (int)s.x   &&
	    m_plan->get_size()[1] == (int)s.y )
		return;

	std::vector<int> size(2);
	size[0] = s.x;
	size[1] = s.y;


	cvdebug() << "size = " << s.x << ", " << s.y << "\n";
	m_plan.reset(do_prepare(m_forward, size));
}



template class TCST2DKernel<C2DFVectorfield>;
template class TCST2DKernel<C2DFImage>;


template <> const char *  const 
TPluginHandler<TFactory<CCST2DImageKernel>>::m_help =  "These plug-ins define kernels for 2D processing of images "
							"in the Cosine transformed space.";

template <> const char *  const 
TPluginHandler<TFactory<CCST2DVectorKernel>>::m_help =  "These plug-ins define kernels for 2D processing of vector fields "
							"in the Cosine transformed space.";

EXPLICIT_INSTANCE_HANDLER(CCST2DVectorKernel);
EXPLICIT_INSTANCE_HANDLER(CCST2DImageKernel);


NS_MIA_END
