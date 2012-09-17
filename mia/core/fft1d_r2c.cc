/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#include <mia/core/fft1d_r2c.hh>
#include <mia/core/msgstream.hh>
#include <stdexcept>

NS_MIA_BEGIN


using namespace std;
struct CFFT1D_R2CImpl {
	CFFT1D_R2CImpl(size_t n);
	~CFFT1D_R2CImpl();

	vector<CFFT1D_R2C::Complex> forward(const vector<CFFT1D_R2C::Real>& data) const;
	vector<CFFT1D_R2C::Real>    backward(const vector<CFFT1D_R2C::Complex>& data) const;

	void forward(vector<CFFT1D_R2C::Real>::const_iterator in_begin,
		     vector<CFFT1D_R2C::Real>::const_iterator in_end,
		     vector<CFFT1D_R2C::Complex>::iterator out_begin) const;

	void backward(vector<CFFT1D_R2C::Complex>::const_iterator in_begin,
		      vector<CFFT1D_R2C::Complex>::const_iterator in_end,
		      vector<CFFT1D_R2C::Real>::iterator out_begin) const;


	size_t m_in_size;
	size_t m_out_size;

	fftwf_plan m_forward_plan;
	fftwf_plan m_backward_plan;

	float *m_in;
	fftwf_complex *m_out;


};

CFFT1D_R2C::CFFT1D_R2C(size_t n)
{
	TRACE_FUNCTION;
	impl = new CFFT1D_R2CImpl(n);
}

CFFT1D_R2C::~CFFT1D_R2C()
{
	TRACE_FUNCTION;
	delete impl;
}

size_t CFFT1D_R2C::out_size() const
{
	return impl->m_out_size;
}

vector<CFFT1D_R2C::Complex> CFFT1D_R2C::forward(const vector<Real>& data) const
{
	TRACE_FUNCTION;
	return impl->forward(data);
}

vector<CFFT1D_R2C::Real>    CFFT1D_R2C::backward(const vector<Complex>& data) const
{
	TRACE_FUNCTION;
	return impl->backward(data);
}


CFFT1D_R2CImpl::CFFT1D_R2CImpl(size_t n):
	m_in_size(n),
	m_out_size(n/2 + 1)
{
	TRACE_FUNCTION;
	string msg;
	m_in = (float *) fftwf_malloc(sizeof(fftwf_complex) * m_out_size);
	if (NULL == m_in) {
		msg = "unable to allocate FFTW data";
		goto in_fail;
	}
	m_out  = (fftwf_complex *) m_in;
	m_forward_plan  = fftwf_plan_dft_r2c_1d(m_in_size, m_in, m_out, FFTW_ESTIMATE);
	m_backward_plan  = fftwf_plan_dft_c2r_1d(m_in_size, m_out, m_in, FFTW_ESTIMATE);
	return;

 in_fail:
	throw runtime_error(msg);

}

CFFT1D_R2CImpl::~CFFT1D_R2CImpl()
{
	fftwf_destroy_plan(m_backward_plan);
	fftwf_destroy_plan(m_forward_plan);
	fftwf_free(m_in);
}

void CFFT1D_R2C::forward(vector<Real>::const_iterator in_begin,
			     vector<Real>::const_iterator in_end,
			     vector<Complex>::iterator out_begin) const
{
	impl->forward(in_begin, in_end, out_begin);
}

void CFFT1D_R2C::backward(vector<Complex>::const_iterator in_begin,
			      vector<Complex>::const_iterator in_end,
			      vector<Real>::iterator out_begin) const
{
	impl->backward(in_begin, in_end, out_begin);
}

vector<CFFT1D_R2C::Complex> CFFT1D_R2CImpl::forward(const vector<CFFT1D_R2C::Real>& data) const
{
	vector<CFFT1D_R2C::Complex> result(m_out_size);
	forward(data.begin(), data.end(), result.begin());
	return result;
}

vector<CFFT1D_R2C::Real> CFFT1D_R2CImpl::backward(const vector<CFFT1D_R2C::Complex>& data) const
{
	vector<CFFT1D_R2C::Real> result(m_in_size);
	backward(data.begin(), data.end(), result.begin());
	return result;
}


void CFFT1D_R2CImpl::forward(vector<CFFT1D_R2C::Real>::const_iterator in_begin,
			     vector<CFFT1D_R2C::Real>::const_iterator in_end,
			     vector<CFFT1D_R2C::Complex>::iterator out_begin) const
{
	copy(in_begin, in_end, m_in);
	fftwf_execute( m_forward_plan);
	copy(m_out, m_out + m_out_size, out_begin);
}

void CFFT1D_R2CImpl::backward(vector<CFFT1D_R2C::Complex>::const_iterator in_begin,
			      vector<CFFT1D_R2C::Complex>::const_iterator MIA_PARAM_UNUSED(in_end),
			      vector<CFFT1D_R2C::Real>::iterator out_begin) const
{
	assert((size_t)distance(in_begin, in_end) == m_out_size);
	for (size_t i = 0; i < m_out_size; ++i, ++in_begin) {
		m_out[i][0] = in_begin->real();
		m_out[i][1] = in_begin->imag();
	}
	fftwf_execute( m_backward_plan);
	copy(m_in, m_in + m_in_size, out_begin);
}

NS_MIA_END
