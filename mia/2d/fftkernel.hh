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
#include <mia/2d/defines2d.hh>
#include <mia/2d/2DVector.hh>

NS_MIA_BEGIN

struct EXPORT_2D fft2d_kernel_data {
	static const char *type_descr;
};

class EXPORT_2D CFFT2DKernel :public CProductBase {
public:
	CFFT2DKernel();
	virtual ~CFFT2DKernel();
	void apply() const;
	float *prepare(const C2DBounds& size);
private:
	void tear_down();
	virtual void do_apply(const C2DBounds& _M_size, size_t _M_realsize_x, fftwf_complex *_M_cbuffer) const = 0;

	C2DBounds _M_size;
	fftwf_complex *_M_cbuffer;
	float   *_M_fbuffer;
	float _M_scale;
	fftwf_plan _M_forward_plan;
	fftwf_plan _M_backward_plan;

	size_t _M_realsize_x;
};

typedef  std::shared_ptr<CFFT2DKernel > PFFT2DKernel;


typedef TFactory<CFFT2DKernel, fft2d_kernel_data, kernel_plugin_type> CFFT2DKernelPlugin;
typedef THandlerSingleton<TFactoryPluginHandler<CFFT2DKernelPlugin> > CFFT2DKernelPluginHandler;

NS_MIA_END

#endif

