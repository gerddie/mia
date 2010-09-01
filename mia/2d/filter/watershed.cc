/* -*- mona-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <queue>
#include <sstream>
#include <stdexcept>

#include <boost/type_traits.hpp>

#include <mia/2d/2dfilter.hh>
#include <libmona/filter.hh>

#include <libmona/probmapio.hh>

namespace watershed_2dimage_filter {
NS_MIA_USE;
using namespace std; 

static char const *plugin_name = "watershed";
static const CStringOption param_map("map", "seed class map", "");
static const CIntOption param_width("w", "window width for variation evaluation", 1,1,1000);
static const CFloatOption param_thresh("thresh", "threshold for seed probability", 0.9, 0.5, 1.0); 
static const CFloatOption param_tol("tol", "tolerance for watershed", 0.8, 0.1, 1.2); 
static const CFloatOption param_steep("p", "decision parameter whether fuzzy segmentation or neighborhood should be used", 1.5, 1, 5.0); 
static const CIntOption param_class("class", "class to be segmented", 2, 0, 254); 

class C2DWatershedFilter: public C2DFilter {
	float _M_thresh; 
	float _M_tol; 
	float _M_steep; 
	CProbabilityVector _M_pv;
	size_t _M_sclass; 
public:
	C2DWatershedFilter(float thresh, float steep, float tol, const CProbabilityVector& pv, size_t sclass):
		_M_thresh(thresh), 
		_M_tol(tol), 
		_M_steep(steep),
		_M_pv(pv), 
		_M_sclass(sclass)
	{
	}
	
	template <typename Data2D, typename Var2D>
	typename C2DWatershedFilter::result_type operator () (const Data2D& data, const Var2D& var_image) const ;

};


class C2DWatershedFilterImageFilter: public C2DImageFilterBase {
	C2DWatershedFilter _M_filter; 
	C2DFilterPlugin::ProductPtr _M_var_evaluator; 
public:
	C2DWatershedFilterImageFilter(int hwidth, float thresh, float steep, float tol, const CProbabilityVector& pv, size_t sclass);

	virtual P2DImage do_filter(const C2DImage& image) const;
};


class C2DWatershedFilterImageFilterFactory: public C2DFilterPlugin {
public: 
	C2DWatershedFilterImageFilterFactory();
	virtual C2DFilterPlugin::ProductPtr create(const CParsedOptions& options) const;
	virtual const string do_get_descr()const; 
private: 
//	virtual int do_test() const; 
};

inline void fill(const C2DBounds& p, C2DBitImage& result, queue< C2DBounds >& seedpoints,  const C2DFImage& var_image, 
		 float var, queue< C2DBounds >& backlog)
{
	if (!(p.x < result.get_size().x && p.y < result.get_size().y))
		return; 
	
	if ( !result(p)) {
		if (var_image(p) >= var) {
			result(p) = true; 
			seedpoints.push(p); 
		}else
			backlog.push(p); 
	}
}

template <typename D, typename V, bool is_integral> 
struct  dispatch_filter {
	static P2DImage apply(const D& data, const CProbabilityVector& pv, float steep, 
				  float thresh, float tol, const V& var_image, size_t sclass) {
		throw invalid_argument("Non integral data type not supported"); 
		return P2DImage(); 
	}
};

template <typename D, typename V> 
struct  dispatch_filter<D, V, true> {
	static P2DImage apply(const D& data, const CProbabilityVector& pv, float steep, 
				  float thresh, float tol, const V& var_image, size_t sclass) {
		assert(!"The variation image must be of type float");
		return P2DImage(); 
	}
};

template <typename D>
struct  dispatch_filter<D, C2DFImage, true> {
	static P2DImage apply(const D& data, const CProbabilityVector& pv, float steep, 
				  float thresh, float tol, const C2DFImage& var_image, size_t sclass) {
		
		typedef typename D::value_type T; 
		
		cvdebug() << "Evaluate seed points\n"; 
		// create the target image
		C2DBitImage *result = new C2DBitImage(data.get_size()); 
		
		C2DBitImage::iterator r = result->begin(); 
		typename D::const_iterator i = data.begin(); 
		
		queue< C2DBounds > seedpoints;
		queue< C2DBounds > backlog; 
		
		const CDoubleVector& pv_class = pv[sclass]; 
		
		// find all points above the given probability threshold and use them as seed points
		for (size_t y = 0; y < data.get_size().y; ++y)
			for (size_t x = 0; x < data.get_size().x; ++x, ++r, ++i) {
				*r = false; 
				size_t ival = *i; 
				if ( ival < pv_class.size()) {
					if (pv_class[ival] > thresh) {
						*r = true;
						seedpoints.push(C2DBounds(x,y)); 
						break; 
					}
				}
			}
		// target image contains a seed segmentation
		
		vector<int> counter(pv.size());
		
		while (!seedpoints.empty()) {
			
			// do the watershead segmentation
			while ( !seedpoints.empty() ) {
				C2DBounds p = seedpoints.front(); 
				seedpoints.pop(); 
				
				float var = tol * var_image(p); 

				
				fill(C2DBounds(p.x - 1, p.y), *result, seedpoints, var_image, var, backlog); 
				fill(C2DBounds(p.x + 1, p.y), *result, seedpoints, var_image, var, backlog); 
				fill(C2DBounds(p.x, p.y - 1), *result, seedpoints, var_image, var, backlog); 
				fill(C2DBounds(p.x, p.y + 1), *result, seedpoints, var_image, var, backlog); 		
			}
			
			// read all pixels that failed to be assigned to a class
			cvdebug() << "proceed with backlog\n"; 
			
			while ( !backlog.empty() ) {
				C2DBounds p = backlog.front(); 
				backlog.pop(); 
				
				if ( (*result)(p) ) 
					continue; 
				
				// check whether the point is most probable of class sclass
				size_t val_p = data(p); 
				if ( val_p < pv_class.size()) {
					
					bool use_val = true; 					
					float pv_val = pv_class[val_p];
					for (size_t i = 0; i < pv.size() && use_val; ++i)
						if (i != sclass && pv[i][val_p] > pv_val)
							use_val = false;
					
					if (use_val) { // store the seed point for the next run
						(*result)(p) = true; 
						seedpoints.push(p); 
					}
				}
			} // while ( !backlog.empty() )
		} // while (!seedpoints.empty())
		return P2DImage(result); 
	}
};




template <typename  Data2D, typename V>
typename C2DWatershedFilter::result_type C2DWatershedFilter::operator () (const Data2D& data, const V& var_image) const
{
	const bool is_integral = ::boost::is_integral<typename Data2D::value_type>::value; 
	
	return dispatch_filter<Data2D, V, is_integral>::apply(data, _M_pv, _M_steep, _M_thresh, 
											   _M_tol, var_image,_M_sclass); 
}

C2DWatershedFilterImageFilter::C2DWatershedFilterImageFilter(int hwidth, float thresh, float steep, 
							     float tol, const CProbabilityVector& pv, size_t sclass):
	_M_filter(thresh, steep, tol, pv, sclass)
{
	C2DImageFilterHandler filter_plugins;
	stringstream filter_descr; 
	filter_descr << "variation:w=" << hwidth <<",float=1"; 
		
	_M_var_evaluator = C2DFilterPlugin::produce(filter_descr.str().c_str(), filter_plugins); 
	if (!_M_var_evaluator)
		throw runtime_error("unable to create variation filter"); 
}

P2DImage C2DWatershedFilterImageFilter::do_filter(const C2DImage& image) const
{
	P2DImage var_image = _M_var_evaluator->filter(image); 
	return wrap_filter(_M_filter,image, var_image); 
}

C2DWatershedFilterImageFilterFactory::C2DWatershedFilterImageFilterFactory():
	C2DFilterPlugin(plugin_name)
{
	add_help(param_width);
}

C2DFilterPlugin::ProductPtr C2DWatershedFilterImageFilterFactory::create(const CParsedOptions& options) const
{
	string map = param_map.get_value(options); 
	float thresh = param_thresh.get_value(options);
	float steep = param_steep.get_value(options);
	float tol = param_tol.get_value(options);
	int hwidth = param_width.get_value(options); 
	size_t sclass = param_class.get_value(options);
	
	CProbabilityVector pv = load_probability_map(map); 
	if (pv.empty())
		throw invalid_argument(string("Unable to load probability map from ") + map); 
	
	if (sclass < pv.size())
		return C2DFilterPlugin::ProductPtr(new C2DWatershedFilterImageFilter(hwidth, thresh, steep, tol, pv, sclass)); 
	else {
		stringstream errmsg; 
		errmsg << "selected class '" << sclass << "' out of probability map range"; 
		throw invalid_argument(errmsg.str()); 
		return C2DFilterPlugin::ProductPtr();
	}
		
}

const string C2DWatershedFilterImageFilterFactory::do_get_descr()const
{
	return "2D image watershead filter with seed"; 
}

#if 0
bool C2DWatershedFilterImageFilterFactory::do_test() const
{
	cvfail() << do_get_descr() <<": NO TEST IMPLEMENTED\n"; 
	return true; 
}
#endif

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DWatershedFilterImageFilterFactory(); 
}

} // end namespace watershed_2dimage_filter
