/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
	mutable value_type *m_in; 
	mutable value_type *m_out; 

	std::vector<int> m_size; 
	float m_scale; 
	size_t m_n; 

	fftwf_plan m_forward_plan; 
	fftwf_plan m_backward_plan; 

	virtual void do_execute(value_type *buffer) const = 0;
	
	struct Scale {
		Scale(float scale): m_scale(scale){
		}
		value_type operator ()(const value_type& x) const{
			return m_scale * x; 
		}
		float m_scale; 
	}; 
}; 

template <typename T> 
const std::vector<int>& TCSTPlan<T>::get_size() const
{
	return m_size; 
}

template <typename T> 
TCSTPlan<T>::TCSTPlan(fftwf_r2r_kind forward, std::vector<int> size):
	m_size(size),
	m_scale(1.0)
{
	std::string msg; 
	size_t rank = m_size.size(); 
	fftwf_r2r_kind backward; 


	switch (forward) {
	case FFTW_RODFT00: 
		for (size_t i = 0; i < rank; ++i)
			m_scale *= 0.5 / ( size[i] + 1.0); 
		break; 
	case FFTW_REDFT00: 
		for (size_t i = 0; i < rank; ++i)
			m_scale *= 0.5 / ( size[i] - 1.0); 
		break; 
	default: 
		for (size_t i = 0; i < rank; ++i)
			m_scale *= 0.5 / size[i]; 
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
	m_n = 1; 
	for (size_t i = 0; i < rank; ++i) 
		m_n *= m_size[i]; 


	if (NULL == (m_in = (value_type *) fftwf_malloc(sizeof(T) * m_n))) {
		msg = "unable to allocate FFTW data"; 
		goto in_fail; 
	}

	if ( NULL == (m_out  = (value_type *) fftwf_malloc(sizeof(T) * m_n))) {
		msg = "unable to allocate FFTW data"; 
		goto out_fail; 
	}
	
        if (0 == (m_forward_plan = fftwf_plan_many_r2r(rank, &size[0], howmany,
                        m_in, NULL, howmany, 1,
                        m_out, NULL, howmany, 1,
                        &fw_kind[0], FFTW_ESTIMATE))) {
		msg = "unable to create FFTW forward plan"; 
		goto plan_fw_fail; 
	}

	if (0 == (m_backward_plan = fftwf_plan_many_r2r(rank, &size[0], howmany, 
                                                        m_out, NULL, howmany, 1,
							m_in,  NULL, howmany, 1, 
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
TCSTPlan<T>::~TCSTPlan()
{
	fftwf_destroy_plan(m_backward_plan); 
	fftwf_destroy_plan(m_forward_plan); 
	fftwf_free(m_out); 
	fftwf_free(m_in); 

}

template <typename T>
template <typename D>
void TCSTPlan<T>::execute(const D& in_data, D& out_data) const
{
	assert(m_n == in_data.size()); 
	assert(m_n == out_data.size()); 

	copy(in_data.begin(), in_data.end(), m_in); 
	fftwf_execute( m_forward_plan); 
	do_execute(m_out); 
	fftwf_execute(m_backward_plan);
	transform( m_in,  m_in + m_n, out_data.begin(), Scale(m_scale) ); 
}

NS_MIA_END
