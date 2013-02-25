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

#ifndef mia_internal_seededwatershed_hh
#define mia_internal_seededwatershed_hh

#include <queue>
#include <mia/template/dimtrait.hh>

NS_MIA_BEGIN

///  @cond DOC_PLUGINS 

template <int dim> 
class TSeededWS : public watershed_traits<dim>::Handler::Product {
public:
	typedef typename watershed_traits<dim>::PNeighbourhood  PNeighbourhood; 
	typedef typename PNeighbourhood::element_type::value_type MPosition; 
	typedef typename watershed_traits<dim>::Handler Handler; 
	typedef typename watershed_traits<dim>::FileHandler FileHandler; 
	typedef typename Handler::Product CFilter; 
	typedef typename FileHandler::Instance::DataKey  DataKey; 
	typedef typename CFilter::Pointer PFilter; 
	typedef typename CFilter::Image CImage; 
	typedef typename CImage::Pointer PImage; 
	typedef typename CImage::dimsize_type Position; 
	

	TSeededWS(const DataKey& mask_image, PNeighbourhood neighborhood, 
		  bool with_borders, bool input_is_gradient);

	template <template <typename>  class Image, typename T>
	typename TSeededWS<dim>::result_type operator () (const Image<T>& data) const;
private:
	virtual PImage do_filter(const CImage& image) const;

	DataKey m_label_image_key; 
	PNeighbourhood m_neighborhood; 
	PFilter m_togradnorm; 
	bool m_with_borders; 
};

template <int dim> 
class TSeededWSFilterPlugin: public watershed_traits<dim>::Handler::Interface {
public:
	typedef typename watershed_traits<dim>::PNeighbourhood  PNeighbourhood; 
	typedef typename watershed_traits<dim>::Handler Handler; 
	typedef typename watershed_traits<dim>::FileHandler FileHandler; 
	typedef typename Handler::Product CFilter; 

	TSeededWSFilterPlugin();
private:
	virtual CFilter *do_create()const;
	virtual const std::string do_get_descr()const;
	std::string m_seed_image_file;
	PNeighbourhood m_neighborhood; 
	bool m_with_borders; 
	bool m_input_is_gradient; 
};

template <template <typename>  class Image, typename  T, typename  S, typename N, typename R, int dim, bool supported>
struct seeded_ws {
	static R apply(const Image<T>& image, const Image<S>& seed, N n, bool with_borders); 
}; 

template <template <typename>  class Image, typename  T, typename  S, typename N, typename R, int dim>
struct seeded_ws<Image, T, S, N, R, dim, false> {
	static R apply(const Image<T>& /*image*/, const Image<S>& /*seed*/, N /*n*/, bool /*with_borders*/) {
		throw create_exception<std::invalid_argument>("C2DRunSeededWS: seed data type '", __type_descr<S>::value, "' not supported");
	}
}; 


// helper to dispatch based on the pixel type of the seed image 
template <template <typename>  class Image, typename  T, typename N, typename R, int dim>
struct dispatch_RunSeededWS : public TFilter<R> {
	
	dispatch_RunSeededWS(N neighborhood, const Image<T>& image, bool with_borders):
		m_neighborhood(neighborhood), 
		m_image(image), 
		m_with_borders(with_borders)
		{}

	template <typename  S>
	R operator () (const Image<S>& seed) const {
		const bool supported = std::is_integral<S>::value && !std::is_same<S, bool>::value; 
		return seeded_ws<Image, T, S, N, R, dim, supported>::apply(m_image, seed, m_neighborhood, m_with_borders); 
	}
	N m_neighborhood; 
	const Image<T>& m_image; 
	bool m_with_borders; 
}; 


template <typename L, typename Position>
struct PixelWithLocation {
	Position pos; 
	float value; 
	L label; 
}; 

template <typename L, typename Position>
bool operator < (const PixelWithLocation<L,Position>& lhs, const PixelWithLocation<L,Position>& rhs) 
{		
	mia::less_then<Position> l; 
	return lhs.value > rhs.value|| 
		( lhs.value ==  rhs.value && l(rhs.pos, lhs.pos)); 
}

template <template <typename>  class Image, typename T, typename S, int dim>
class TRunSeededWatershed {
public: 
	typedef typename watershed_traits<dim>::PNeighbourhood  PNeighbourhood; 
	typedef typename PNeighbourhood::element_type::value_type MPosition; 
	typedef typename watershed_traits<dim>::Handler Handler; 
	typedef typename Handler::Product CFilter; 
	typedef typename CFilter::Pointer PFilter; 
	typedef typename CFilter::Image CImage; 
	typedef typename CImage::Pointer PImage; 
	typedef typename CImage::dimsize_type Position; 
	

	TRunSeededWatershed(const Image<T>& image, const Image<S>& seed, PNeighbourhood neighborhood, bool with_borders); 
	PImage run(); 
private: 
	void add_neighborhood(const PixelWithLocation<S, Position>& pixel); 	
	const Image<T>& m_image;
	const Image<S>& m_seed; 
	PNeighbourhood m_neighborhood; 
	Image<unsigned char> m_visited; 
	Image<unsigned char>  m_stored; 
	Image<S>       *m_result;
	PImage m_presult; 
	std::priority_queue<PixelWithLocation<S, Position> > m_seeds; 
	S m_watershed; 
	bool m_with_borders; 
}; 

template <template <typename>  class Image, typename T, typename S, int dim>
TRunSeededWatershed<Image, T, S, dim>::TRunSeededWatershed(const Image<T>& image, const Image<S>& seed, 
							   PNeighbourhood neighborhood, bool with_borders):
	m_image(image), 
	m_seed(seed), 
	m_neighborhood(neighborhood), 
	m_visited(seed.get_size()),
	m_stored(seed.get_size()),
	m_watershed(std::numeric_limits<S>::max()), 
	m_with_borders(with_borders)
{
	m_result = new Image<S>(seed.get_size(), image); 
	m_presult.reset(m_result); 
}

template <template <typename>  class Image, typename T, typename S, int dim>
void TRunSeededWatershed<Image, T, S, dim>::add_neighborhood(const PixelWithLocation<S, Position>& pixel)
{
	PixelWithLocation<S, Position> new_pixel; 
	new_pixel.label = pixel.label; 
	bool hit_boundary = false; 
	for (auto i = m_neighborhood->begin(); i != m_neighborhood->end(); ++i) {
		Position new_pos( pixel.pos + *i); 
		if (new_pos != pixel.pos && new_pos < m_seed.get_size()) {
			if (!m_visited(new_pos)) {
				if (!m_stored(new_pos) ) {
					new_pixel.value = m_image(new_pos); 
					new_pixel.pos = new_pos; 
					m_seeds.push(new_pixel); 
					m_stored(new_pos) = 1; 
				}
			}else{
				hit_boundary |= (*m_result)(new_pos) != pixel.label &&
					(*m_result)(new_pos) != m_watershed; 
			}
		}
	}
	// set pixel to new label 
	if (!m_visited(pixel.pos)) {
		m_visited(pixel.pos) = true; 
		(*m_result)(pixel.pos) = (m_with_borders && hit_boundary) ? m_watershed : pixel.label;
	}
}

template <template <typename>  class Image, typename T, typename S, int dim>
typename TRunSeededWatershed<Image,T,S,dim>::PImage 
TRunSeededWatershed<Image,T,S,dim>::run()
{
	// copy seed and read initial pixels 
	auto iv = m_visited.begin(); 
	auto is = m_seed.begin_range(Position::_0, m_seed.get_size());
	auto es = m_seed.end_range(Position::_0, m_seed.get_size());
	auto ir = m_result->begin();
	auto ii = m_image.begin(); 
	auto ist = m_stored.begin(); 
	
	PixelWithLocation<S, Position> pixel; 
	while (is != es) {
		*ir = *is; 
		if (*is) {
			*iv = 1; 
			*ist; 
			pixel.pos = is.pos(); 
			pixel.value = *ii; 
			pixel.label = *is; 
			m_seeds.push(pixel); 
		}
		++iv; ++is; ++ir; ++ist; ++ii; 
	}
	
	while (!m_seeds.empty()) {
		auto p = m_seeds.top(); 
		m_seeds.pop(); 
		add_neighborhood(p); 
	}
	return m_presult; 
}

template <template <typename>  class Image, typename  T, typename  S, typename N, typename R, int dim, bool supported>
R seeded_ws<Image,T,S,N,R,dim,supported>::apply(const Image<T>& image, 
						const Image<S>& seed, N neighborhood, bool with_borders) 
{
	TRunSeededWatershed<Image, T, S, dim> ws(image, seed, neighborhood, with_borders);
	return ws.run(); 
}

template <int dim> 
TSeededWS<dim>::TSeededWS(const DataKey& label_image_key, PNeighbourhood neighborhood, bool with_borders, bool input_is_gradient):
	m_label_image_key(label_image_key), 
	m_neighborhood(neighborhood), 
	m_with_borders(with_borders)
	
{
	if (!input_is_gradient) 
		m_togradnorm = Handler::instance().produce("gradnorm"); 
}

template <int dim> 
template <template <typename>  class Image, typename T>
typename TSeededWS<dim>::result_type TSeededWS<dim>::operator () (const Image<T>& data) const 
{
	// read start label image
	auto in_image_list = m_label_image_key.get();
	if (!in_image_list || in_image_list->empty())
		throw create_exception<std::runtime_error>( "C2DSeededWS: no seed image could be loaded");
	
	if (in_image_list->size() > 1) 
		cvwarn() << "C2DSeededWS:got more than one seed image. Ignoring all but first"; 
	
	auto seed = (*in_image_list)[0]; 
	if (seed->get_size() != data.get_size()) {
		throw create_exception<std::invalid_argument>( "C2DSeededWS: seed and input differ in size: seed " , seed->get_size() 
		      , ", input " , data.get_size()); 
	}
	dispatch_RunSeededWS<Image, T, PNeighbourhood, PImage, dim> ws(m_neighborhood, data, m_with_borders); 
	return mia::filter(ws, *seed); 
}

template <int dim> 
typename TSeededWS<dim>::PImage TSeededWS<dim>::do_filter(const CImage& image) const
{
	PImage result;  
	if (m_togradnorm) {
		auto grad = m_togradnorm->filter(image); 
		result = mia::filter(*this, *grad);
	}
	else
		result = mia::filter(*this, image);
	
	return result;		
}

template <int dim> 
TSeededWSFilterPlugin<dim>::TSeededWSFilterPlugin(): 
	Handler::Interface("sws"), 
	m_with_borders(false), 
	m_input_is_gradient(false)
{
	this->add_parameter("seed", new CStringParameter(m_seed_image_file, true, 
						   "seed input image containing the lables for the initial regions"));
	this->add_parameter("n", make_param(m_neighborhood, "sphere:r=1", false, "Neighborhood for watershead region growing")); 
	this->add_parameter("mark", new CBoolParameter(m_with_borders, false, "Mark the segmented watersheds with a special gray scale value")); 
	this->add_parameter("grad", new CBoolParameter(m_input_is_gradient, false, "Interpret the input image as gradient. ")); 
}

template <int dim> 
typename TSeededWSFilterPlugin<dim>::CFilter *TSeededWSFilterPlugin<dim>::do_create()const
{
	auto seed = FileHandler::instance().load_to_pool(m_seed_image_file);
	return new TSeededWS<dim>(seed, m_neighborhood, m_with_borders, m_input_is_gradient);
}

template <int dim> 
const std::string TSeededWSFilterPlugin<dim>::do_get_descr()const
{
	return "seeded watershead. The algorithm extracts exactly so "
		"many reagions as initial labels are given in the seed image.";
}

NS_MIA_END

///  @endcond
#endif
