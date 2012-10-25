/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#include <queue>
#include <stdexcept>

#include <mia/2d/filter/regiongrow.hh>

NS_BEGIN(regiongrow_2d_filter)
NS_MIA_USE; 
using namespace std; 

C2DRegiongrowFilter::C2DRegiongrowFilter(const mia::C2DImageDataKey& seed_image_key, mia::P2DShape neighborhood):
	m_seed_image_key(seed_image_key)
{
	m_neighborhood.reserve(neighborhood->size() - 1);
	for (auto i = neighborhood->begin(); i != neighborhood->end();++i) 
		if ( *i != MPosition::_0 ) 
			m_neighborhood.push_back(*i); 
}
	
struct GrowLocation {
	C2DBounds pos; 
	float value; 
}; 


template <typename  T>
C2DRegiongrowFilter::result_type C2DRegiongrowFilter::operator () (const mia::T2DImage<T>& data) const
{
	auto size = data.get_size(); 

	C2DImageIOPlugin::PData in_image_list = m_seed_image_key.get();
	
	if (!in_image_list || in_image_list->empty())
		throw create_exception<invalid_argument>( "C2DRegiongrowFilter: Empty image list loaded from pool");

	P2DImage pseed_image = (*in_image_list)[0];
	if (pseed_image->get_pixel_type() != it_bit)
		throw invalid_argument("C2DRegiongrowFilter: seed image must be of type 'bit'"); 
	const C2DBitImage& seed_image = dynamic_cast<const C2DBitImage&>(*pseed_image); 

	if (size != seed_image.get_size())
		throw create_exception<invalid_argument>( "Input image is of size ", size, 
						", but seed image is ", seed_image.get_size());
	
	C2DBitImage *presult = new C2DBitImage(size, data); 
	P2DImage result(presult); 

	queue<GrowLocation> seeds; 
	auto i = data.begin_range(Position::_0, size);
	auto e = data.end_range(Position::_0, size);
	auto s = seed_image.begin(); 
	auto r = presult->begin(); 

	// read initial seeds and set initial output 
	while (i != e) {
		if (*s) {
			GrowLocation gloc; 
			gloc.pos = i.pos();
			gloc.value = *i; 
			seeds.push(gloc); 
		}
		++i; ++r, ++s; 
	}

	while (!seeds.empty()) {
		auto t = seeds.front(); 
		seeds.pop(); 
		
		// already visited?  
		if ((*presult)(t.pos)) 
			continue;  
		
		// set output pixel 
		(*presult)(t.pos) = true; 
		for (auto n = m_neighborhood.begin(); n != m_neighborhood.end(); ++n) {
			GrowLocation gloc; 
			gloc.pos = Position( t.pos + *n);
			
			// inside? 
			if (gloc.pos < size) {
				// already visited? 
				if ((*presult)(gloc.pos)) 
					continue; 
			
				gloc.value = data(gloc.pos); 
				if (gloc.value >= t.value)
					seeds.push(gloc); 
			}
		}
	}
	return result; 
}

C2DRegiongrowFilter::result_type C2DRegiongrowFilter::do_filter(const mia::C2DImage& image) const
{
	return mia::filter(*this, image); 
}

C2DRegiongrowFilterPlugin::C2DRegiongrowFilterPlugin(): 
	mia::C2DFilterPlugin("regiongrow")
{
	add_parameter("seed", new CStringParameter(m_seed_image, true, "seed image (bit valued)", 
						   &C2DImageIOPluginHandler::instance()));
	add_parameter("n", make_param(m_neighborhood, "8n", false, "Neighborhood shape"));
}

mia::C2DFilter *C2DRegiongrowFilterPlugin::do_create()const
{
	C2DImageDataKey seed_key = C2DImageIOPluginHandler::instance().load_to_pool(m_seed_image);
	return new C2DRegiongrowFilter(seed_key, m_neighborhood); 
}

const std::string C2DRegiongrowFilterPlugin::do_get_descr()const
{
	return "Region growing startin from a seed until only along increasing gradients";  
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRegiongrowFilterPlugin();
}

NS_END
