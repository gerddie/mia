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

/* 
   LatexBeginPluginDescription{2D full cost functions}

      
   \subsection{Image cost function}
   \label{fullcost2d:image}
   
   \begin{description}
   
   \item [Plugin:] image
   \item [Description:] Evaluate the image similarity measure of a cost function 

  
   \plugtabstart
   weight &  float & all-over weight of the cost function & 1.0  \\
   src & string & the study (or floating) image & src.@ \\
   ref & string & the reference (or fixed) image & ref.@ \\
   cost & string & Description of the image similarity measure kernel (\ref{sec:cost2d} & ssd \\
   interp & string & Image interpolator kernel & bspline3 \\
   debug & bool & Save intermediate resuts for debugging (creates a lot of data)  & false \\
   \plugtabend
   
   \item [Remark:] The default parameters for the src and ref image are used for passing 
                   the data within the program. Don't change these parameters unless you know
		   what you are doing.  
   \end{description}

   LatexEnd
*/


#include <mia/2d/fullcost/image.hh>
#include <mia/2d/2dfilter.hh>

NS_MIA_BEGIN

C2DImageFullCost::C2DImageFullCost(const std::string& src, 
				   const std::string& ref, 
				   C2DImageCostPluginHandler::ProductPtr cost, 
				   double weight, 
				   bool debug):
	C2DFullCost(weight), 
	m_src_key(C2DImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C2DImageIOPluginHandler::instance().load_to_pool(ref)), 
	m_cost_kernel(cost), 
	m_debug(debug)
{
	assert(m_cost_kernel); 
}

bool C2DImageFullCost::do_has(const char *property) const
{
	return m_cost_kernel->has(property); 
}

bool C2DImageFullCost::do_get_full_size(C2DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	if (size == C2DBounds::_0) {
		size = m_src->get_size(); 
		return true; 
	}else
		return 	size == m_src->get_size(); 
}


double C2DImageFullCost::do_value(const C2DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	P2DImage temp  = t(*m_src_scaled);
	const double result = m_cost_kernel->value(*temp); 
	cvdebug() << "C2DImageFullCost::value = " << result << "\n"; 
	return result; 
}

double C2DImageFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	// one should apply an identity transform here, to ensure that the test image is 
	// of the same size like the reference image
	const double result = m_cost_kernel->value(*m_src_scaled); 
	cvdebug() << "C2DImageFullCost::value = " << result << "\n"; 
	return result; 
}


double C2DImageFullCost::do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled); 
	
	static int idx = 0; 
	static auto  toubyte_converter = 
		C2DFilterPluginHandler::instance().produce("convert:repn=ubyte"); 
	P2DImage temp  = t(*m_src_scaled);

	if (m_debug) {
		stringstream fname; 
		fname << "test" << setw(5) << setfill('0') << idx << ".png"; 
		save_image(fname.str(), toubyte_converter->filter(*temp)); 
		
		stringstream rname; 
		rname << "ref" << setw(5) << setfill('0') << idx << ".png"; 
		save_image(rname.str(), toubyte_converter->filter(*m_ref_scaled)); 

	}
	
	C2DFVectorfield force(get_current_size()); 

 	double result = m_cost_kernel->evaluate_force(*temp, 1.0, force); 

	t.translate(force, gradient); 
	idx++;

	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C2DImageFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src); 
	assert(m_ref); 

	if (!m_src_scaled || m_src_scaled->get_size() != get_current_size() ||
	    !m_ref_scaled || m_ref_scaled->get_size() != get_current_size() ) {
		if (get_current_size() == m_src->get_size()) {
			m_src_scaled = m_src; 
			m_ref_scaled = m_ref; 
		}else{
			stringstream filter_descr; 
			filter_descr << "scale:s=[" << get_current_size()<<"]"; 
			auto scaler = C2DFilterPluginHandler::instance().produce(filter_descr.str()); 
			assert(scaler); 
			cvdebug() << "C2DImageFullCost:scale images to " << get_current_size() << 
				" using '" << filter_descr.str() << "'\n"; 
			m_src_scaled = scaler->filter(*m_src); 
			m_ref_scaled = scaler->filter(*m_ref); 
		}
		m_cost_kernel->set_reference(*m_ref_scaled); 
	}

}

void C2DImageFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	m_src = get_from_pool(m_src_key);
	m_ref = get_from_pool(m_ref_key);
	m_src_scaled.reset(); 
	m_ref_scaled.reset(); 

	// is this true? Actually the deformed image is used and it is always interpolated on the full 
	// space of the reference image 
	if (m_src->get_size() != m_ref->get_size()) 
		throw runtime_error("C2DImageFullCost only works with images of equal size"); 
	
	if (m_src->get_pixel_size() != m_ref->get_pixel_size()) {
		cverr() << "C2DImageFullCost: src and reference image are of differnet pixel dimensions."
			<< "This code doesn't honour this and linear registration should be applied first."; 
	}
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
	C2DFullCost *do_create(float weight) const;
	const std::string do_get_descr() const;
	std::string m_src_name;
	std::string m_ref_name;
	C2DImageCostPluginHandler::ProductPtr m_cost_kernel;
	bool m_debug; 
}; 

C2DImageFullCostPlugin::C2DImageFullCostPlugin():
	C2DFullCostPlugin("image"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
	m_debug(false)
{
	add_parameter("src", new CStringParameter(m_src_name, false, "Study image"));
	add_parameter("ref", new CStringParameter(m_ref_name, false, "Reference image"));
	add_parameter("cost", new CFactoryParameter<C2DImageCostPluginHandler>(m_cost_kernel, "ssd", false, "Cost function kernel, see PLUGINS:cost/2d"));
	add_parameter("debug", new CBoolParameter(m_debug, false, "Save intermediate resuts for debugging")); 
}

C2DFullCost *C2DImageFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C2DImageFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
		  << " cost=" << m_cost_kernel << "\n";
	return 	new C2DImageFullCost(m_src_name, m_ref_name, 
				     m_cost_kernel, weight, m_debug); 
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
