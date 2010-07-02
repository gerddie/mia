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

#include <stdexcept>
#include <string>
#include <vector>

#include <fftw3.h>

#include <mia/core/msgstream.hh>

NS_MIA_BEGIN


template <typename T> 
class TCSTPlan {
public: 
	typedef typename T::value_type value_type; 

	TCSTPlan(fftwf_r2r_kind forward, std::vector<int> size); 

	virtual ~TCSTPlan(); 

	template <typename D>
	void execute(const D& in_data, D& out_data) const; 

	const std::vector<int>& get_size() const; 

private: 
	mutable value_type *_M_in; 
	mutable value_type *_M_out; 

	std::vector<int> _M_size; 
	float _M_scale; 
	size_t _M_n; 

	fftwf_plan _M_forward_plan; 
	fftwf_plan _M_backward_plan; 

	virtual void do_execute(value_type *buffer) const = 0;
	
	struct Scale {
		Scale(float scale): _M_scale(scale){
		}
		value_type operator ()(const value_type& x) const{
			return _M_scale * x; 
		}
		float _M_scale; 
	}; 
}; 

template <typename T> 
const std::vector<int>& TCSTPlan<T>::get_size() const
{
	return _M_size; 
}

template <typename T> 
TCSTPlan<T>::TCSTPlan(fftwf_r2r_kind forward, std::vector<int> size):
	_M_size(size),
	_M_scale(1.0)
{
	std::string msg; 
	size_t rank = _M_size.size(); 
	fftwf_r2r_kind backward; 


	switch (forward) {
	case FFTW_RODFT00: 
		for (size_t i = 0; i < rank; ++i)
			_M_scale *= 0.5 / ( size[i] + 1.0); 
		break; 
	case FFTW_REDFT00: 
		for (size_t i = 0; i < rank; ++i)
			_M_scale *= 0.5 / ( size[i] - 1.0); 
		break; 
	default: 
		for (size_t i = 0; i < rank; ++i)
			_M_scale *= 0.5 / size[i]; 
		break; 
	}

	switch (forward) {
	case FFTW_RODFT00: 
	case FFTW_REDFT00: 
	case FFTW_RODFT11:
	case FFTW_REDFT11:
		backward = forward; 
		break; 
	case FFTW_RODFT10: 
		backward = FFTW_RODFT01; 
		break; 
	case FFTW_REDFT10: 
		backward = FFTW_REDFT01; 
		break; 
	case FFTW_RODFT01:
		backward = FFTW_RODFT10; 
		break; 
	case FFTW_REDFT01:
		backward = FFTW_REDFT10; 
		break; 
	default: 
		throw std::invalid_argument("TCSTPlan: unknown transformtion requested"); 
	}
	
	std::vector<fftwf_r2r_kind> fw_kind(rank, forward); 
	std::vector<fftwf_r2r_kind> bw_kind(rank, backward); 


        const int howmany = sizeof(value_type)/sizeof(float); 
	cvdebug() << "howmany = " << howmany << "\n"; 
	_M_n = 1; 
	for (size_t i = 0; i < rank; ++i) 
		_M_n *= _M_size[i]; 


	if (NULL == (_M_in = (value_type *) fftwf_malloc(sizeof(T) * _M_n))) {
		msg = "unable to allocate FFTW data"; 
		goto in_fail; 
	}

	if ( NULL == (_M_out  = (value_type *) fftwf_malloc(sizeof(T) * _M_n))) {
		msg = "unable to allocate FFTW data"; 
		goto out_fail; 
	}
	
        if (0 == (_M_forward_plan = fftwf_plan_many_r2r(rank, &size[0], howmany,
                        _M_in, NULL, howmany, 1,
                        _M_out, NULL, howmany, 1,
                        &fw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW forward plan"; 
		goto plan_fw_fail; 
	}

	if (0 == (_M_backward_plan = fftwf_plan_many_r2r(rank, &size[0], howmany, 
                                                        _M_out, NULL, howmany, 1,
							_M_in,  NULL, howmany, 1, 
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
TCSTPlan<T>::~TCSTPlan()
{
	fftwf_destroy_plan(_M_backward_plan); 
	fftwf_destroy_plan(_M_forward_plan); 
	fftwf_free(_M_out); 
	fftwf_free(_M_in); 

}

template <typename T>
template <typename D>
void TCSTPlan<T>::execute(const D& in_data, D& out_data) const
{
	assert(_M_n == in_data.size()); 
	assert(_M_n == out_data.size()); 

	copy(in_data.begin(), in_data.end(), _M_in); 
	fftwf_execute( _M_forward_plan); 
	do_execute(_M_out); 
	fftwf_execute(_M_backward_plan);
	transform( _M_in,  _M_in + _M_n, out_data.begin(), Scale(_M_scale) ); 
}

NS_MIA_END
