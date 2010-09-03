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

#include <mia/2d/fftkernel.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

const char* fft2d_kernel_data::type_descr = "fft2d";

CFFT2DKernel::CFFT2DKernel():
	_M_size(0,0),
	_M_cbuffer(NULL)
{
}

CFFT2DKernel::~CFFT2DKernel()
{
	tear_down();
}

void CFFT2DKernel::apply() const
{
	fftwf_execute( _M_forward_plan);
	do_apply(_M_size, _M_realsize_x, _M_cbuffer);
	fftwf_execute( _M_backward_plan);
}

void CFFT2DKernel::tear_down()
{
	if (_M_cbuffer) {
		fftwf_free(_M_cbuffer);
		fftwf_free(_M_fbuffer);
		fftwf_destroy_plan( _M_forward_plan);
		fftwf_destroy_plan( _M_backward_plan);
	}
}

float *CFFT2DKernel::prepare(const C2DBounds& size)
{
	if (_M_size  == size)
		return _M_fbuffer;

	tear_down();
	_M_size = size;
	_M_realsize_x = 2 * (_M_size.x /2 + 1);

	cvdebug() << "size = " << _M_size.x << ", " << _M_size.y << "\n";

	_M_cbuffer = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * _M_size.y * _M_realsize_x);

	if (!_M_cbuffer) {
		throw runtime_error("unable to allocate fftw buffers");
	}

	// create the fftw plans
	_M_fbuffer = (float *)fftwf_malloc(sizeof(fftwf_complex) * _M_size.y * _M_size.x);
	if (!_M_fbuffer) {
		fftwf_free(_M_cbuffer);
		throw runtime_error("unable to allocate fftw buffers");
	}

	cvdebug() << "buffer at " << _M_cbuffer << ":" << _M_fbuffer << "\n";


	 _M_forward_plan = fftwf_plan_dft_r2c_2d(_M_size.y, _M_size.x,
						_M_fbuffer, _M_cbuffer, FFTW_ESTIMATE);

	 cvdebug() << "forward plan at " <<  _M_forward_plan << "\n";
	 if (!_M_forward_plan) {
		 fftwf_free(_M_cbuffer);
		 fftwf_free(_M_fbuffer);
		 throw runtime_error("unable to create forward plans ...");
	 }

	 _M_backward_plan = fftwf_plan_dft_c2r_2d(_M_size.y, _M_size.x,
						 _M_cbuffer, _M_fbuffer, FFTW_ESTIMATE);

	 if (!_M_backward_plan) {
		 fftwf_free(_M_cbuffer);
		 fftwf_free(_M_fbuffer);
		 fftwf_destroy_plan( _M_forward_plan);
		 throw runtime_error("unable to create backward plans ...");
	 }
	 return _M_fbuffer;
}

template class TPlugin<fft2d_kernel_data, kernel_plugin_type>;
template class TFactory<CFFT2DKernel>;
template class TPluginHandler<CFFT2DKernelPlugin>;
template class TFactoryPluginHandler<CFFT2DKernelPlugin>;
template class THandlerSingleton<TFactoryPluginHandler<CFFT2DKernelPlugin> >;




NS_MIA_END
