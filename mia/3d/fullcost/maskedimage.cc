/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
 *
 * MIA is free software; you can redistribute it and/or modify
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
 * along with MIA; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <mia/3d/fullcost/maskedimage.hh>
#include <mia/3d/filter.hh>

NS_MIA_BEGIN

using namespace std; 

C3DMaskedImageFullCost::C3DMaskedImageFullCost(const std::string& src, 
				   const std::string& ref, 
                                   const std::string& src_mask, 
				   const std::string& ref_mask, 
				   P3DImageCost cost, 
				   double weight):
	C3DFullCost(weight), 
	m_src_key(C3DMaskedImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C3DMaskedImageIOPluginHandler::instance().load_to_pool(ref)),
        m_src_mask_bit(nullptr), 
        m_ref_mask_bit(nullptr), 
	m_cost_kernel(cost)
{
	assert(m_cost_kernel); 

        if (!src_mask.empty()) 
                m_src_mask_key = C3DMaskedImageIOPluginHandler::instance().load_to_pool(src_mask); 
        
        if (!ref_mask.empty()) 
                m_ref_mask_key = C3DMaskedImageIOPluginHandler::instance().load_to_pool(ref_mask); 

        if (src_mask.empty() && ref_mask.empty()) {
                throw invalid_argument("C3DMaskedImageFullCost: No masks give, you should used "
                                       "the plain image cost instead"); 
        }
}

bool C3DMaskedImageFullCost::do_has(const char *property) const
{
	return m_cost_kernel->has(property); 
}

double C3DMaskedImageFullCost::do_value(const C3DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled  && "Hint: call 'reinit()' before calling value(transform)"); 
	P3DImage temp  = t(*m_src_scaled);
	const double result = m_cost_kernel->value(*temp); 
	cvdebug() << "C3DMaskedImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C3DMaskedImageFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled  && "Hint: call 'reinit()' before calling value()"); 
	const double result = m_cost_kernel->value(*m_src_scaled); 
	cvdebug() << "C3DMaskedImageFullCost::value = " << result << "\n"; 
	return result; 
}


double C3DMaskedImageFullCost::do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled  && "Hint: call 'reinit()' before calling evaluate()"); 
	
	P3DImage temp  = t(*m_src_scaled);
        
        P3DImage temp_mask; 
        
        C3DBitImage *temp_mask_bit = m_ref_mask_scaled_bit; 
        
        if (m_src_mask_scaled) {
                temp_mask = t(*m_src_mask_scaled); 
                temp_mask_bit = static_cast<C3DBitImage *>(temp_mask.get());
                
                if (m_ref_mask_scaled_bit) {
                        transform(temp_mask_bit.begin(), temp_mask_bit.end(), 
                                  m_ref_mask_scaled_bit.begin(), temp_mask_bit.begin(), 
                                  [](bool a, bool b){ return a && b;}); 

                        // here a penalty could be added to ensure, e.g. that the moving mask is 
                        // always inside the fixed mask. However, it is difficult to evaluate a 
                        // force for this.
                }
        }
        assert(temp_mask_bit); 
        
	C3DFVectorfield force(get_current_size()); 
 	m_cost_kernel->evaluate_force(*temp, *temp_mask_bit, force); 
	t.translate(force, gradient); 
	double result = m_cost_kernel->value(*temp); 
	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C3DMaskedImageFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src); 
	assert(m_ref); 

	if (m_src_scaled->get_size() != get_current_size() ||
	    m_ref_scaled->get_size() != get_current_size())
	{
		stringstream filter_descr; 
		filter_descr << "scale:s=[" << get_current_size()<<"]"; 
		auto scaler = C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
		assert(scaler); 
		cvdebug() << "C3DMaskedImageFullCost:scale images to " << get_current_size() << 
			" using '" << filter_descr.str() << "'\n"; 
		m_src_scaled = scaler->filter(*m_src); 
		m_ref_scaled = scaler->filter(*m_ref);
		m_cost_kernel->set_reference(*m_ref_scaled); 

                if (m_src_mask_bit != nullptr)  {
                    m_src_mask_scaled  = scaler->filter(*m_src_mask);
                    m_src_mask_scaled_bit = static_cast<C3DBitImage*>(m_src_mask_scaled);  
                }

                if (m_ref_mask_bit != nullptr)  {
                    m_ref_mask_scaled  = scaler->filter(*m_ref_mask);
                    m_ref_mask_scaled_bit = static_cast<C3DBitImage*>(m_ref_mask_scaled);  
                }
                assert (m_ref_mask_scaled_bit || m_src_mask_scaled_bit); 
	}
}

bool C3DMaskedImageFullCost::do_get_full_size(C3DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_src && "Hint: call 'reinit()' before calling get_full_size()"); 
	if (size == C3DBounds::_0) {
		size = m_src->get_size(); 
		return true; 
	}else
		return 	size == m_src->get_size(); 
}

void C3DMaskedImageFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	//cvmsg() << "C3DMaskedImageFullCost: read " << m_src_key << " and " << m_ref_key << "\n"; 
	m_src_scaled = m_src = get_from_pool(m_src_key);
	m_ref_scaled = m_ref = get_from_pool(m_ref_key);

        if (m_src_mask_key.valid()) {
                m_src_mask = get_from_pool(m_src_mask_key);
                if (m_src->get_size() != m_src_mask->get_size()) {
                        throw create_exception<runtime_error>("C3DMaskedImageFullCost: moving image has size [", 
                                                              m_src->get_size(), "], but corresponding mask is of size ["
                                                              m_src_mask->get_size(), "]"); 
                }
                m_src_mask_scaled_bit = m_src_mask_bit = dynamic_cast<C3DBitImage *>(m_src_mask.get()); 
                if (!m_src_mask_scaled_bit)  {
                        throw create_exception<invalid_argument>("C3DMaskedImageFullCost: moving mask image "
                                                                 "must be binary"); 
                }
        }

        if (m_ref_mask_key.valid()) {
                m_ref_masked = get_from_pool(m_ref_mask_key);
                if (m_ref->get_size() != m_ref_mask->get_size()) {
                        throw create_exception<runtime_error>("C3DMaskedImageFullCost: reference image has size [", 
                                                              m_src->get_size(), "], but corresponding mask is of size ["
                                                              m_src_mask->get_size(), "]"); 
                }
                m_ref_mask_scaled_bit = m_ref_mask_bit = dynamic_cast<C3DBitImage *>(m_ref_mask.get());
                if (!m_ref_mask_scaled_bit)  {
                        throw create_exception<invalid_argument>("C3DMaskedImageFullCost: reference mask image "
                                                                 "must be binary");
                }
        }

	if (m_src->get_size() != m_ref->get_size()) {
                cvinfo() << "C3DMaskedImageFullCost: registering moving image of size [" << m_src->get_size() << "] " 
                         << "to reference image of size [" << m_ref->get_size() << "]\n"; 
        }

        

	if (m_src->get_voxel_size() != m_ref->get_voxel_size()) {
		cvwarn() << "C3DMaskedImageFullCost: moving and reference image are of differnet pixel dimensions."
                         << "unless you optimize a transformation that supports global scaling this might "
                         << "not be what you want to do\n"; 
	}
	m_cost_kernel->set_reference(*m_ref_scaled); 
}

P3DImage C3DMaskedImageFullCost::get_from_pool(const C3DMaskedImageDataKey& key)
{
	C3DMaskedImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DMaskedImageFullCost: no image available in data pool");

	return (*in_image_list)[0];
}


// plugin implementation 
class C3DMaskedImageFullCostPlugin: public C3DFullCostPlugin {
public: 
	C3DMaskedImageFullCostPlugin(); 
private: 
	C3DFullCost *do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string m_src_name;
	std::string m_ref_name;
	std::string m_src_mask_name;
	std::string m_ref_mask_name;

	P3DImageCost m_cost_kernel;
}; 

C3DMaskedImageFullCostPlugin::C3DMaskedImageFullCostPlugin():
	C3DFullCostPlugin("image"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
        m_debug(false)
{
	add_parameter("src", new CStringParameter(m_src_name, CCmdOptionFlags::input, "Study image", &C3DImageIOPluginHandler::instance()));
	add_parameter("ref", new CStringParameter(m_ref_name, CCmdOptionFlags::input, "Reference image", &C3DImageIOPluginHandler::instance()));

	add_parameter("src-mask", new CStringParameter(m_src_mask_name, CCmdOptionFlags::input, 
                                                       "Study image mask (binary)", &C3DImageIOPluginHandler::instance()));
	add_parameter("ref-mask", new CStringParameter(m_ref_mask_name, CCmdOptionFlags::input, 
                                                       "Reference image mask  (binary)", &C3DImageIOPluginHandler::instance()));

	add_parameter("cost", make_param(m_cost_kernel, "ssd", false, "Cost function kernel"));
}

C3DFullCost *C3DMaskedImageFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C3DMaskedImageFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
                  << " src-mask='" << m_src_mask_name << "' ref='" << m_ref_name 
		  << "' cost=" << m_cost_kernel << "\n";

	return new C3DMaskedImageFullCost(m_src_name, m_ref_name, 
                                          m_src_mask_name, m_ref_mask_name, 
                                          m_cost_kernel, weight); 
}

const std::string C3DMaskedImageFullCostPlugin::do_get_descr() const
{
	return "Generalized masked image similarity cost function that also handles multi-resolution processing. "
                "The provided masks should be densly filled regions in multi-resolution procesing because "
                "otherwise the mask information may get lost when downscaling the image. " 
                "The reference mask and the transformed mask of the study image are combined by binary AND. 
		"The actual similarity measure is given es extra parameter.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DMaskedImageFullCostPlugin();
}

NS_MIA_END
