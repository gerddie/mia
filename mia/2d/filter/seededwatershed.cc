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
#include <mia/2d/filter/seededwatershed.hh>

NS_BEGIN( sws_2dimage_filter)
using namespace mia; 
using namespace std; 


template <typename  T, typename  S, bool supported>
struct seeded_ws {
	static P2DImage apply(const T2DImage<T>& image, const T2DImage<S>& seed, P2DShape neighborhood); 
}; 

template <typename  T, typename  S>
struct seeded_ws<T,S,false> {
	static P2DImage apply(const T2DImage<T>& /*image*/, const T2DImage<S>& /*seed*/, P2DShape /*neighborhood*/) {
		throw invalid_argument("C2DRunSeededWS: seed data type not supported"); 
	}
}; 


// helper to dispatch based on the pixel type of the seed image 
template <typename T> 
struct dispatch_RunSeededWS : public TFilter<P2DImage> {
	
	dispatch_RunSeededWS(P2DShape neighborhood, const T2DImage<T>& image):
		m_neighborhood(neighborhood), 
		m_image(image)
		{}

	template <typename  S>
	P2DImage operator () (const T2DImage<S>& seed) const {
		const bool supported = is_integral<S>::value && !is_same<S, bool>::value; 
		return seeded_ws<T,S, supported>::apply(m_image, seed, m_neighborhood); 
	}
	P2DShape m_neighborhood; 
	const T2DImage<T>& m_image; 
}; 


template <typename L>
struct PixelWithLocation {
	C2DBounds pos; 
	float value; 
	L label; 
}; 

template <typename L>
bool operator < (const PixelWithLocation<L>& lhs, const PixelWithLocation<L>& rhs) 
{
	return lhs.value > rhs.value; 
}

template <typename T, typename S>
class TRunSeededWatershed {
public: 
	TRunSeededWatershed(const T2DImage<T>& image, const T2DImage<S>& seed, P2DShape neighborhood); 
	P2DImage run(); 
private: 
	void add_neighborhood(const PixelWithLocation<S>& pixel); 	
	const T2DImage<T>& m_image;
	const T2DImage<S>& m_seed; 
	P2DShape m_neighborhood; 
	C2DUBImage m_visited; 
	T2DImage<S>       *m_result;
	P2DImage m_presult; 
	priority_queue<PixelWithLocation<S> > m_seeds; 
	S m_watershed; 
}; 

template <typename T, typename S>
TRunSeededWatershed<T,S>::TRunSeededWatershed(const T2DImage<T>& image, const T2DImage<S>& seed, P2DShape neighborhood):
	m_image(image), 
	m_seed(seed), 
	m_neighborhood(neighborhood), 
	m_visited(seed.get_size()),
	m_watershed(numeric_limits<S>::max())
{
	m_result = new T2DImage<S>(seed.get_size(), image); 
	m_presult.reset(m_result); 
}

template <typename T, typename S>
void TRunSeededWatershed<T,S>::add_neighborhood(const PixelWithLocation<S>& pixel)
{
	PixelWithLocation<S> new_pixel; 
	new_pixel.label = pixel.label; 
	bool hit_boundary = false; 
	for (auto i = m_neighborhood->begin(); i != m_neighborhood->end(); ++i) {
		C2DBounds new_pos( pixel.pos.x + i->x, pixel.pos.y + i->y); 
		if (new_pos.x < m_seed.get_size().x && new_pos.y < m_seed.get_size().y) {
			if (!m_visited(new_pos)) {
				new_pixel.value = m_image(new_pos); 
				new_pixel.pos = new_pos; 
				m_seeds.push(new_pixel); 
			}else{
				hit_boundary |= (*m_result)(new_pos) != pixel.label && 
					(*m_result)(new_pos) != m_watershed; 
			}
		}
	}
	// set pixel to new label 
	if (!m_visited(pixel.pos)) {
		m_visited(pixel.pos) = true; 
		(*m_result)(pixel.pos) = hit_boundary ? m_watershed :pixel.label;
	}
}

template <typename T, typename S>
P2DImage TRunSeededWatershed<T,S>::run()
{
	// copy seed and read initial pixels 
	auto iv = m_visited.begin(); 
	auto is = m_seed.begin(), ir = m_result->begin();
	auto ii = m_image.begin(); 
	
	PixelWithLocation<S> pixel; 
	for (pixel.pos.y = 0; pixel.pos.y < m_seed.get_size().y; ++pixel.pos.y) 
		for (pixel.pos.x = 0; pixel.pos.x < m_seed.get_size().x; 
		     ++pixel.pos.x, ++is, ++ir, ++iv, ++ii) {
			*ir = *is; 
			if (*is) {
				*iv = 1; 
				pixel.value = *ii; 
				pixel.label = *is; 
				m_seeds.push(pixel); 
			}
		}
	
	while (!m_seeds.empty()) {
		auto p = m_seeds.top(); 
		m_seeds.pop(); 
		add_neighborhood(p); 
	}
	return m_presult; 
}

template <typename T, typename S, bool supported>
P2DImage seeded_ws<T,S,supported>::apply(const T2DImage<T>& image, const T2DImage<S>& seed, P2DShape neighborhood) 
{
	TRunSeededWatershed<T,S> ws(image, seed, neighborhood); 
	return ws.run(); 
}


C2DSeededWS::C2DSeededWS(const mia::C2DImageDataKey& label_image_key, P2DShape neighborhood):
	m_label_image_key(label_image_key), 
	m_neighborhood(neighborhood)
	
{
	m_togradnorm = produce_2dimage_filter("gradnorm"); 
}

template <class T>
typename C2DSeededWS::result_type C2DSeededWS::operator () (const T2DImage<T>& data) const 
{
	// read start label image
	C2DImageIOPlugin::PData in_image_list = m_label_image_key.get();
	if (!in_image_list || in_image_list->empty())
		THROW(runtime_error, "C2DSeededWS: no seed image could be loaded");
	
	if (in_image_list->size() > 1) 
		cvwarn() << "C2DSeededWS:got more than one seed image. Ignoring all but first"; 
	
	P2DImage seed = (*in_image_list)[0]; 
	if (seed->get_size() != data.get_size()) {
		THROW(invalid_argument, "C2DSeededWS: seed and input differ in size: seed " << seed->get_size() 
		      << ", input " << data.get_size()); 
	}
	dispatch_RunSeededWS<T> ws(m_neighborhood, data); 
	return mia::filter(ws, *seed); 
}


P2DImage C2DSeededWS::do_filter(const C2DImage& image) const
{
	P2DImage grad = m_togradnorm->filter(image); 
	return mia::filter(*this, *grad);
}

C2DSeededWSFilterPlugin::C2DSeededWSFilterPlugin(): 
	C2DFilterPlugin("sws")
{
	add_parameter("seed", new CStringParameter(m_seed_image_file, true, 
						   "seed input image containing the lables for the initial regions"));
	add_parameter("n", make_param(m_neighborhood, "8n", false, "Neighborhood for watershead region growing")); 
}


C2DFilter *C2DSeededWSFilterPlugin::do_create()const
{
	C2DImageDataKey seed = C2DImageIOPluginHandler::instance().load_to_pool(m_seed_image_file);
	return new C2DSeededWS(seed, m_neighborhood);
}

const string C2DSeededWSFilterPlugin::do_get_descr()const
{
	return "2D seesed watershead. The algorithm extracts exactly so "
		"many reagions as initial labels are given in the seed image.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSeededWSFilterPlugin();
}


NS_END
