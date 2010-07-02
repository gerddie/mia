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

// $Id: fftkernel.hh,v 1.1 2006-03-01 19:14:13 wollny Exp $

/*! \brief basic type of a plugin handler

A basis for fft filter plugins

\author Gert Wollny <wollny at die.upm.es>

*/

#ifndef mia_2d_fftkernel_hh
#define mia_2d_fftkernel_hh

#include <complex>
#include <fftw3.h>

#include <mia/core/defines.hh>
#include <mia/core/factory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/cstplan.hh>
#include <mia/2d/2DDatafield.hh>
#include <mia/2d/2DImage.hh>

NS_MIA_BEGIN

struct EXPORT_2D cst2d_vector_kernel {
	static const char *type_descr;
};

struct EXPORT_2D cst2d_image_kernel {
	static const char *type_descr;
};

template <typename T>
class EXPORT_2D TCST2DKernel :public CProductBase {
public:
	typedef TCSTPlan<T> CPlan;
	TCST2DKernel(fftwf_r2r_kind forward);
	virtual ~TCST2DKernel();
	void apply(const T& in, T& out) const;
	void prepare(const C2DBounds& size);


private:
        virtual CPlan *do_prepare(fftwf_r2r_kind fw_kind, const std::vector<int>& size) = 0;

	fftwf_r2r_kind _M_forward;
	auto_ptr<CPlan> _M_plan;
};

/*
  image filter
*/

typedef TCST2DKernel<C2DFVectorfield> CCST2DVectorKernel;
typedef TCST2DKernel<C2DFImage>       CCST2DImageKernel;

typedef  SHARED_PTR(CCST2DImageKernel) PCST2DImageKernel;
typedef  SHARED_PTR(CCST2DVectorKernel) PCST2DVectorKernel;

typedef TFactory<CCST2DVectorKernel, cst2d_vector_kernel, kernel_plugin_type> CCST2DVectorKernelPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CCST2DVectorKernelPlugin> > CCST2DVectorKernelPluginHandler;

typedef TFactory<CCST2DImageKernel, cst2d_image_kernel, kernel_plugin_type> CCST2DImgKernelPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CCST2DImgKernelPlugin> > CCST2DImgKernelPluginHandler;



NS_MIA_END

#endif

