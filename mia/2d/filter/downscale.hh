/* -*- mia-c++  -*-
 * Copyright (c) 2004 
 * Max-Planck-Institute for Evolutionary Anthropoloy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/*! \brief basic type of a plugin handler 

A bandpass filter 

\file bandpass_3dimage_filter.cc
\author Gert Wollny <wollny at eva.mpg.de>

*/

#include <limits>
#include <sstream>
#include <mia/2d/2dfilter.hh>

NS_BEGIN(downscale_2dimage_filter)

class CDownscale: public mia::C2DFilter {
public: 	
	CDownscale(const mia::C2DBounds& block_size, const std::string& filter); 
	
	~CDownscale(); 

	template <typename  T>
	CDownscale::result_type operator () (const mia::T2DImage<T>& data) const;

private: 
	CDownscale::result_type do_filter(const mia::C2DImage& image) const;
	
 	mia::C2DFilterPlugin::ProductPtr _M_smoothing; 
	const mia::C2DBounds _M_block_size;

	
};

class C2DDownscaleFilterPlugin: public mia::C2DFilterPlugin {
public: 
	C2DDownscaleFilterPlugin();
	virtual mia::C2DFilterPlugin::ProductPtr do_create()const;
	virtual const std::string do_get_descr()const; 
private: 
	int _M_bsx;
	int _M_bsy; 
	std::string _M_filter;
};

NS_END
