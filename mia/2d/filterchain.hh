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

#ifndef mia_2d_filterchain_hh
#define mia_2d_filterchain_hh

#include <mia/2d/2dfilter.hh>
#include <list>

NS_MIA_BEGIN


/**
   \brief a convenience class to create a chain of 2D image filters 

   This class is used to construct a filter chain from a vector of 
   parameter strings. 
   
*/

class EXPORT_2D C2DFilterChain {
public:
	C2DFilterChain();

	/**
	   Construct the filter chain by using a list of strings 
	   that describe the filters to be chained up. 
	   @param descr 
	*/
	C2DFilterChain(const vector<const char *>& descr);
	
	/**
	   Adds a filter at the begin of the chain. 
	   @param filter 
	 */
	void push_front(P2DFilter filter);

	/**
	   Adds a filter at the end of the chain. 
	   @param filter 
	 */
	void push_back(P2DFilter filter);

	/**
	   run the filter chain on an image
	   @param image input image 
	   @returns filtered image 
	 */
	P2DImage filter(const C2DImage& image) const;

	/// @returns true if the filter chain is empty, false otherwise 
	bool empty() const;
private:
	std::list<P2DFilter> m_chain;
};

NS_MIA_END

#endif
