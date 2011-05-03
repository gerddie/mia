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

#include <queue>
#include <stdexcept>

#include <boost/type_traits.hpp>

#include <mia/2d/2dfilter.hh>
#include <libmona/filter.hh>

#include <libmona/probmapio.hh>

namespace regiongrow_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const *plugin_name = "regiongrow";
static const CStringOption param_map("map", "seed class map", "");
static const CFloatOption param_thresh("low", "low threshold for acceptance probability", 0.5f, .0f, 1.0f); 
static const CFloatOption param_seed("seed", "threshold for seed probability", 0.9f, 0.0f, 1.0f); 
static const CIntOption   param_class("class", "class to be segmented", 2, 0, numeric_limits<int>::max()); 


class C2DReagiongrow: public C2DFilter {
	float m_low; 
	float m_seed; 
	CDoubleVector m_pv;
	vector<T2DVector<int> >  m_env;
public:
	C2DReagiongrow(float low, float seed,const CDoubleVector& pv):
		m_low(low), 
		m_seed(seed), 
		m_pv(pv)
	{
		m_env.push_back(T2DVector<int>(-1,  0)); 
		m_env.push_back(T2DVector<int>( 1,  0)); 
		m_env.push_back(T2DVector<int>( 0, -1)); 
		m_env.push_back(T2DVector<int>( 0,  1)); 
	}
	
	template <class Data2D>
	typename C2DReagiongrow::result_type operator () (const Data2D& data) const ;

};


class C2DReagiongrowImageFilter: public C2DImageFilterBase {
	C2DReagiongrow m_filter; 
public:
	C2DReagiongrowImageFilter(float low, float seed,const CDoubleVector& pv);

	virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DReagiongrowImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DReagiongrowImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
};

template <typename T, bool is_integral>
struct FBinarizeRegionGrow {
	FBinarizeRegionGrow(float thresh, const CDoubleVector& pv):
		m_thresh(thresh), 
		m_pv(pv)
	{
	}
	
	bool operator() (T x) {
		size_t xi = x; 
		if (xi < m_pv.size())
			return m_pv[xi] >= m_thresh; 
		else
			return false; 
	}
private: 
	float m_thresh; 
	const CDoubleVector& m_pv; 
};

template <typename T>
struct FBinarizeRegionGrow<T, false> {
	FBinarizeRegionGrow(float thresh, const CDoubleVector& pv)
	{
		throw invalid_argument("Reagiongrow::FBinarizeRegionGrow:only integral valued input images are supported"); 
	}
	
	bool operator()(T x) {
		throw invalid_argument("Regiongrow::FBinarizeRegionGrow:only integral valued input images are supported"); 
	}
};



template <class Data2D>
typename C2DReagiongrow::result_type C2DReagiongrow::operator () (const Data2D& data) const
{
	const bool is_integral = ::boost::is_integral<typename Data2D::value_type>::value; 
	
	C2DBitImage *result = new C2DBitImage(data.get_size()); 
	
	C2DBitImage low_thresh(data.get_size()); 
	
	// find seed segmentation 
	transform(data.begin(), data.end(), result->begin(), 
		  FBinarizeRegionGrow<typename Data2D::value_type, is_integral>(m_seed, m_pv)); 
	
	// find maximum segmentation
	transform(data.begin(), data.end(), low_thresh.begin(), 
		  FBinarizeRegionGrow<typename Data2D::value_type, is_integral>(m_low, m_pv)); 
	
	// grow the seed until it hits the low_thresh
	
	C2DBitImage::iterator r = result->begin(); 
	
	vector<T2DVector<int> >::const_iterator ke = m_env.end(); 
	
	queue<C2DBounds> seed_points; 
	
	for (size_t y = 0; y < data.get_size().y; ++y)
		for (size_t x = 0; x < data.get_size().x; ++x, ++r) {
			if (*r) 
				seed_points.push(C2DBounds(x,y)); 
		}
	
	while (!seed_points.empty()) {
		C2DBounds p = seed_points.front(); 
		seed_points.pop(); 
		
		vector<T2DVector<int> >::const_iterator kb = m_env.begin(); 
		while (kb != ke) {
			size_t ix = kb->x + p.x; 
			if (ix < data.get_size().x) {
				size_t iy = kb->y + p.y;
				if (iy < data.get_size().y) {
					if (low_thresh(ix,iy) && !(*result)(ix, iy)) {
						(*result)(ix, iy) = true; 
						seed_points.push(C2DBounds(ix, iy)); 
					}
				}
			}
			++kb; 
		}
	}
	
	return P2DImage(result); 
}

C2DReagiongrowImageFilter::C2DReagiongrowImageFilter(float low, float seed,const CDoubleVector& pv):
	m_filter(low, seed, pv)
{
}

P2DImage C2DReagiongrowImageFilter::do_filter(const C2DImage& image) const
{
	return wrap_filter(m_filter,image); 
}

C2DReagiongrowImageFilterFactory::C2DReagiongrowImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_map);
	add_help(param_seed);
	add_help(param_thresh);
}

C2DFilterPlugin::ProductPtr C2DReagiongrowImageFilterFactory::create(const CParsedOptions& options) const
{
	string map_name = param_map.get_value(options); 
	float seed_thresh = param_seed.get_value(options); 
	float low_thresh = param_thresh.get_value(options); 
	size_t sclass = param_class.get_value(options); 
	
	CProbabilityVector pv = load_probability_map(map_name); 
	if (pv.empty())
		throw invalid_argument(string("Unable to load probability map from ") + map_name); 
	
	if (sclass >= pv.size()) {
		stringstream errmsg; 
		errmsg << "selected class '" << sclass << "' out of probability map range"; 
		throw invalid_argument(errmsg.str()); 
		return C2DFilterPlugin::ProductPtr();
	}
	
	return C2DFilterPlugin::ProductPtr(new C2DReagiongrowImageFilter(low_thresh, seed_thresh, pv[sclass]));
}

const string C2DReagiongrowImageFilterFactory::do_get_descr()const
{
	return "2D region growing with probability based stopping funtion"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DReagiongrowImageFilterFactory(); 
}
} // end namespace regiongrow_2dimage_filter
