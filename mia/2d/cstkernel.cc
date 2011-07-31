/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

/*
   LatexBeginPluginSection{2D CST filter kernels}
   \label {sec:cst2dkern}
   
   These are kernels for filters that work in the frequency domain on the 
   image that was transformed by a cosinus or a sinus transform. 

   \LatexEnd
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

EXPLICIT_INSTANCE_HANDLER(CCST2DVectorKernel);
EXPLICIT_INSTANCE_HANDLER(CCST2DImageKernel);

NS_MIA_END
