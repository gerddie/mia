/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <vector>
#include <memory>
#include <cstdlib>
#include <boost/call_traits.hpp>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN


// implementation 

template <typename T> 
TFifoFilter<T>::TFifoFilter(size_t filter_width, size_t min_fill, size_t read_start):
	m_buf_size(filter_width + read_start),
	m_min_fill(min_fill + read_start), 
	m_read_start(read_start), 
	m_fill(0), 
	m_start_slice(0), 
	m_end_slice(0), 
	m_initialized(false)
{
}

template <typename T> 
void TFifoFilter<T>::push(typename ::boost::call_traits<T>::param_type x)
{
	TRACE_FUNCTION; 

	if (!m_initialized) {
		do_initialize(x); 
		m_initialized = true; 
	} else {
		shift_buffer();
	}
		
	do_push(x);
	++m_fill; 

	cvdebug() << "push: fill : " << m_fill << " ,need "<< m_min_fill <<", max="<< m_buf_size<<"\n"; 

	if (m_fill > m_read_start)  
		evaluate(m_read_start); 

	if (m_fill >= m_min_fill) {
		m_start_slice = m_read_start; 
		m_end_slice = m_fill; 

		cvdebug() << "do_filter: slices : [" << m_start_slice << ", "<< m_end_slice 
			  <<"] fill " << m_fill << "\n"; 
		
		T help = do_filter(); 
		if (m_chain) 
			m_chain->push(help); 
	}
		
	
	if (m_fill < m_buf_size) {
		return; 
	}
	--m_fill; 
}
template <typename T> 
size_t TFifoFilter<T>::get_buffer_size() const
{
	return m_buf_size; 
}

template <typename T> 
size_t TFifoFilter<T>::get_start() const
{
	return m_start_slice; 
}

template <typename T> 
size_t TFifoFilter<T>::get_end() const
{
	return m_end_slice; 
}


template <typename T> 
void TFifoFilter<T>::evaluate(size_t /*slice*/)
{
}

template <typename T> 
T TFifoFilter<T>::do_filter()
{
	return T(); 
}

template <typename T> 
size_t TFifoFilter<T>::get_pos() const
{
	return m_fill; 
}

template <typename T> 
void TFifoFilter<T>::finalize()
{
	TRACE_FUNCTION; 
	size_t overfill = m_read_start; 
	// prepare all remaining slices 
	for (size_t i = 0; i < m_read_start; i++) 
		evaluate(i);

	while (overfill-- > 0) {
		shift_buffer(); 
		if (m_fill < m_buf_size) 
			++m_fill; 

		m_start_slice = m_read_start; 
		m_end_slice = m_fill; 

		cvdebug() << "do_filter (finalize 1): slices : [" << m_start_slice << ", "<< m_end_slice 
			  <<"] fill " << m_fill << "\n"; 

		T help = do_filter(); 
				
		if (m_chain) 
			m_chain->push(help); 

	}
	
	if (m_read_start > 0) 
		--m_fill;

	cvdebug() << "finalize: fill=" << m_fill << ", min-fill=" << m_min_fill << "\n"; 

	m_end_slice = m_fill + 1; 

	while (m_fill >= m_min_fill && m_fill) {

		shift_buffer(); 
		if (m_end_slice < m_buf_size) 
			++m_end_slice; 

		m_start_slice = m_end_slice - m_fill + m_read_start; 
		cvdebug() << "do_filter (finalize 2): slices : [" << m_start_slice << ", "<< m_end_slice 
			  <<"] fill " << m_fill << "\n"; 
		T help = do_filter(); 

		if (m_chain) 
			m_chain->push(help); 
		--m_fill;
	}

	post_finalize(); 
	m_initialized = false; 

	if (m_chain)
		m_chain->finalize(); 

}

template <typename T> 
void TFifoFilter<T>::do_initialize(typename ::boost::call_traits<T>::param_type /*x*/)
{
}

template <typename T> 
void TFifoFilter<T>::post_finalize()
{
}

template <typename T> 
void TFifoFilter<T>::shift_buffer()
{
	TRACE("TFifoFilter<T>::shift_buffer()\n"); 
}

template <typename T> 
void TFifoFilter<T>::append_filter(typename TFifoFilter<T>::Pointer last)
{
	TRACE("TFifoFilter<T>::append_filter"); 

	if (!m_chain) 
		m_chain = last; 
	else {
		Pointer n = m_chain; 
		while (n->next())
			n = n->next(); 
		m_chain->append_filter(last);
	}
}

template <typename T> 
typename TFifoFilter<T>::Pointer TFifoFilter<T>::next() const 
{
	return m_chain; 
}

template <typename T> 
TFifoFilterSink<T>::TFifoFilterSink():
	TFifoFilter<T>(0,1,0)
{
}
	
template <typename T> 
const typename TFifoFilterSink<T>::result_type& TFifoFilterSink<T>::result()
{
	return m_result; 
}


template <typename T> 
void TFifoFilterSink<T>::do_push(typename ::boost::call_traits<T>::param_type x)
{
	m_result.push_back(x); 
}

template <typename T> 
void TFifoFilterSink<T>::shift_buffer()
{
	TRACE("TFifoFilterSink<T>::shift_buffer()\n"); 
}


NS_MIA_END
