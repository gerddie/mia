/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2010-2011
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

/* 
   LatexBeginPluginDescription{3D full cost functions}

      
   \subsection{Image cost function}
   \label{fullcost3d:image}
   
   \begin{description}
   
   \item [Plugin:] image
   \item [Description:] Evaluate the image similarity measure of a cost function 

  
   \plugtabstart
   weight &  float & all-over weight of the cost function & 1.0  \\
   src & string & the study (or floating) image & src.@ \\
   ref & string & the reference (or fixed) image & ref.@ \\
   cost & string & Description of the image similarity measure kernel (\ref{sec:cost3d} & ssd \\
   interp & string & Image interpolator kernel & bspline3 \\
   debug & bool & Save intermediate resuts for debugging (creates a lot of data)  & false \\
   \plugtabend
   
   \item [Remark:] The default parameters for the src and ref image are used for passing 
                   the data within the program. Don't change these parameters unless you know
		   what you are doing.  
   \end{description}

   LatexEnd
*/

#include <boost/lambda/lambda.hpp>
#include <mia/3d/fullcost/image.hh>
#include <mia/3d/3dfilter.hh>

NS_MIA_BEGIN

C3DImageFullCost::C3DImageFullCost(const std::string& src, 
				   const std::string& ref, 
				   const std::string& cost, 
				   EInterpolation ip_type, 
				   double weight, 
				   bool debug):
	C3DFullCost(weight), 
	m_src_key(C3DImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C3DImageIOPluginHandler::instance().load_to_pool(ref)), 
	m_cost_kernel(C3DImageCostPluginHandler::instance().produce(cost)), 
	m_ipf(create_3dinterpolation_factory(ip_type)), 
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
	assert(m_src); 
	P3DImage temp  = t(*m_src, *m_ipf);
	const double result = m_cost_kernel->value(*temp); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C3DImageFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	const double result = m_cost_kernel->value(*m_src); 
	cvdebug() << "C3DImageFullCost::value = " << result << "\n"; 
	return result; 
}


double C3DImageFullCost::do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	
	static int idx = 0; 
	static auto  toubyte_converter = 
		C3DFilterPluginHandler::instance().produce("convert:repn=ubyte"); 
	P3DImage temp  = t(*m_src, *m_ipf);

	if (m_debug) {
		stringstream fname; 
		fname << "test" << setw(5) << setfill('0') << idx << ".v"; 
		save_image(fname.str(), temp); 
	}
	
	C3DFVectorfield force(get_current_size()); 

 	m_cost_kernel->evaluate_force(*temp, 1.0, force); 

	t.translate(force, gradient); 
	idx++;

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
			filter_descr << "scale:sx=" << get_current_size().x 
				     << ",sy=" << get_current_size().y
				     << ",sz=" << get_current_size().z; 
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

void C3DImageFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	m_src = get_from_pool(m_src_key);
	m_ref = get_from_pool(m_ref_key);
	m_src_scaled.reset(); 
	m_ref_scaled.reset(); 

	if (m_src->get_size() != m_ref->get_size()) 
		throw runtime_error("C3DImageFullCost only works with images of equal size"); 

	if (m_src->get_voxel_size() != m_ref->get_voxel_size()) {
		cverr() << "C3DImageFullCost: src and reference image are of differnet pixel dimensions."
			<< "This code doesn't honour this and linear registration should be applied first."; 
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
	C3DFullCostPlugin::ProductPtr do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string m_src_name;
	std::string m_ref_name;
	std::string m_cost_kernel;
	EInterpolation m_interpolator;
	bool m_debug; 
}; 

C3DImageFullCostPlugin::C3DImageFullCostPlugin():
	C3DFullCostPlugin("image"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
	m_cost_kernel("ssd"), 
	m_interpolator(ip_bspline3), 
	m_debug(false)
{
	add_parameter("src", new CStringParameter(m_src_name, false, "Study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, false, "Reference image"));
	add_parameter("cost", new CStringParameter(m_cost_kernel, false, "Cost function kernel"));
	add_parameter("interp", new CDictParameter<EInterpolation>(m_interpolator, 
								   GInterpolatorTable, "image interpolator"));
	add_parameter("debug", new CBoolParameter(m_debug, false, "Save intermediate resuts for debugging")); 
}

C3DFullCostPlugin::ProductPtr C3DImageFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C3DImageFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
		  << " cost=" << m_cost_kernel << "\n";

	return C3DFullCostPlugin::ProductPtr(
		new C3DImageFullCost(m_src_name, m_ref_name, 
				     m_cost_kernel, m_interpolator, weight, m_debug)); 
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
