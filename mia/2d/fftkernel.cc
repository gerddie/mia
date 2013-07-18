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

#include <mia/2d/fftkernel.hh>

#include <mia/core/plugin_base.cxx>
#include <mia/core/handler.cxx>

NS_MIA_BEGIN

using std::runtime_error; 

const char* fft2d_kernel_data::data_descr = "fft2d";

CFFT2DKernel::CFFT2DKernel():
	m_size(0,0),
	m_cbuffer(NULL),
	m_fbuffer(NULL),
	m_scale(1.0)
{
}

CFFT2DKernel::~CFFT2DKernel()
{
	tear_down();
}

void CFFT2DKernel::apply() const
{
	fftwf_execute( m_forward_plan);
	do_apply(m_size, m_realsize_x, m_cbuffer);
	fftwf_execute( m_backward_plan);
}

void CFFT2DKernel::tear_down()
{
	if (m_cbuffer) {
		fftwf_free(m_cbuffer);
		fftwf_free(m_fbuffer);
		fftwf_destroy_plan( m_forward_plan);
		fftwf_destroy_plan( m_backward_plan);
	}
}

float *CFFT2DKernel::prepare(const C2DBounds& size)
{
	if (m_size  == size)
		return m_fbuffer;

	tear_down();
	m_size = size;
	m_realsize_x = 2 * (m_size.x /2 + 1);

	m_scale = 1.0f / (m_size.x * m_size.y); 

	cvdebug() << "size = " << m_size.x << ", " << m_size.y << "\n";

	m_cbuffer = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * 
						 m_size.y * m_realsize_x);

	if (!m_cbuffer) {
		throw runtime_error("unable to allocate fftw buffers");
	}

	// create the fftw plans
	m_fbuffer = (float *)fftwf_malloc(sizeof(fftwf_complex) * m_size.y * m_size.x);
	if (!m_fbuffer) {
		fftwf_free(m_cbuffer);
		throw runtime_error("unable to allocate fftw buffers");
	}

	cvdebug() << "buffer at " << m_cbuffer << ":" << m_fbuffer << "\n";

	m_forward_plan = fftwf_plan_dft_r2c_2d(m_size.y, m_size.x,
					       m_fbuffer, m_cbuffer, FFTW_ESTIMATE);
	
	cvdebug() << "forward plan at " <<  m_forward_plan << "\n";
	if (!m_forward_plan) {
		fftwf_free(m_cbuffer);
		fftwf_free(m_fbuffer);
		throw runtime_error("unable to create forward plans ...");
	}
	
	m_backward_plan = fftwf_plan_dft_c2r_2d(m_size.y, m_size.x,
						m_cbuffer, m_fbuffer, FFTW_ESTIMATE);
	
	if (!m_backward_plan) {
		fftwf_free(m_cbuffer);
		fftwf_free(m_fbuffer);
		fftwf_destroy_plan( m_forward_plan);
		throw runtime_error("unable to create backward plans ...");
	}
	return m_fbuffer;
}

template <> const char *  const 
TPluginHandler<TFactory<CFFT2DKernel>>::m_help =  "These plug-ins define kernels for 2D processing "
				"in the Fourier transformed space.";

EXPLICIT_INSTANCE_HANDLER(CFFT2DKernel);

NS_MIA_END
