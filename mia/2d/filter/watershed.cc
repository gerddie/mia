/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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

#include <type_traits> 
#include <queue> 
#include <mia/2d/filter/watershed.hh>

NS_BEGIN( ws_2dimage_filter)
using namespace mia; 
using namespace std; 



C2DWatershed::C2DWatershed(P2DShape neighborhood, bool with_borders, float thresh):
	m_with_borders(with_borders), 
	m_thresh(thresh)
	
{
	m_neighborhood.reserve(neighborhood->size() - 1);
	for (auto i = neighborhood->begin(); i != neighborhood->end();++i) 
		if ( *i != C2DBounds::_0 ) 
			m_neighborhood.push_back(*i); 
		
	m_togradnorm = produce_2dimage_filter("gradnorm"); 
}

template <class T>
void C2DWatershed::grow(const PixelWithLocation& p, C2DUIImage& labels, const T2DImage<T>& data) const 
{
	const auto size = data.get_size(); 
	vector<C2DBounds> backtrack; 
	priority_queue<C2DBounds> locations; 
	locations.push(p.pos); 
	backtrack.push_back(p.pos); 
	bool backtracked = false; 

	float value = p.value; 
	unsigned int label = labels(p.pos); 
	while (!locations.empty()) {
		auto loc = locations.top(); 
		locations.pop(); 
		
		for (auto i = m_neighborhood.begin(); i != m_neighborhood.end(); ++i) {
			C2DBounds new_pos( loc.x + i->x, loc.y + i->y);
			if (new_pos < size) {
				// grow only for the same or lower gradient magnitudes, 
				if (data(new_pos) <= value) {
					if (!labels(new_pos)) {
						labels(new_pos) = label; 
						locations.push(new_pos); 
						backtrack.push_back(new_pos);
					}else {
						if (labels(new_pos) != label) {
							/* in a perfect world this wouldn't happen, since all the 
							   seeds are set only for the lowest gradient value. However, in the discrete 
							   space it might happen that we hit already labeled pixles. 
							   Then we should use the backtrack to reset all already set labels,
							   and continue growing with the new label. 
							   Unfortunately this may leave some holes in the label numbering. 
							*/
							label = labels(new_pos); 
							for_each(backtrack.begin(), backtrack.end(), 
								 [&label, &labels]( const C2DBounds l) {labels(l) = label;}); 
							if (backtracked) 
								cvwarn() << "Backtracking the second time\n"; 
							backtracked = true; 
						}
					}
				}
			}
		}
	}
}

template <class T>
typename C2DWatershed::result_type C2DWatershed::operator () (const T2DImage<T>& data) const 
{
	C2DUIImage labels(data.get_size()); 

	priority_queue<PixelWithLocation> pixels; 
	PixelWithLocation p; 
	auto i = data.begin();
	const auto size = data.get_size(); 
	
	// read and sort all the pixels
	for(p.pos.y = 0; p.pos.y < size.y; ++p.pos.y )
		for(p.pos.x = 0; p.pos.x < size.x; ++p.pos.x, ++i ) {
			p.value = *i > m_thresh ? *i : m_thresh; 
			pixels.push(p); 
		}
	
	long next_label = 1; 
	while (!pixels.empty()) {
		auto pixel = pixels.top(); 
		pixels.pop(); 
		// this label was set because we grew an initial region
		if (labels(pixel.pos)) {
			continue; 
		}

		unsigned int first_label = 0; 
		bool is_boundary = false; 
		// check if neighborhood is already labeled
		
		for (auto i = m_neighborhood.begin(); i != m_neighborhood.end() && !is_boundary; ++i) {
			C2DBounds new_pos( pixel.pos.x + i->x, pixel.pos.y + i->y);
			if (new_pos < size) {
				auto l = labels(new_pos); 
				if ( l ) {
					if (!first_label)
						first_label = l; 
					else 
						if (first_label != l) 
							is_boundary = true; 
				}
			}
		}
		if (first_label) {
			if (!is_boundary)
				labels(pixel.pos) = first_label; 
			continue; 
		}
		// new label to assign
		// if a new label is assigned, we have to grow the region of equal gradient values 
		// to assure we catch the whole bassin 
		labels(pixel.pos) = next_label++; 
		grow(pixel, labels, data); 
	}
	// convert to smalles possible intensity range and convert the boundary label to highest 
	// intensity value
	C2DImage *r = NULL; 
	if (next_label < 255) {
		C2DUBImage *result = new C2DUBImage(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? static_cast<unsigned char>(p) : 255; }); 
		r = result; 
	}else if (next_label < numeric_limits<unsigned short>::max()) {
		C2DUSImage *result = new C2DUSImage(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? static_cast<unsigned short>(p) : numeric_limits<unsigned short>::max(); });
		r = result; 
	}else {
		C2DUIImage * result = new C2DUIImage(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? p : numeric_limits<unsigned int>::max(); });
		r = result; 
	}
	return P2DImage(r); 
	
}


P2DImage C2DWatershed::do_filter(const C2DImage& image) const
{
	P2DImage grad = m_togradnorm->filter(image); 
	return mia::filter(*this, *grad);
}

C2DWatershedFilterPlugin::C2DWatershedFilterPlugin(): 
	C2DFilterPlugin("ws"), 
	m_with_borders(false), 
	m_thresh(0.0)
{
	add_parameter("n", make_param(m_neighborhood, "8n", false, "Neighborhood for watershead region growing")); 
	add_parameter("mark", new CBoolParameter(m_with_borders, false, "Mark the segmented watersheds with a special gray scale value")); 
	add_parameter("thresh", new CFloatParameter(m_thresh, 0, 1.0, false, "Gradient norm threshold. Bassins seperated by gradients "
						    "with a lower norm will be joined"));  
}

C2DFilter *C2DWatershedFilterPlugin::do_create() const
{
	return new C2DWatershed(m_neighborhood, m_with_borders, m_thresh);
}

const string C2DWatershedFilterPlugin::do_get_descr()const
{
	return "basic 2D watershead segmentation.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DWatershedFilterPlugin();
}


NS_END
