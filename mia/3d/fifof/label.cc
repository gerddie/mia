/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <limits>
#include <queue>
#include <ostream>
#include <fstream>
#include <set>
#include <map>
#include <ctime>
#include <mia/3d/fifof/label.hh>


NS_BEGIN(label_2dstack_filter)

NS_MIA_USE; 
using namespace std; 

C2DLabelStackFilter::C2DLabelStackFilter(const string& mapfile, P2DShape n):
	C2DImageFifoFilter(1, 1, 0), 
	m_neigbourhood(n), 
	m_last_label(1), 
	m_map_file(mapfile), 
	m_first_pass(true)
{
}
			   
C2DLabelStackFilter::~C2DLabelStackFilter()
{
}

static int slice = 0; 
void C2DLabelStackFilter::do_initialize(::boost::call_traits<mia::P2DImage>::param_type x)
{
	m_first_pass = true; 
	m_joints.clear(); 
	m_out_buffer = C2DUSImage(x->get_size()); 
	m_last_label = 1; 
}

void C2DLabelStackFilter::grow( int x, int y, C2DBitImage& input, unsigned short l)
{
	vector<C2DBounds> seed; 
	seed.push_back(C2DBounds(x,y)); 
	
	while (!seed.empty()) {
		C2DBounds loc = seed.back(); 
		seed.pop_back(); 
		for (auto si = m_neigbourhood->begin(); 
		     si != m_neigbourhood->end(); ++si) {
			size_t px = si->x + loc.x; 
			size_t py = si->y + loc.y; 
			if (px >= input.get_size().x ||
			    py >= input.get_size().y) 
				continue; 
			
			unsigned short lold = m_out_buffer(px, py); 
			if (lold) {
				if (l != lold) {
					m_joints.add_pair(l, lold); 
					m_out_buffer(px, py) = l; 
				}
			}else if (input(px, py)) {
				m_out_buffer(px, py) = l; 
				input(px, py) = false; 
				seed.push_back(C2DBounds(px,py)); 
			}
		}
	}
}

void C2DLabelStackFilter::label_new_regions(C2DBitImage& input)
{
	auto ii = input.begin();
	auto usi = m_out_buffer.begin(); 
	
	for (size_t y = 0; y < input.get_size().y; ++y) 
		for (size_t x = 0; x < input.get_size().x; ++x, ++usi, ++ii) {
			if (*ii) {
				cvdebug() << "("<< x << ", " << y <<"," << slice <<  "):" 
					  << m_last_label << "\n"; 
				if (m_last_label < numeric_limits<unsigned short>::max()) 
					*usi = m_last_label++;
				else 
					throw create_exception<invalid_argument>("C2DLabelStackFilter: number of connected components is about to "
								       "exeed the  supported limit of ",
								       numeric_limits<unsigned short>::max(), 
								       ", sorry can't continue\n");
				*ii = false; 
				grow(x,y,input,*usi); 
			}
		}
}

void C2DLabelStackFilter::label(C2DBitImage& input)
{
	// first grow all regions that are already labeled from the last slice
	auto usi = m_out_buffer.begin(); 
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
	m_out_buffer = C2DUSImage(input.get_size(), input); 
	label_new_regions(input); 
}

void  C2DLabelStackFilter::re_label(C2DBitImage& input)
{
	
	auto usi = m_out_buffer.begin(); 
	auto use = m_out_buffer.end(); 
	auto ii = input.begin(); 

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
	
	const C2DBitImage *inp = dynamic_cast<const C2DBitImage*>(x.get()); 
	if (!inp) 
		throw invalid_argument("C2DLabelStackFilter: input images must be binary"); 
	
	C2DBitImage input(*inp); 
	if (m_first_pass) {
		new_label(input); 
		m_first_pass = false; 
	}else {
		re_label(input);  
	}
	++slice; 

}

void CLabelRemapper::clear()
{
	m_raw_map.clear(); 
}

P2DImage C2DLabelStackFilter::do_filter()
{
	return P2DImage(new C2DUSImage(m_out_buffer)); 
}

void CLabelRemapper::add_pair(unsigned short a, unsigned short b)
{
	if (a > b) 
		m_raw_map.insert(T2DVector<unsigned short>(a,b));
	else 
		m_raw_map.insert(T2DVector<unsigned short>(b,a)); 
}

struct greater_than {
	typedef T2DVector<unsigned short> value_type; 
	bool operator() (const T2DVector<unsigned short>& lhs, 
			 const T2DVector<unsigned short>& rhs) {
		return lhs.x > rhs.x || ((lhs.x  == rhs.x)  && lhs.y > rhs.y); 
	}
};

CLabelMap CLabelRemapper::get_map() const
{
	CLabelMap result; 
	priority_queue<T2DVector<unsigned short>, vector<T2DVector<unsigned short>>, greater_than> sorted; 
	cvdebug()<< "got " << m_raw_map.size() << " joints\n"; 

	for (auto i = m_raw_map.begin();  i != m_raw_map.end();  ++i)
		sorted.push(*i); 
	
	
	

	while (!sorted.empty()) {
		auto v = sorted.top(); 
		cvdebug() << "Top = " << v << "\n"; 
		sorted.pop();
		
		// first check, if the value we map to is already mapped to a lower number
		// if yes, take this mapping for the new value
		auto m = result.find(v.y);
		if (m != result.end()) {
			result[v.x] = m->second; 
		} else {
			// now test of the value to be mapped is already available 
			// and if not add the new mapping, if yes, add a mapping for the target 
			m = result.find(v.x);
			if (m == result.end()) {
				result[v.x] = v.y; 
			} else {
				result[v.y] = m->second; 
			}
		}
	}
	return result; 
}


void C2DLabelStackFilter::post_finalize()
{
	m_target = m_joints.get_map(); 
	
	if (!m_map_file.empty()) {
		ofstream outfile(m_map_file.c_str(), ios_base::out );
		m_target.save(outfile); 
		if (!outfile.good()) {
			throw create_exception<runtime_error>("C2DLabelStackFilter: failed to save labale join map to '", 
						    m_map_file, "'"); 
		}
	}
}

const CLabelMap& C2DLabelStackFilter::get_joints() const
{
	return m_target; 
}


class C2DLabelFifoFilterPlugin : public C2DFifoFilterPlugin {
public:
	C2DLabelFifoFilterPlugin();
private:

	virtual const string do_get_descr() const;
	virtual C2DImageFifoFilter *do_create()const;

	mia::P2DShape m_neighbourhood; 
	string m_mapfile;
};

C2DLabelFifoFilterPlugin::C2DLabelFifoFilterPlugin():
	C2DFifoFilterPlugin("label")
{
	add_parameter("n", make_param(m_neighbourhood, "4n", false, 
				      "2D neighbourhood shape to define connectedness"));
	add_parameter("map", new CStringParameter(m_mapfile, CCmdOptionFlags::required_input, 
						  "Mapfile to save label numbers that are joined"));
}

const string C2DLabelFifoFilterPlugin::do_get_descr() const
{
	return "Stack Label filter";
}

C2DImageFifoFilter *C2DLabelFifoFilterPlugin::do_create()const
{
	return new C2DLabelStackFilter(m_mapfile, m_neighbourhood);
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{

	return new C2DLabelFifoFilterPlugin();
}


NS_END
