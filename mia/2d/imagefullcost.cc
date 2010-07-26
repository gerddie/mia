/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <mia/2d/imagefullcost.hh>
#include <mia/2d/2dfilter.hh>

NS_MIA_BEGIN

C2DImageFullCost::C2DImageFullCost(const std::string& src, 
				   const std::string& ref, 
				   const std::string& cost, 
				   EInterpolation ip_type, 
				   double weight):
	C2DFullCost(weight), 
	_M_src_key(C2DImageIOPluginHandler::instance().load_to_pool(src)), 
	_M_ref_key(C2DImageIOPluginHandler::instance().load_to_pool(ref)), 
	_M_cost_kernel(C2DImageCostPluginHandler::instance().produce(cost)), 
	_M_ipf(create_2dinterpolation_factory(ip_type))
{
	assert(_M_cost_kernel); 
}

double C2DImageFullCost::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	assert(_M_src); 
	assert(_M_ref); 


	P2DImage temp  = t(*_M_src, *_M_ipf);
	
	C2DFVectorfield force(get_current_size()); 
 	_M_cost_kernel->evaluate_force(*temp, *_M_ref, 1.0, force); 
	t.translate(force, gradient); 
	return _M_cost_kernel->value(*temp, *_M_ref); 
	
}

void C2DImageFullCost::do_set_size()
{
	_M_src = get_from_pool(_M_src_key); 
	_M_ref = get_from_pool(_M_ref_key); 
	
	if (_M_src->get_size() != _M_ref->get_size()) 
		throw runtime_error("C2DImageFullCost only works with images of equal size"); 

	if (_M_src->get_size() != get_current_size()) {
		stringstream filter_descr; 
		filter_descr << "scale:sx=" << get_current_size().x << ",sy=" << get_current_size().y; 
		auto scaler = C2DFilterPluginHandler::instance().produce(filter_descr.str()); 
		assert(scaler); 
		cvdebug() << "C2DImageFullCost:scale images to " << get_current_size() << 
			" using '" << filter_descr.str() << "'\n"; 
		_M_src = scaler->filter(*_M_src); 
		_M_ref = scaler->filter(*_M_ref); 
	}
	_M_cost_kernel->prepare_reference(*_M_ref); 
}

P2DImage C2DImageFullCost::get_from_pool(const C2DImageDataKey& key)
{
	C2DImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C2DImageFullCost: no image available in data pool");

	return (*in_image_list)[0];
}


NS_MIA_END
