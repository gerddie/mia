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
	m_size(0,0),
	m_cbuffer(NULL)
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

	cvdebug() << "size = " << m_size.x << ", " << m_size.y << "\n";

	m_cbuffer = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * m_size.y * m_realsize_x);

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

EXPLICIT_INSTANCE_HANDLER(CFFT2DKernel);

NS_MIA_END
