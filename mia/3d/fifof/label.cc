/* -*- mona-c++  -*-
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#include <limits>
#include <queue>
#include <ostream>
#include <fstream>
#include <set>
#include <map>
#include <ctime>

/* 
   atexBeginPlugin{2D image stack filters}
   
   \subsection{Label}
   \label{fifof:label}
   
   \begin{description}
   
   \item [Plugin:] label
   \item [Description:] Labels connected components in a series of binary images. 
      Since lables may join after preceeding label images are already saved and 
      discarded from the pipeline stage, a map for labels that were joined is 
      created. 
   \item [Input:] Binary images, all of the same size
   \item [Output:] The labeled image(s) 
   
   \plugtabstart
   \plugtabend
   shape & string & shape to define neighbourhood of voxels, only simple neighborhoods are supported & 6n \\ 
   map & string & file name where the label join map will be saved &  - \ \
   first & bool & Set true if this is the first labeling pass
   \end{description}

   atexEnd  
 */

NS_BEGIN(label_2dstack_filter)

NS_MIA_USE; 
using namespace std; 

C2DLabelStackFilter::C2DLabelStackFilter(const string& mapfile, P2DShape n):
	C2DStackFilterBase(1, 1, 1), 
	m_neighborhood(n), 
	m_max_label_num(1), 
	m_map_file(map_file), 
	m_first_pass(true)
{
}
			   
C2DLabelStackFilter::~C2DLabelStackFilter()
{
}


void C2DLabelStackFilter::do_initialize(::boost::call_traits<mia::P2DImage>::param_type x)
{
	m_first_pass = true; 
	m_label_map.clear(); 
	m_out_buffer.clear(); 
	m_last_label = 1; 
}

void C2DLabelStackFilter::grow( int x, int y, C2DBitImage& input, unsigned short l)
{
	vector<C2DBounds> seed; 
	seed.push_back(C2DBounds(x,y)); 
	
	while (!seed.empty()) {
		C2DBounds l = seed.back(); 
		seed.pop_back(); 
		for (C2DShape::const_iterator si = m_shape.begin(); 
		     si != m_shape.end(); ++si) {
			size_t px = si->x + l.x; 
			size_t py = si->y + l.y; 
			if (px >= input.get_size().x ||
			    py >= input.get_size().y) 
				continue; 
			
			unsigned short lold = m_out_buffer(px, py); 
			if (lold) {
				assert( l != lold); 
				if (lold > l) 
					m_joints.insert(T2DVector<unsigned short>(l, lold)); 
				else
					m_joints.insert(T2DVector<unsigned short>(lold, l)); 
				continue; 
			}
			if (input(px, py)) {
				m_out_buffer(px, py) = l; 
				input(px, py) = false; 
				seed.push_back(C2DBounds(px,py)); 
			}
		}
	}
}

void C2DLabelStackFilter::label_new_regions(C2DBitImage& input)
{
	C3DBitImage::iterator ii = input.begin();
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	
	for (size_t y = 0; y < input.get_size().y; ++y) 
		for (size_t x = 0; x < input.get_size().x; ++x, ++usi, ++i) {
			if (*ii) {
				*usi = m_last_label++;
				*ii = false; 
				grow(x,y,input,*usi); 
			}
		}
}

void C2DLabelStackFilter::label(C2DBitImage& input)
{
	// first grow all regions that are already labeled from the last slice
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	for (size_t y = 0; y < input.get_size().y; ++y) 
		for (size_t x = 0; x < input.get_size().x; ++x, ++usi) {
			if ( *usi )
				grow(x,y, input, *usi);
		}
	
	// then label all not yet connected regions 
	label_new_regions(input); 
}

void C2DLabelStackFilter::new_label(C2DBitImage& input)
{
	m_out_buffer = C3DUSImage(input.get_size(), input.get_attribute_list()); 
	label_new_regions(input); 
}

void  C2DLabelStackFilter::re_label(C2DBitImage& input)
{
	
	C3DUSImage::iterator usi = m_out_buffer.begin(); 
	C3DUSImage::iterator use = m_out_buffer.end(); 
	C2DBitImage::iterator ii = input.begin(); 

	// maintain old labeling for new slice, and clean the input at 
	// labeled positions
	while (usi != use)  {
		if (!*ii) 
			*usi = 0; 
		else if (*usi) 
			*ii = 0; 
		++ii; 
		++usi; 
	}

	// now continue normal labeling 
	label(input); 
}

void C2DLabelStackFilter::do_push(::boost::call_traits<mia::P2DImage>::param_type x)
{
	const C2DBitImage *b = dynamic_cast<const C2DBitImage *>(x); 
	if (!b) 
		throw invalid_argument("C2DLabelStackFilter: input image is not binary"); 
	
	C2DBitImage input(*b); 

	if (m_first_pass) {
		m_out_buffer = label(input); 
	}else {
		m_out_buffer = re_label(input);  
	}

}

mia::P2DImage C2DLabelStackFilter::do_filter()
{
	return P2DImage(new C3DUSImage(m_out_buffer)); 
}
void C2DLabelStackFilter::post_finalize()
{
	// removed chained joints in the label map and save it
	typedef map<unsigned short, unsigned short> CMapMap; 
	
	priority_queue<T2DVector<unsigned short> > sorted; 

	for (std::set<T2DVector<unsigned short> >::const_iterator i = 
		     m_joints.begin();  i != m_joints.end();  ++i)
		sorted.push(*i); 

	CMapMap target; 
	
	while (!sorted.empty()) {
		T2DVector<unsigned short> v = sorted.front(); 
		sorted.pop();
		
		CMapMap::const_iterator m = target.find(v.y);
		if (m == target.end())  
			target[v.y] = v.x;
		else {
			target[v.x] = m->y; 
		}
	}
}

NS_END
