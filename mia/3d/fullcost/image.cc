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

#include <boost/lambda/lambda.hpp>
#include <mia/3d/fullcost/image.hh>
#include <mia/3d/3dfilter.hh>

NS_MIA_BEGIN

using boost::lambda::_1; 

C3DImageFullCost::C3DImageFullCost(const std::string& src, 
				   const std::string& ref, 
				   const std::string& cost, 
				   EInterpolation ip_type, 
				   double weight, 
				   bool debug):
	C3DFullCost(weight), 
	_M_src_key(C3DImageIOPluginHandler::instance().load_to_pool(src)), 
	_M_ref_key(C3DImageIOPluginHandler::instance().load_to_pool(ref)), 
	_M_cost_kernel(C3DImageCostPluginHandler::instance().produce(cost)), 
	_M_ipf(create_3dinterpolation_factory(ip_type)), 
	_M_debug(debug)
{
	assert(_M_cost_kernel); 
}

bool C3DImageFullCost::do_has(const char *property) const
{
	return _M_cost_kernel->has(property); 
}

double C3DImageFullCost::do_value(const C3DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 
	P3DImage temp  = t(*_M_src, *_M_ipf);
	const double result = _M_cost_kernel->value(*temp, *_M_ref); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C3DImageFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 
	const double result = _M_cost_kernel->value(*_M_src, *_M_ref); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}


double C3DImageFullCost::do_evaluate(const C3DTransformation& t, gsl::DoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 
	
	static int idx = 0; 
	static auto  toubyte_converter = 
		C3DFilterPluginHandler::instance().produce("convert:repn=ubyte"); 
	P3DImage temp  = t(*_M_src, *_M_ipf);

	if (_M_debug) {
		stringstream fname; 
		fname << "test" << setw(5) << setfill('0') << idx << ".v"; 
		save_image(fname.str(), temp); 
	}
	
	C3DFVectorfield force(get_current_size()); 

 	_M_cost_kernel->evaluate_force(*temp, *_M_ref, 1.0, force); 

	t.translate(force, gradient); 
	idx++;

	double result = _M_cost_kernel->value(*temp, *_M_ref); 
	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C3DImageFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 

	if (_M_src->get_size() != get_current_size()) {
		stringstream filter_descr; 
		filter_descr << "scale:sx=" << get_current_size().x << ",sy=" << get_current_size().y; 
		auto scaler = C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
		assert(scaler); 
		cvdebug() << "C3DImageFullCost:scale images to " << get_current_size() << 
			" using '" << filter_descr.str() << "'\n"; 
		_M_src = scaler->filter(*_M_src); 
		_M_ref = scaler->filter(*_M_ref); 
		_M_cost_kernel->prepare_reference(*_M_ref); 
	}
}

void C3DImageFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	_M_src = get_from_pool(_M_src_key);
	_M_ref = get_from_pool(_M_ref_key);
	if (_M_src->get_size() != _M_ref->get_size()) 
		throw runtime_error("C3DImageFullCost only works with images of equal size"); 
	_M_cost_kernel->prepare_reference(*_M_ref); 
}

P3DImage C3DImageFullCost::get_from_pool(const C3DImageDataKey& key)
{
	C3DImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DImageFullCost: no image available in data pool");

	return (*in_image_list)[0];
}


// plugin implementation 
class C3DImageFullCostPlugin: public C3DFullCostPlugin {
public: 
	C3DImageFullCostPlugin(); 
private: 
	C3DFullCostPlugin::ProductPtr do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string _M_src_name;
	std::string _M_ref_name;
	std::string _M_cost_kernel;
	EInterpolation _M_interpolator;
	bool _M_debug; 
}; 

C3DImageFullCostPlugin::C3DImageFullCostPlugin():
	C3DFullCostPlugin("image"), 
	_M_src_name("src.@"), 
	_M_ref_name("ref.@"), 
	_M_cost_kernel("ssd"), 
	_M_interpolator(ip_bspline3), 
	_M_debug(false)
{
	add_parameter("src", new CStringParameter(_M_src_name, false, "Study image"));
	add_parameter("ref", new CStringParameter(_M_ref_name, false, "Reference image"));
	add_parameter("cost", new CStringParameter(_M_cost_kernel, false, "Cost function kernel"));
	add_parameter("interp", new CDictParameter<EInterpolation>(_M_interpolator, 
								   GInterpolatorTable, "image interpolator"));
	add_parameter("debug", new CBoolParameter(_M_debug, false, "Save intermediate resuts for debugging")); 
}

C3DFullCostPlugin::ProductPtr C3DImageFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C3DImageFullCostPlugin with weight= " << weight 
		  << " src=" << _M_src_name << " ref=" << _M_ref_name 
		  << " cost=" << _M_cost_kernel << "\n";

	return C3DFullCostPlugin::ProductPtr(
		new C3DImageFullCost(_M_src_name, _M_ref_name, 
				     _M_cost_kernel, _M_interpolator, weight, _M_debug)); 
}

const std::string C3DImageFullCostPlugin::do_get_descr() const
{
	return "image similarity cost function"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DImageFullCostPlugin();
}

NS_MIA_END
