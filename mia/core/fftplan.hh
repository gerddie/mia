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
	mutable T *_M_in; 
	mutable T *_M_out; 

	fftwf_plan _M_forward_plan; 
	fftwf_plan _M_backward_plan; 
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
	if (NULL == (_M_in = (T *) fftwf_malloc(sizeof(T) * n))) {
		msg = "unable to allocate FFTW data"; 
		goto in_fail; 
	}

	if ( NULL == (_M_out  = (T *) fftwf_malloc(sizeof(T) * n))) {
		msg = "unable to allocate FFTW data"; 
		goto out_fail; 
	}
	
	if (0 == (_M_forward_plan = fftwf_plan_many_r2r(rank, n, rank, 
						       _M_in, NULL, rank, 1, 
						       _M_out, NULL, rank, 1, 
						       &fw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW forward plan"; 
		goto plan_fw_fail; 
	}

	if (0 == (_M_backward_plan = fftwf_plan_many_r2r(rank, n, rank, 
							_M_out, NULL, rank, 1, 
							_M_in,  NULL, rank, 1, 
							&bw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW backward plan";
		goto  plan_bw_fail; 
	}
	
	return; 

 plan_bw_fail:		
	fftwf_destroy_plan(_M_forward_plan); 
 plan_fw_fail:
	fftwf_free(_M_out); 
 out_fail:
	fftwf_free(_M_in); 
 in_fail: 
	throw std::runtime_error(msg); 
}
	
template <typename T> 
TFFTPlan<T>::~TFFTPlan()
{
	fftwf_destroy_plan(_M_backward_plan); 
	fftwf_destroy_plan(_M_forward_plan); 
	fftwf_free(_M_out); 
	fftwf_free(_M_in); 

}

template <typename F>
void TFFTPlan<T>::execute(I cbegin, I cend, O cbegin, const F&  op) const
{
	op.prepare(_M_in);
	fftwf_execute( _M_forward_plan); 
	op.run(_M_out); 
	fftwf_execute(_M_backward_plan);
	op.get_result(_M_in); 
}
	
NS_MIA_END
