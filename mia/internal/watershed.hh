
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

#ifndef mia_internal_watershed_hh
#define mia_internal_watershed_hh

#include <mia/core/filter.hh>
#include <queue>

NS_MIA_BEGIN 

template <int dim>
class TWatershed : public watershed_traits<dim>::CFilter {
public:
	typedef typename watershed_traits<dim>::PNeighbourhood  PNeighbourhood; 
	typedef typename watershed_traits<dim>::Position Position; 
	typedef typename watershed_traits<dim>::PFilter PFilter; 
	typedef typename watershed_traits<dim>::CFilter CFilter; 
	typedef typename CFilter::Image CImage; 
	typedef typename watershed_traits<dim>::PImage PImage; 
	typedef typename watershed_traits<dim>::Handler Handler; 


	TWatershed(PNeighbourhood neighborhood, bool with_borders, float treash);

	template <template <typename>  class Image, typename T>
	typename TWatershed<dim>::result_type operator () (const Image<T>& data) const ;
private:
	struct PixelWithLocation {
		Position pos; 
		float value; 
	}; 

	typename TWatershed<dim>::result_type do_filter(const CImage& image) const;

	template <template <typename>  class Image, typename T>
	void grow(const PixelWithLocation& p, Image<unsigned int>& labels, const Image<T>& data) const;

	friend bool operator < (const PixelWithLocation& lhs, const PixelWithLocation& rhs) {
		mia::less_then<Position> l; 
		return lhs.value > rhs.value|| 
			( lhs.value ==  rhs.value && l(rhs.pos, lhs.pos)); 
	}

	std::vector<Position> m_neighborhood; 
	PFilter m_togradnorm; 
	bool m_with_borders; 
	float m_thresh;
};


template <int dim>
class TWatershedFilterPlugin: public watershed_traits<2>::CPlugin {
public:
	TWatershedFilterPlugin();
private:
	virtual typename watershed_traits<2>::CPlugin::Product *do_create()const;
	virtual const std::string do_get_descr()const;
	typename watershed_traits<2>::PNeighbourhood m_neighborhood; 
	bool m_with_borders; 
	float m_thresh; 
};


template <int dim>
TWatershed<dim>::TWatershed(PNeighbourhood neighborhood, bool with_borders, float thresh):
	m_with_borders(with_borders), 
	m_thresh(thresh)
	
{
	m_neighborhood.reserve(neighborhood->size() - 1);
	for (auto i = neighborhood->begin(); i != neighborhood->end();++i) 
		if ( *i != Position::_0 ) 
			m_neighborhood.push_back(*i); 
		
	m_togradnorm = Handler::instance().produce("gradnorm"); 
}

template <int dim>
template <template <typename>  class Image, typename T>
void TWatershed<dim>::grow(const PixelWithLocation& p, Image<unsigned int>& labels, const Image<T>& data) const 
{
	const auto size = data.get_size(); 
	std::vector<Position> backtrack; 
	std::priority_queue<Position, std::vector<Position>, mia::less_then<Position> > locations; 
	locations.push(p.pos); 
	backtrack.push_back(p.pos); 
	bool backtracked = false; 

	float value = p.value; 
	unsigned int label = labels(p.pos); 
	while (!locations.empty()) {
		auto loc = locations.top(); 
		locations.pop(); 
		
		for (auto i = m_neighborhood.begin(); i != m_neighborhood.end(); ++i) {
			Position new_pos( loc + *i);
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
								 [&label, &labels]( const Position& l) {labels(l) = label;}); 
							if (backtracked) 
								mia::cvwarn() << "Backtracking the second time\n"; 
							backtracked = true; 
						}
					}
				}
			}
		}
	}
}

template <int dim>
template <template <typename>  class Image, typename T>
typename TWatershed<dim>::result_type TWatershed<dim>::operator () (const Image<T>& data) const 
{
	auto sizeND =  data.get_size(); 
	Image<unsigned int> labels(data.get_size()); 

	std::priority_queue<PixelWithLocation> pixels; 
	PixelWithLocation p; 
	auto i = data.begin_range(Position::_0, data.get_size());
	auto e = data.end_range(Position::_0, data.get_size());
	
	while (i != e) {
		p.pos = i.pos(); 
		p.value = *i > m_thresh ? *i : m_thresh; 
		pixels.push(p); 
		++i; 
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
			Position new_pos( pixel.pos + *i);
			if (new_pos < sizeND) {
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
	CImage *r = NULL; 
	if (next_label < 255) {
		Image<unsigned char> *result = new Image<unsigned char>(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? static_cast<unsigned char>(p) : 255; }); 
		r = result; 
	}else if (next_label < std::numeric_limits<unsigned short>::max()) {
		Image<unsigned short> *result = new Image<unsigned short>(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? static_cast<unsigned short>(p) : std::numeric_limits<unsigned short>::max(); });
		r = result; 
	}else {
		Image<unsigned int> * result = new Image<unsigned int>(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return p ? p : std::numeric_limits<unsigned int>::max(); });
		r = result; 
	}
	return PImage(r); 
	
}

template <int dim>
typename TWatershed<dim>::result_type TWatershed<dim>::do_filter(const CImage& image) const
{
	auto grad = m_togradnorm->filter(image); 
	return mia::filter(*this, *grad);
}

template <int dim>
TWatershedFilterPlugin<dim>::TWatershedFilterPlugin(): 
	watershed_traits<dim>::CPlugin("ws"), 
	m_with_borders(false), 
	m_thresh(0.0)
{
	this->add_parameter("n", make_param(m_neighborhood, "8n", false, "Neighborhood for watershead region growing")); 
	this->add_parameter("mark", new mia::CBoolParameter(m_with_borders, false, "Mark the segmented watersheds with a special gray scale value")); 
	this->add_parameter("thresh", new mia::CFloatParameter(m_thresh, 0, 1.0, false, "Gradient norm threshold. Bassins seperated by gradients "
						    "with a lower norm will be joined"));  
}

template <int dim>
typename watershed_traits<2>::CPlugin::Product *
TWatershedFilterPlugin<dim>::do_create() const
{
	return new TWatershed<dim>(m_neighborhood, m_with_borders, m_thresh);
}

template <int dim>
const std::string TWatershedFilterPlugin<dim>::do_get_descr()const
{
	return "basic watershead segmentation.";
}

NS_MIA_END

#endif
