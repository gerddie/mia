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

#include <mia/3d/fullcost/image.hh>
#include <mia/3d/3dfilter.hh>

NS_MIA_BEGIN

using namespace std; 

C3DImageFullCost::C3DImageFullCost(const std::string& src, 
				   const std::string& ref, 
				   const std::string& cost, 
				   double weight, 
				   bool debug):
	C3DFullCost(weight), 
	m_src_key(C3DImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C3DImageIOPluginHandler::instance().load_to_pool(ref)), 
	m_cost_kernel(C3DImageCostPluginHandler::instance().produce(cost)), 
	m_debug(debug)
{
	assert(m_cost_kernel); 
}

bool C3DImageFullCost::do_has(const char *property) const
{
	return m_cost_kernel->has(property); 
}

double C3DImageFullCost::do_value(const C3DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	P3DImage temp  = t(*m_src_scaled);
	const double result = m_cost_kernel->value(*temp); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C3DImageFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	const double result = m_cost_kernel->value(*m_src_scaled); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}


double C3DImageFullCost::do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	
	P3DImage temp  = t(*m_src_scaled);
	C3DFVectorfield force(get_current_size()); 
 	m_cost_kernel->evaluate_force(*temp, 1.0, force); 
	t.translate(force, gradient); 
	double result = m_cost_kernel->value(*temp); 
	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C3DImageFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src); 
	assert(m_ref); 

	if (!m_src_scaled || m_src_scaled->get_size() != get_current_size() ||
	    !m_ref_scaled || m_ref_scaled->get_size() != get_current_size())
	{
		if (m_src->get_size() == get_current_size()) {
			m_src_scaled = m_src;
			m_ref_scaled = m_ref;
		}else{
			stringstream filter_descr; 
			filter_descr << "scale:s=[" << get_current_size()<<"]"; 
			auto scaler = C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
			assert(scaler); 
			cvdebug() << "C3DImageFullCost:scale images to " << get_current_size() << 
				" using '" << filter_descr.str() << "'\n"; 
			m_src_scaled = scaler->filter(*m_src); 
			m_ref_scaled = scaler->filter(*m_ref);
		}
		m_cost_kernel->set_reference(*m_ref_scaled); 
	}
}

bool C3DImageFullCost::do_get_full_size(C3DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	if (size == C3DBounds::_0) {
		size = m_src->get_size(); 
		return true; 
	}else
		return 	size == m_src->get_size(); 
}

void C3DImageFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	//cvmsg() << "C3DImageFullCost: read " << m_src_key << " and " << m_ref_key << "\n"; 
	m_src = get_from_pool(m_src_key);
	m_ref = get_from_pool(m_ref_key);
	m_src_scaled.reset(); 
	m_ref_scaled.reset(); 

	if (m_src->get_size() != m_ref->get_size()) 
		throw runtime_error("C3DImageFullCost only works with images of equal size"); 

	if (m_src->get_voxel_size() != m_ref->get_voxel_size()) {
		cverr() << "C3DImageFullCost: src and reference image are of differnet pixel dimensions."
			<< "This code doesn't honour this and a proper scaling should be applied first."; 
	}
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
	C3DFullCost *do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string m_src_name;
	std::string m_ref_name;
	std::string m_cost_kernel;
	bool m_debug; 
}; 

C3DImageFullCostPlugin::C3DImageFullCostPlugin():
	C3DFullCostPlugin("image"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
	m_cost_kernel("ssd"), 
	m_debug(false)
{
	add_parameter("src", new CStringParameter(m_src_name, false, "Study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, false, "Reference image"));
	add_parameter("cost", new CStringParameter(m_cost_kernel, false, "Cost function kernel"));
	add_parameter("debug", new CBoolParameter(m_debug, false, "Save intermediate resuts for debugging")); 
}

C3DFullCost *C3DImageFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C3DImageFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
		  << " cost=" << m_cost_kernel << "\n";

	return new C3DImageFullCost(m_src_name, m_ref_name, m_cost_kernel, weight, m_debug); 
}

const std::string C3DImageFullCostPlugin::do_get_descr() const
{
	return "Generalized image similarity cost function that also handles multi-resolution processing. "
		"The actual similarity measure is given es extra parameter.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DImageFullCostPlugin();
}

NS_MIA_END
