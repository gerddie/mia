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

#include <stdexcept>
#include <string>
#include <vector>

#include <fftw3.h>

#include <mia/core/defines.hh>

NS_MIA_BEGIN

template <typename T> 
class TFFTPlan {
public: 
	TFFTPlan(fftwf_r2r_kind forward, fftwf_r2r_kind backward, size_t rank, size_t *size); 

	~TFFTPlan(); 

	template <typename F>
	void execute(const F&  op) const; 
	
private: 
	mutable T *m_in; 
	mutable T *m_out; 

	fftwf_plan m_forward_plan; 
	fftwf_plan m_backward_plan; 
}; 


template <typename T> 
TFFTPlan<T>::TFFTPlan(fftwf_r2r_kind forward, fftwf_r2r_kind backward, size_t rank, size_t *size)
{
	std::string msg; 
	
	vector<fftwf_r2r_kind> fw_kind(rank, forward); 
	vector<fftwf_r2r_kind> bw_kind(rank, backward); 
	

	size_t n = 1; 
	for (int i = 0; i < rank; ++i) 
		n *= size[i]; 

	;
	if (NULL == (m_in = (T *) fftwf_malloc(sizeof(T) * n))) {
		msg = "unable to allocate FFTW data"; 
		goto in_fail; 
	}

	if ( NULL == (m_out  = (T *) fftwf_malloc(sizeof(T) * n))) {
		msg = "unable to allocate FFTW data"; 
		goto out_fail; 
	}
	
	if (0 == (m_forward_plan = fftwf_plan_many_r2r(rank, n, rank, 
						       m_in, NULL, rank, 1, 
						       m_out, NULL, rank, 1, 
						       &fw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW forward plan"; 
		goto plan_fw_fail; 
	}

	if (0 == (m_backward_plan = fftwf_plan_many_r2r(rank, n, rank, 
							m_out, NULL, rank, 1, 
							m_in,  NULL, rank, 1, 
							&bw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW backward plan";
		goto  plan_bw_fail; 
	}
	
	return; 

 plan_bw_fail:		
	fftwf_destroy_plan(m_forward_plan); 
 plan_fw_fail:
	fftwf_free(m_out); 
 out_fail:
	fftwf_free(m_in); 
 in_fail: 
	throw std::runtime_error(msg); 
}
	
template <typename T> 
TFFTPlan<T>::~TFFTPlan()
{
	fftwf_destroy_plan(m_backward_plan); 
	fftwf_destroy_plan(m_forward_plan); 
	fftwf_free(m_out); 
	fftwf_free(m_in); 

}

template <typename F>
void TFFTPlan<T>::execute(I cbegin, I cend, O cbegin, const F&  op) const
{
	op.prepare(m_in);
	fftwf_execute( m_forward_plan); 
	op.run(m_out); 
	fftwf_execute(m_backward_plan);
	op.get_result(m_in); 
}
	
NS_MIA_END
