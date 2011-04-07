/* -*- mona-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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

#include <mia/3d/2dimagefifofilter.hh>

NS_BEGIN(label_2dstack_filter)


class C2DLabelFifoFilter: public mia::C2DImageFifoFilter {
public: 
	typedef mia::C2DImage *result_type; 

	C2DLabelFifoFilter(const std::string& mapfile, mia::P2DShape n); 

private: 
	virtual void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x); 
	virtual void do_push(::boost::call_traits<mia::P2DImage>::param_type x); 
	virtual mia::P2DImage do_filter();
	virtual void post_finalize(); 
	void shift_buffer(); 
	
	void label(C2DBitImage& input); 
	void new_label(C2DBitImage& input); 
	void re_label(C2DBitImage& input);
	void grow( int x, int y, C2DBitImage& input, unsigned short l); 

	mia::P2DShape m_neigbourhood; 
	mutable int   m_last_label;
	std::set<T2DVector<unsigned short> > m_joints; 
	std::string   m_map_file;
	bool m_first_pass; 

	mia::C2DBounds m_slice_size; 
	mia::C3DUSImage m_out_buffer;
	
	

};

NS_END

#endif
