
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
	typedef typename PNeighbourhood::element_type::value_type MPosition; 
	typedef typename watershed_traits<dim>::Position Position; 
	typedef typename watershed_traits<dim>::PFilter PFilter; 
	typedef typename watershed_traits<dim>::CFilter CFilter; 
	typedef typename CFilter::Image CImage; 
	typedef typename watershed_traits<dim>::PImage PImage; 
	typedef typename watershed_traits<dim>::Handler Handler; 


	TWatershed(PNeighbourhood neighborhood, bool with_borders, float treash, bool eval_grad);

	template <template <typename>  class Image, typename T>
	typename TWatershed<dim>::result_type operator () (const Image<T>& data) const ;
private:
	struct PixelWithLocation {
		Position pos; 
		float value; 
	}; 

	typename TWatershed<dim>::result_type do_filter(const CImage& image) const;

	template <template <typename>  class Image, typename T>
	bool grow(const PixelWithLocation& p, Image<unsigned int>& labels, const Image<T>& data) const;

	friend bool operator < (const PixelWithLocation& lhs, const PixelWithLocation& rhs) {
		mia::less_then<Position> l; 
		return lhs.value > rhs.value|| 
			( lhs.value ==  rhs.value && l(rhs.pos, lhs.pos)); 
	}

	std::vector<MPosition> m_neighborhood; 
	PFilter m_togradnorm; 
	bool m_with_borders; 
	float m_thresh;
};


template <int dim>
class TWatershedFilterPlugin: public watershed_traits<dim>::CPlugin {
public:
	TWatershedFilterPlugin();
private:
	virtual typename watershed_traits<dim>::CPlugin::Product *do_create()const;
	virtual const std::string do_get_descr()const;
	typename watershed_traits<dim>::PNeighbourhood m_neighborhood; 
	bool m_with_borders; 
	float m_thresh; 
	bool m_eval_grad; 
};


template <int dim>
TWatershed<dim>::TWatershed(PNeighbourhood neighborhood, bool with_borders, float thresh, bool eval_grad):
	m_with_borders(with_borders), 
	m_thresh(thresh)
	
{
	m_neighborhood.reserve(neighborhood->size() - 1);
	for (auto i = neighborhood->begin(); i != neighborhood->end();++i) 
		if ( *i != MPosition::_0 ) 
			m_neighborhood.push_back(*i); 

	if (eval_grad) 
		m_togradnorm = Handler::instance().produce("gradnorm"); 
}

static const unsigned int boundary_label = numeric_limits<unsigned int>::max(); 

template <int dim>
template <template <typename>  class Image, typename T>
bool TWatershed<dim>::grow(const PixelWithLocation& p, Image<unsigned int>& labels, const Image<T>& data) const 
{
	const auto size = data.get_size(); 
	std::vector<Position> backtrack; 
	std::priority_queue<Position, std::vector<Position>, mia::less_then<Position> > locations; 
	bool has_backtracked = false; 

	backtrack.push_back(p.pos); 
	
	std::vector<Position> new_positions; 
	new_positions.reserve(m_neighborhood.size()); 
	
	cvdebug()<< "grow " << p.pos << "\n"; 
	float value = p.value; 
	unsigned int label = labels(p.pos); 

	for (auto i = m_neighborhood.begin(); i != m_neighborhood.end(); ++i) {
		Position new_pos( p.pos + *i);
		if (new_pos < size && !labels(new_pos) && data(new_pos) <= value) {
			locations.push(new_pos); 
			backtrack.push_back(new_pos); 
		}
	}

	while (!locations.empty()) {
		// incoming locations are always un-labelled, and the gradient value is in the equal or below the target value
		auto loc = locations.top(); 
		locations.pop(); 
		
		new_positions.clear(); 

		cvdebug()<< "**extend*************" << loc << "*******************\n"; 
		unsigned int first_label = 0; 
		bool loc_is_boundary = false; 

		for (auto i = m_neighborhood.begin(); i != m_neighborhood.end() && !loc_is_boundary; ++i) {
			Position new_pos( loc + *i);
			
			cvdebug()<< "  check " << new_pos << " @ "; 
			if (! (new_pos < size) ){
				cverb << "outside\n"; 
				continue; 
			}
			cverb << data(new_pos); 

			if (data(new_pos) > value) {
				cverb << " gradient higher\n"; 
				continue; 
			}
			
			unsigned int new_pos_label = labels(new_pos);
			if (!new_pos_label) {
				cverb << " empty, add to queue\n"; 
				new_positions.push_back(new_pos); 
				continue; 
			}
			
			// already visited? 
			if (new_pos_label == label || new_pos_label == boundary_label) {
				cverb << " already visited\n"; 
				continue; 
			}
			
			// first label hit 
			if (!first_label) { 
				first_label = new_pos_label; 
				cverb << " set first label: "<< new_pos_label << "\n"; 
			}else if (first_label != new_pos_label) {
				// hit two different labels (apart from the original one) 
				loc_is_boundary = true; 
				cverb << " boundary\n"; 
			}else {
				cverb << " keep first label: "<< first_label << "\n"; 
			}
		}
		if (first_label) {
			if (!loc_is_boundary) {
				cvdebug() << "Got non-boundary label " << first_label << "\n"; 
				labels(loc) = first_label; 
				backtrack.push_back(loc); 
				if (first_label != label) {

					// we hit a single label from a lower gradient value, this means 
					// we are connected to an already labeled basin -> 
					//   first time = backtrack 
					//   later = boundary 
					if (!has_backtracked) {
						cvdebug() << "Backtrack " << label << " to " << first_label << "\n"; 
						for_each(backtrack.begin(), backtrack.end(), 
							 [&first_label, &labels](const Position& p){labels(p) = first_label;}); 
						label = first_label; 
						has_backtracked = true; 
					}else 
						labels(loc) = boundary_label; 
				}
			}else 
				labels(loc) = boundary_label; 
		} else { 
			labels(loc) = label;
			backtrack.push_back(loc); 
		}
		cvdebug() << "Set " << loc << " to " << labels(loc) << "\n"; 
		
		if (labels(loc) != boundary_label) {
			for_each(new_positions.begin(), new_positions.end(), 
				 [&locations](const Position& p){locations.push(p);}); 
		}

		// is there a queue that doesn't repeat values? 
		while (!locations.empty() && locations.top() == loc)
			locations.pop(); 
		
	}
	return has_backtracked; 
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
	auto l = labels.begin(); 
	long next_label = 1; 	
	while (i != e) {
		p.pos = i.pos(); 
		p.value = *i > m_thresh ? *i : m_thresh; 
		if (p.value <= m_thresh) {
			if (!*l) {
				*l = next_label;
				if (!grow(p, labels, data)) 
					++next_label; 
			}
		}else
			pixels.push(p); 
		++i; 
		++l; 
	}
	

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
				if ( l  && l != boundary_label) {
					if (!first_label)
						first_label = l; 
					else 
						if (first_label != l) 
							is_boundary = m_with_borders; 
				}
			}
		}
		if (first_label) {
			if (!is_boundary) 
				labels(pixel.pos) = first_label; 
			else 
				labels(pixel.pos) = boundary_label; 
			cvdebug() << " set " << pixel.pos << " with " << data(pixel.pos) << " to "<< labels(pixel.pos) <<"\n"; 
			continue; 
		}
		// new label to assign
		// if a new label is assigned, we have to grow the region of equal gradient values 
		// to assure we catch the whole bassin 
		labels(pixel.pos) = next_label; 
		if (!grow(pixel, labels, data)) 
			++next_label; 
	}
	// convert to smalles possible intensity range and convert the boundary label to highest 
	// intensity value
	CImage *r = NULL; 
	cvmsg() << "Got " << next_label << "distinct bassins\n"; 
	if (next_label < 255) {
		Image<unsigned char> *result = new Image<unsigned char>(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return (p != boundary_label) ? static_cast<unsigned char>(p) : 255; }); 
		r = result; 
	}else if (next_label < std::numeric_limits<unsigned short>::max()) {
		Image<unsigned short> *result = new Image<unsigned short>(data.get_size(), data); 
		transform(labels.begin(), labels.end(), result->begin(), 
			  [](unsigned int p){ return (p != boundary_label) ? static_cast<unsigned short>(p) : std::numeric_limits<unsigned short>::max(); });
		r = result; 
	}else {
		Image<unsigned int> * result = new Image<unsigned int>(data.get_size(), data); 
		copy(labels.begin(), labels.end(), result->begin());
		r = result; 
	}
	return PImage(r); 
	
}

template <int dim>
typename TWatershed<dim>::result_type TWatershed<dim>::do_filter(const CImage& image) const
{
	return m_togradnorm ?  mia::filter(*this, *m_togradnorm->filter(image)):
		mia::filter(*this, image); 
}

template <int dim>
TWatershedFilterPlugin<dim>::TWatershedFilterPlugin(): 
	watershed_traits<dim>::CPlugin("ws"), 
	m_with_borders(false), 
	m_thresh(0.0), 
	m_eval_grad(false)
{
	this->add_parameter("n", make_param(m_neighborhood, "sphere:r=1", false, "Neighborhood for watershead region growing")); 
	this->add_parameter("mark", new mia::CBoolParameter(m_with_borders, false, "Mark the segmented watersheds with a special gray scale value")); 
	this->add_parameter("thresh", new mia::CFloatParameter(m_thresh, 0, 1.0, false, "Gradient norm threshold. Bassins seperated by gradients "
						    "with a lower norm will be joined"));  
	this->add_parameter("evalgrad", new mia::CBoolParameter(m_eval_grad, false, "Set to 1 if the input image does not represent a gradient norm image")); 
}

template <int dim>
typename watershed_traits<dim>::CPlugin::Product *
TWatershedFilterPlugin<dim>::do_create() const
{
	return new TWatershed<dim>(m_neighborhood, m_with_borders, m_thresh, m_eval_grad);
}

template <int dim>
const std::string TWatershedFilterPlugin<dim>::do_get_descr()const
{
	return "basic watershead segmentation.";
}

NS_MIA_END

#endif
