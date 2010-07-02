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

#include <vector>
#include <memory>
#include <cstdlib>
#include <boost/call_traits.hpp>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN
using namespace std; 


// implementation 

template <typename T> 
TFifoFilter<T>::TFifoFilter(size_t filter_width, size_t min_fill, size_t read_start):
	_M_buf_size(filter_width + read_start),
	_M_min_fill(min_fill + read_start), 
	_M_read_start(read_start), 
	_M_fill(0), 
	_M_initialized(false)
{
}

template <typename T> 
void TFifoFilter<T>::push(typename ::boost::call_traits<T>::param_type x)
{
	TRACE("TFifoFilter<T>::push"); 

	if (!_M_initialized) {
		do_initialize(x); 
		_M_initialized = true; 
	} else {
		shift_buffer();
	}
		
	do_push(x);
	++_M_fill; 

	if (_M_fill > _M_read_start)  
		evaluate(_M_read_start); 


	if (_M_fill >= _M_min_fill) {
		_M_start_slice = _M_read_start; 
		_M_end_slice = _M_fill; 

		T help = do_filter(); 
		if (_M_chain) 
			_M_chain->push(help); 
	}
	
	if (_M_fill < _M_buf_size) {
		return; 
	}
	--_M_fill; 
}
template <typename T> 
size_t TFifoFilter<T>::get_buffer_size() const
{
	return _M_buf_size; 
}

template <typename T> 
size_t TFifoFilter<T>::get_start() const
{
	return _M_start_slice; 
}

template <typename T> 
size_t TFifoFilter<T>::get_end() const
{
	return _M_end_slice; 
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
	return _M_fill; 
}

template <typename T> 
void TFifoFilter<T>::finalize()
{
	TRACE("TFifoFilter<T>::finalize()"); 
	size_t overfill = _M_read_start; 

	while (overfill-- > 0) {
		shift_buffer(); 
		evaluate(_M_read_start); 
		_M_start_slice = _M_read_start; 
		_M_end_slice = _M_buf_size; 
		T help = do_filter(); 
		
		if (_M_chain) 
			_M_chain->push(help); 
	}

	// it makes the test run through, but I'm not sure why 
	size_t start = _M_read_start + 1; 
	
	while (_M_fill >= _M_min_fill) {
		shift_buffer(); 
		
		_M_start_slice = start; 
		_M_end_slice = _M_buf_size; 

		T help = do_filter(); 

		if (_M_chain) 
			_M_chain->push(help); 
		--_M_fill;
		++start; 
	}

	post_finalize(); 
	_M_initialized = false; 

	if (_M_chain)
		_M_chain->finalize(); 

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

	if (!_M_chain) 
		_M_chain = last; 
	else {
		Pointer n = _M_chain; 
		while (n->next())
			n = n->next(); 
		_M_chain->append_filter(last);
	}
}

template <typename T> 
typename TFifoFilter<T>::Pointer TFifoFilter<T>::next() const 
{
	return _M_chain; 
}

template <typename T> 
TFifoFilterSink<T>::TFifoFilterSink():
	TFifoFilter<T>(0,1,0)
{
}
	
template <typename T> 
const typename TFifoFilterSink<T>::result_type& TFifoFilterSink<T>::result()
{
	return _M_result; 
}


template <typename T> 
void TFifoFilterSink<T>::do_push(typename ::boost::call_traits<T>::param_type x)
{
	TRACE("TFifoFilterSink<T>::do_push()"); 
	_M_result.push_back(x); 
}

template <typename T> 
void TFifoFilterSink<T>::shift_buffer()
{
	TRACE("TFifoFilterSink<T>::shift_buffer()\n"); 
}


NS_MIA_END
