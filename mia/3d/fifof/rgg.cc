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

/*! \brief basic type of a plugin handler 

A gradient based region growing filter seeded based on a fuzzy c-means classification. 

\author Gert Wollny <wollny at eva.mpg.de>

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <queue>
#include <iomanip>
#include <limits>

#include <boost/type_traits.hpp>
#include <boost/lambda/lambda.hpp>

NS_MIA_BEGIN;

using namespace std; 
using namespace boost::lambda;


NAMESPACE_BEGIN(rgg_2dstack_filter)

struct PvSeeder {
	PvSeeder(const CProbabilityVector& p, float seed, int cls); 
	
	template <typename T> 
	bool operator()(T x) const; 


}; 

struct C2DRGGStackFilterImpl {
	C2DRGGStackFilterImpl(float seed, const CProbabilityVector& pv, 
			      size_t depth, float gt); 
	void do_initialize(::boost::call_traits<mia::P2DImage>::param_type x); 
	void do_push(::boost::call_traits<mia::P2DImage>::param_type x); 
	P2DImage do_filter(); 
	void post_finalize(); 
	void shift_buffer(); 

	template <typename T> 
	int operator () ( const T2DImage<T>& x); 
private: 
	
	float _M_seed; 
	const CProbabilityVector pv; 
	size_t _M_depth; 
	float _M_gt; 
	
	vector<C3DUBImage>  _M_region_buffer;  
	vector<P2DImage>    _M_image_buffer; 
}; 

void C2DRGGStackFilterImpl::shift_buffer()
{
	copy_backward(_M_region_buffer->begin(), 
		      _M_region_buffer->end() - _M_region_buffer->get_size().x * _M_region_buffer->get_size().y , 
		      _M_region_buffer->end());
	copy_backward(_M_image_buffer->begin(), 
		      _M_image_buffer->end() - _M_image_buffer->get_size().x * _M_image_buffer->get_size().y , 
		      _M_image_buffer->end());
}


C2DRGGStackFilterImpl::C2DRGGStackFilterImpl(float seed, const CProbabilityVector& pv, 
					     size_t depth, float gt):
	C2DImageFifoFilter(1, depth, 0)
{
}

void C2DRGGStackFilterImpl::do_initialize(::boost::call_traits<mia::P2DImage>::param_type x)
{
}

template <typename T> 
int C2DRGGStackFilterImpl::operator () ( const T2DImage<T>& x)
{
	C2DUBImage mask(x.get_size(), x.get_attribute_list()); 
	transform(x.begin(), x.end(), mask.begin(), _M_pvseed); 
}


void C2DRGGStackFilterImpl::do_push(::boost::call_traits<mia::P2DImage>::param_type x)
{
	_M_image_buffer.push_back(x); 
	if (_M_image_buffer.size() > depth) 
		_M_image_buffer.pop_font(); 
	
	// run the region growing 
	mia::accumulate(*this, x); 
}

P2DImage C2DRGGStackFilterImpl::do_filter()
{
}

void C2DRGGStackFilterImpl::post_finalize()
{
}

C2DRGGStackFilter::C2DRGGStackFilter(float seed, const CProbabilityVector& pv, 
				     size_t depth, float gt):
	_M_impl(new C2DRGGStackFilterImpl(seed, pv, depth, gt)
{
}


void C2DRGGStackFilter::do_initialize(::boost::call_traits<mia::P2DImage>::param_type x)
{
	_M_impl->do_initialize(x); 
}

void C2DRGGStackFilter::do_push(::boost::call_traits<mia::P2DImage>::param_type x)
{
	_M_impl->do_push(x);
}

P2DImage C2DRGGStackFilter::do_filter()
{
	return _M_impl->do_filter(); 
}

void C2DRGGStackFilter::post_finalize()
{
	_M_impl->post_finalize(); 
}

class C2DRGGStackFilterFactory : public C2DFifoFilterPlugin {
public: 
	C2DRGGStackFilterFactory(); 
private: 
	
	virtual const string do_get_descr() const;
	virtual bool do_test() const; 
	virtual C2DFifoFilterPlugin::ProductPtr do_create()const;

	string _M_seed_map; 
	float  _M_seed_thresh; 
	int    _M_depth; 
	float  _M_gradient_thresh; 
}; 


C2DRGGStackFilterFactory::C2DRGGStackFilterFactory():
	C2DFifoFilterPlugin("regiongrow"), 
	_M_seed_thresh(0.9), 
	_M_depth(5), 
	_M_gradient_thresh(4.0)
{
	add_parameter("map", new CStringParameter(_M_seed_map, true, "class probability map"));
	add_parameter("st", new CFloatParameter(_M_seed_thresh, 0.0, 1.0, 
					      false, "seed probability threshhold"));
	add_parameter("depth", new CIntParameter(_M_depth, 1, 30, 
					      false, "number of slices to keep during processing"));
	add_parameter("gt", new CFloatParameter(_M_gradient_thresh, 4.0, 
						numeric_limits<float>::max(), 
						false, "gradient threshhold"));
}
 

const string C2DRGGStackFilterFactory::do_get_descr() const
{
	return "Gradiend based region growing 2D fifo filter"; 
}
bool C2DRGGStackFilterFactory::do_test() const
{
	return true; 
}

C2DFifoFilterPlugin::ProductPtr C2DRGGStackFilterFactory::do_create()const
{
	CProbabilityVector pv(_M_seed_map);
	return C2DFifoFilterPlugin::ProductPtr(
             new C2DRGGStackFilter(_M_seed_thresh, pv, 
				   _M_depth, _M_gradient_thresh)); 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DRGGStackFilterFactory(); 
}

NAMESPACE_END // end namespace regiongrow_2dstack_filter
