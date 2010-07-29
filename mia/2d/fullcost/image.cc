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
#include <mia/2d/fullcost/image.hh>
#include <mia/2d/2dfilter.hh>

NS_MIA_BEGIN

using boost::lambda::_1; 

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

bool C2DImageFullCost::do_has(const char *property) const
{
	return _M_cost_kernel->has(property); 
}

double C2DImageFullCost::do_value(const C2DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 
	P2DImage temp  = t(*_M_src, *_M_ipf);
	const double result = _M_cost_kernel->value(*temp, *_M_ref); 
	cvdebug() << "C2DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C2DImageFullCost::do_evaluate(const C2DTransformation& t, gsl::DoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(_M_src); 
	assert(_M_ref); 
	
	static int idx = 0; 
	static auto  toubyte_converter = C2DFilterPluginHandler::instance().produce("convert:repn=ubyte"); 
	P2DImage temp  = t(*_M_src, *_M_ipf);

	stringstream fname; 
	fname << "test" << setw(5) << setfill('0') << idx << ".png"; 
	save_image2d(fname.str(), toubyte_converter->filter(*temp)); 


	double maxnorm = 0.0; 
	auto transfparame = t.get_parameters(); 
	for(auto i = transfparame.begin();i  != transfparame.end(); i+=2) {
		double n = i[0] * i[0] + i[1] * i[1];
		if (maxnorm  < n)
			maxnorm = n;
	}
	cvinfo() << "maxnorm=" <<maxnorm<< "\n"; 

	if (maxnorm > 0.0) {
		C2DFImage *tn = new C2DFImage(t.get_size()); 
		P2DImage tnorm(tn); 
		for (auto g = tn->begin(), i = transfparame.begin(); g != tn->end(); ++g, i+=2)
			*g = sqrt(i[0] * i[0] + i[1] * i[1]); 
		stringstream nname; 
		nname << "tnorm" << setw(5) << setfill('0') << idx << ".v"; 
		save_image2d(nname.str(), tnorm); 
	}	
	
	C2DFVectorfield force(get_current_size()); 
 	_M_cost_kernel->evaluate_force(*temp, *_M_ref, 1.0, force); 

	
	cvinfo() << get_current_size() << " gsize " << gradient.size() << "\n"; 
	
	t.translate(force, gradient); 


	idx++; 	

	transform(gradient.begin(), gradient.end(), gradient.begin(), -1 * _1); 
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


// plugin implementation 
class C2DImageFullCostPlugin: public C2DFullCostPlugin {
public: 
	C2DImageFullCostPlugin(); 
private: 
	C2DFullCostPlugin::ProductPtr do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string _M_src_name;
	std::string _M_ref_name;
	std::string _M_cost_kernel;
	EInterpolation _M_interpolator;
}; 

C2DImageFullCostPlugin::C2DImageFullCostPlugin():
	C2DFullCostPlugin("image"), 
	_M_src_name("src.@"), 
	_M_ref_name("ref.@"), 
	_M_cost_kernel("ssd"), 
	_M_interpolator(ip_bspline3)
{
	add_parameter("src", new CStringParameter(_M_src_name, false, "Study image"));
	add_parameter("ref", new CStringParameter(_M_ref_name, false, "Reference image"));
	add_parameter("cost", new CStringParameter(_M_cost_kernel, false, "Cost function kernel"));
	add_parameter("interp", new CDictParameter<EInterpolation>(_M_interpolator, 
								   GInterpolatorTable, "image interpolator"));
}

C2DFullCostPlugin::ProductPtr C2DImageFullCostPlugin::do_create(float weight) const
{
	return C2DFullCostPlugin::ProductPtr(
		new C2DImageFullCost(_M_src_name, _M_ref_name, 
				     _M_cost_kernel, _M_interpolator, weight)); 
}

const std::string C2DImageFullCostPlugin::do_get_descr() const
{
	return "image similarity cost function"; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DImageFullCostPlugin();
}

NS_MIA_END
