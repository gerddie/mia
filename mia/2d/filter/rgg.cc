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
#include <boost/type_traits.hpp>

#include <mia/2d/2dfilter.hh>
#include <libmia/filter.hh>

#include <libmia/probmapio.hh>
#include "rgg_tools.hh"	

namespace rgg_2dimage_filter {

	
NS_MIA_USE;
using namespace std; 

static char const *plugin_name = "rgg";
static const CStringOption param_map("map", "seed class map", "");
static const CFloatOption param_seed("seed", "threshold for seed probability", 0.9f, 0.0f, 1.0f); 


	
class C2DRGG: public C2DFilter {
	float m_seed; 
	CProbabilityVector m_pv;
	vector<T2DVector<int> >  m_env;
public:
	C2DRGG(float seed,const CProbabilityVector& pv):
		m_seed(seed), 
		m_pv(pv)
	{
		m_env.push_back(T2DVector<int>(-1,  0)); 
		m_env.push_back(T2DVector<int>( 1,  0)); 
		m_env.push_back(T2DVector<int>( 0, -1)); 
		m_env.push_back(T2DVector<int>( 0,  1)); 
	}
	
	template <class T>
	typename C2DRGG::result_type operator () (const T2DImage<T>& data) const ;

};


class C2DRGGImageFilter: public C2DImageFilterBase {
	C2DRGG m_filter; 
public:
	C2DRGGImageFilter(float seed,const CProbabilityVector& pv); 


	virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DRGGImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DRGGImageFilterFactory();
	virtual C2DFilter *create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};


template <class T>
typename C2DRGG::result_type C2DRGG::operator () (const T2DImage<T>& data) const
{
	
//	const bool is_integral = ::boost::is_integral<T>::value; 
	
	C2DUBImage *result = new C2DUBImage(data.get_size()); 
	
	// find seed segmentation 
	transform(data.begin(), data.end(), result->begin(), FMapClass(m_seed, m_pv)); 
	
	// evaluate all boundary pixels and get gradient
	
	C2DUBImage::iterator r = result->begin(); 
	
	vector<T2DVector<int> >::const_iterator ke = m_env.end(); 
	
	priority_queue<Contact<C2DBounds,T> > contacts; 
	
	typename T2DImage<T>::const_iterator di = data.begin(); 
	
	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++r, ++di) {
			if (*r != undefined) {
				vector<T2DVector<int> >::const_iterator kb = m_env.begin(); 
				while (kb != ke) {
					size_t ix = kb->x + x; 
					if (ix < data.get_size().x) {
						size_t iy = kb->y + y;
						if (iy < data.get_size().y) {
							if ((*result)(ix, iy) == undefined) {
								T p2 = data(ix, iy);
								Contact<C2DBounds,T> c(C2DBounds(ix, iy), *r, p2, *di > p2 ? *di - p2 : p2 - *di, 0);
								contacts.push(c);  
								//cvdebug() << "contact: " << c << "\n";
							}
						}
					}
					++kb; 
				}
			}
		}
	
	cvdebug() << "Have " << contacts.size() << " contact points\n"; 
	
        // run through the contacts, and add to the neighbor
	while (!contacts.empty()) {
		Contact<C2DBounds, T> c = contacts.top(); 
		contacts.pop(); 
		
		C2DUBImage::value_type& r = (*result)(c.l); 
		// it is possible, that this value is already set by a better neighbour
		if (r != undefined) 
			continue; 
		
		r = c.cl;
	
		vector<T2DVector<int> >::const_iterator kb = m_env.begin(); 
		while (kb != ke) {
			size_t ix = kb->x + c.l.x; 
			if (ix < data.get_size().x) {
				size_t iy = kb->y + c.l.y;
				if (iy < data.get_size().y) {
					if ((*result)(ix, iy) == undefined) {
						T p2 = data(ix, iy);
						Contact<C2DBounds,T> c_new(C2DBounds(ix, iy), c.cl, p2, 
									   c.value > p2 ? c.value - p2 : p2 - c.value, 0);
						contacts.push(c_new);  
						//cvdebug() << "contact: " << c_new << "\n"; 
					}
				}
			}
			++kb; 
		}
	}
		
	return P2DImage(result); 
}

C2DRGGImageFilter::C2DRGGImageFilter(float seed,const CProbabilityVector& pv):
	m_filter( seed, pv)
{
}

P2DImage C2DRGGImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter,image); 
}

C2DRGGImageFilterFactory::C2DRGGImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_map);
	add_help(param_seed);
}

C2DFilter *C2DRGGImageFilterFactory::create(const CParsedOptions& options) const
{
	string map_name = param_map.get_value(options); 
	float seed_thresh = param_seed.get_value(options); 
	
	CProbabilityVector pv = ::load_probability_map(map_name); 
	if (pv.empty())
		throw invalid_argument(string("Unable to load probability map from ") + map_name); 
	
	
	
	return new C2DRGGImageFilter(seed_thresh, pv);
}

const string C2DRGGImageFilterFactory::do_get_descr()const
{
	return "2D region growing with probability based stopping funtion"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRGGImageFilterFactory(); 
}
} // end namespace rgg_2dimage_filter
