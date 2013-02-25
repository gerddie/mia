/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#ifndef mia_2d_filter_fifiof_gauss_hh
#define mia_2d_filter_fifiof_gauss_hh

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iomanip>
#include <limits>
#include <numeric>

#include <mia/core/labelmap.hh>
#include <mia/2d/shape.hh>
#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(label_2dstack_filter)


class CLabelRemapper {
public: 
	void clear(); 
	
	void add_pair(unsigned short a, unsigned short b); 
	
	mia::CLabelMap get_map() const; 
private: 
	std::set<mia::T2DVector<unsigned short>, 
		 mia::less_then<mia::T2DVector<unsigned short>>> m_raw_map;
}; 

class C2DLabelStackFilter: public mia::C2DImageFifoFilter {
public: 
	typedef mia::C2DImage *result_type; 


	C2DLabelStackFilter(const std::string& mapfile, mia::P2DShape n); 
	~C2DLabelStackFilter(); 

	// this is public for testing 
	const mia::CLabelMap& get_joints() const; 
private: 
	virtual void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x); 
	virtual void do_push(::boost::call_traits<mia::P2DImage>::param_type x); 
	virtual mia::P2DImage do_filter();
	virtual void post_finalize(); 

	
	void label(mia::C2DBitImage& input); 
	void new_label(mia::C2DBitImage& input); 
	void re_label(mia::C2DBitImage& input);
	void label_new_regions(mia::C2DBitImage& input); 
	void grow( int x, int y, mia::C2DBitImage& input, unsigned short l); 

	mia::P2DShape m_neigbourhood; 
	mutable int   m_last_label;
	CLabelRemapper m_joints; 
	std::string   m_map_file;
	bool m_first_pass; 

	mia::C2DBounds m_slice_size; 
	mia::C2DUSImage m_out_buffer;
	
	mia::CLabelMap m_target; 
	

};

NS_END

#endif
