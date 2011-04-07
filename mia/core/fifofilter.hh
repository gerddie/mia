/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#ifndef __mia_fifofilter_hh
#define __mia_fifofilter_hh

#include <vector>
#include <memory>
#include <cstdlib>
#include <boost/call_traits.hpp>
#include <mia/core/shared_ptr.hh>

#include <mia/core/msgstream.hh>

#ifndef EXPORT_HANDLER 
#ifdef WIN32
#define EXPORT_HANDLER __declspec(dllimport)
#else
#define EXPORT_HANDLER 
#endif
#endif


NS_MIA_BEGIN
using namespace std; 

/**
  Base class for a First-in-first out filter that does not need 
  the whole data to be loaded. 	
*/

template <typename T> 
class EXPORT_HANDLER TFifoFilter  {
public: 
	
	/// smart pointer representing this class 
	typedef std::shared_ptr<TFifoFilter > Pointer; 
	
	/**
	  Constructor 
	  \param width filter width 
	  \param min_fill minimum fill of the buffer to process (usally half
                filter width)
	  \param read_start number of slices need to be pushed into the pipeline
                before the first results can be read from the filter 
	*/
	TFifoFilter(size_t width, size_t min_fill, size_t read_start); 

	/**
	  Push a data element down the filter pipeline
	  \param x data element 
	*/
	void push(typename ::boost::call_traits<T>::param_type x); 

	/**
	  Initiate the processing of the final slices in the pipeline
	*/
	void finalize(); 

	/**
	  Attach a filter at the end of the filter chain 
	*/
	void append_filter(Pointer last);
protected: 
	/// \returns the current buffer fill 
	size_t get_pos() const; 

	/// \returns the size of the buffer 
	size_t get_buffer_size() const; 
	
	/// \returns the start slide for filtering 
	size_t get_start() const; 
	
	/// \returns the start slide for filtering 
	size_t get_end() const; 

private:
	/// \returns next filter in the chain 		
	Pointer next() const; 

	/**
	  initialize the filter, should to be overwritten  
	  \param x prototype of the data to be processed
	*/
	virtual void do_initialize(typename ::boost::call_traits<T>::param_type x); 

	/**
	  Pure abstract method prototyping the pushing of a data element down the
          filter pipeline. This method must be implemented in derived classes 
	  \param x data element to be processed
	*/
	virtual void do_push(typename ::boost::call_traits<T>::param_type x) = 0; 
	
	/**
	  Do the second stage of filtering producing the filter output. 
	  \returns output data element of the filter 
	*/
	virtual T do_filter();
	
	/**
	  Provides an interface to cleanup the filter after all slices are processed. 
	*/
	virtual void post_finalize(); 

	/**
	  Provides the interface to shift tha data in a holding buffer during 
	  processing.  
	*/
	virtual void shift_buffer(); 

	/**
	  Provides the interface to initiate special evaluation of a data element
          within the holding buffer. 
	  \param slice number of the slice to be processed 
	*/
	virtual void evaluate(size_t slice); 


	size_t m_buf_size;
	size_t m_min_fill;
	size_t m_read_start; 
	size_t m_fill; 
	size_t m_start_slice; 
	size_t m_end_slice; 
	Pointer m_chain; 
	bool m_initialized; 
}; 

/**
  Generic data sink as end of the filter chain that is used for testing purpouses. 
*/
template <typename T> 
class EXPORT_HANDLER TFifoFilterSink : public TFifoFilter<T> {
public: 
	/// The result of the processing  
	typedef std::vector<T>               result_type; 

	/// smart pointer representing this class 
	typedef std::shared_ptr< TFifoFilterSink<T>  >  Pointer;

	/**
	 contructor, its implementation is required because  TFifoFilter doesn't 
	 provide a standard constructor
 	*/ 
  
	TFifoFilterSink(); 
	
	/// \returns the vector of result values
	const result_type& result();
private: 
	virtual void do_push(typename ::boost::call_traits<T>::param_type x); 
	void shift_buffer(); 
	result_type m_result; 
}; 


template <typename T> 
struct __copy_create_ptr {
	static std::shared_ptr<T > apply (typename ::boost::call_traits<T>::param_type x){
		return std::shared_ptr<T >(new T(x)); 
	}
}; 

NS_MIA_END

#endif
