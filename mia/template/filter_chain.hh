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

#include <vector>
#include <mia/core/filter.hh>
#include <mia/core/factory.hh>

#ifndef mia_internal_filter_chain_hh
#define mia_internal_filter_chain_hh

NS_MIA_BEGIN

/**
   \ingroup filtering 
   \brief create and use a chain of filters 
   
   This template is used to create and use a chain of filters. 
   \tparam a plug-in handler that is used to create the filters from strings 
   
*/

template <typename Handler> 
class TFilterChain {
	typedef typename Handler::ProductPtr PFilter; 
public: 
	/// the pointer type of the data to be filtered 

	typedef typename PFilter::element_type::plugin_data::Pointer PData; 
	/**
	   The filter chain constrctor 
	   \param filters an array of strings describing the filters 
	   \param nfilters number of parameter strings 
	*/
	TFilterChain(const char *filters[], int nfilters); 


	/**
	   The filter chain constrctor 
	   \param filters a vector of strings describing the filters 
	*/
	TFilterChain(std::vector<std::string> filters);
	
	/**
	   Runs the filter chain. the input data will not be changed. 
	   \param input the input data given as shared pointer 
	   \returns the filtered data as shared pointer 
	*/
	PData run(PData input) const;

	/**
	   Add a filter at the front of the chain
	*/
	void push_front(const char * filter); 

	/**
	   Add a filter at the end of the chain
	*/
	void push_back(const char * filter); 

	/// \returns true if the chain doesn't contain any filters
	bool empty() const; 
private: 
	void init(const char *filters[], int nfilters); 
	std::vector<PFilter> m_chain; 
}; 

/**
   \ingroup filtering 
   \cond INTERNAL 
*/

template <typename Handler>
typename Handler::ProductPtr __get_filter(const Handler& /*h*/, typename Handler::ProductPtr filter) 
{
	return filter; 
}

template <typename Handler>
typename Handler::ProductPtr __get_filter(const Handler& h, const char *filter) 
{
	return h.produce(filter); 
}

template <typename Handler>
typename Handler::ProductPtr __get_filter(const Handler& h, const std::string& filter) 
{
	return h.produce(filter); 
}

template <typename PImage, typename Handler, typename T>
PImage __run_filters(PImage image, const Handler& h, T filter_descr) 
{
	auto f = __get_filter(h, filter_descr); 
	return f->filter(image);
}

template <typename PImage, typename Handler, typename T, typename... Filters>
PImage __run_filters(PImage image, const Handler& h, T filter_descr, Filters ...filters) 
{
	image = __run_filters(image, h, filter_descr);
	return __run_filters(image, h, filters...);
}

/// \endcond 

/**
   \ingroup filtering 
   \brief run a chain of filters on an input image 
   
   This template is used to run a chain of filters on an input image 
   The filters can be described by strings, or given as already created filters
   \tparam PImage the image pointer type of the image to be filtered 
   \tparam Filters the filter description types or filters 
  
*/
template <typename PImage, typename... Filters>
PImage run_filters(PImage image, Filters... filters) 
{
	typedef std::shared_ptr<TImageFilter<typename PImage::element_type> > PFilter; 
	typedef typename FactoryTrait<PFilter>::type Handler;

	return __run_filters(image, Handler::instance(), filters...); 
}

template <typename Handler> 
void TFilterChain<Handler>::init(const char *filters[], int nfilters)
{
	for(int i = 0; i < nfilters; ++i) {
		m_chain[i] = Handler::instance().produce(filters[i]); 
		if (!m_chain[i]) {
			throw create_exception<std::invalid_argument>( "Can't create filter from '", filters[i], "'"); 
		}
	}
}


template <typename Handler> 
TFilterChain<Handler>::TFilterChain(const char *filters[], int nfilters):
	m_chain(nfilters)
{
	init(filters, nfilters); 
}

template <typename Handler> 
TFilterChain<Handler>::TFilterChain(std::vector<std::string> filters):
	m_chain(filters.size())
{
	std::transform(filters.begin(), filters.end(), m_chain.begin(), 
		       [](const std::string& s){ return Handler::instance().produce(s); }); 
}

template <typename Handler> 
void TFilterChain<Handler>::push_front(const char * filter)
{
	auto f = Handler::instance().produce(filter); 
	if (f) 
		m_chain.insert(m_chain.begin(), f); 
	else 
		throw create_exception<std::invalid_argument>( "Can't create filter from '", filter, "'"); 
}

template <typename Handler> 
void TFilterChain<Handler>::push_back(const char * filter)
{
	auto f = Handler::instance().produce(filter); 
	if (f) 
		m_chain.push_back(f); 
	else 
		throw create_exception<std::invalid_argument>( "Can't create filter from '", filter, "'"); 
}

template <typename Handler> 
typename TFilterChain<Handler>::PData 
TFilterChain<Handler>::run(typename TFilterChain<Handler>::PData input) const
{
	for ( auto i = m_chain.begin(); i != m_chain.end(); ++i) {
		input = (*i)->filter(input); 
	}
	return input; 
}

template <typename Handler> 
bool TFilterChain<Handler>::empty() const
{
	return m_chain.empty(); 
}

NS_MIA_END

#endif 
