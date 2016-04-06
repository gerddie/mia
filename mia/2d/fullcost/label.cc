/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#include <cmath>

#include <mia/2d/fullcost/label.hh>
#include <mia/2d/filter.hh>
#include <mia/core/distance.hh>
#include <mia/2d/distance.hh>

NS_MIA_BEGIN
using namespace std; 

C2DLabelFullCost::C2DLabelFullCost(const std::string& src, 
				   const std::string& ref, 
				   double weight, 
                                   int maxlabels, int debug):
	C2DFullCost(weight), 
	m_src_key(C2DImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C2DImageIOPluginHandler::instance().load_to_pool(ref)),
        m_ref_label_exists(maxlabels), 
        m_ref_distances(maxlabels), 
	m_debug(debug)
{
	add(::mia::property_gradient);
}

bool C2DLabelFullCost::do_get_full_size(C2DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	if (size == C2DBounds::_0) {
		size = m_src->get_size(); 
		return true; 
	}else
		return 	size == m_src->get_size(); 
}


double C2DLabelFullCost::do_value(const C2DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled.size()); 
                
	P2DImage temp  = t(m_src_scaled);
        const C2DUBImage& temp_ubyte = static_cast<const C2DUBImage&>(*temp); 
        
        double result = 0.0; 
        for (size_t i = 0; i < temp_ubyte.size(); ++i) {
		double v = value(i, temp_ubyte[i]);
                result += v; 
        }
        return result; 
}

double C2DLabelFullCost::do_value() const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled.size()); 
        
	// one should apply an identity transform here, to ensure that the test image is 
	// of the same size like the reference image
        
        double result = 0.0; 
        for (size_t i = 0; i < m_src_scaled.size(); ++i) {
                result += value(i, m_src_scaled[i]);
        }
	return result; 
}


double C2DLabelFullCost::do_evaluate(const C2DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled.size()); 

	
	P2DImage temp  = t(m_src_scaled);
        const C2DUBImage& temp_ubyte = static_cast<const C2DUBImage&>(*temp);

        C2DFVectorfield force(get_current_size()); 

	C2DBounds pos(0,0); 
	
	double result = value_and_gradient(0, temp_ubyte[0], force[0], pos, eb_xlow | eb_ylow);
	int idx = 1; 
	
	for (pos.x = 1; pos.x < temp_ubyte.get_size().x - 1; ++pos.x, ++idx) {
		result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_ylow);
	}
	result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_xhigh | eb_ylow);
	++idx; 
	
	for (pos.y = 1; pos.y < temp_ubyte.get_size().y-1; ++pos.y) {
		pos.x = 0; 
		result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_xlow);
		++idx; 
			
		for (pos.x = 1; pos.x < temp_ubyte.get_size().x - 1; ++pos.x, ++idx) {
			result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_none);
		}
		result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_xhigh);
		++idx; 
	}
	
	result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_xlow | eb_yhigh);
	++idx; 

	for (pos.x = 1; pos.x < temp_ubyte.get_size().x-1; ++pos.x, ++idx) {
		result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_yhigh);
	}
	result += value_and_gradient(idx, temp_ubyte[idx], force[idx], pos, eb_xhigh | eb_yhigh);
	

	// at this point one could inject a hole-filling algorithm to 
	// add forces inside of the homogen overlapping label regions 
        
	t.translate(force, gradient); 

	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C2DLabelFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src); 
	assert(m_ref); 

	if (m_src_scaled.size() || m_src_scaled.get_size() != get_current_size() ||
	    m_ref_scaled.size() || m_ref_scaled.get_size() != get_current_size() ) {
		if (get_current_size() == m_src->get_size()) {
			m_src_scaled = static_cast<const C2DUBImage&>(*m_src); 
			m_ref_scaled = static_cast<const C2DUBImage&>(*m_ref); 
		}else{
			stringstream filter_descr; 
			filter_descr << "labelscale:out-size=[" << get_current_size()<<"]"; 
			auto scaler = C2DFilterPluginHandler::instance().produce(filter_descr.str()); 
			assert(scaler); 
			cvdebug() << "C2DLabelFullCost:scale images to " << get_current_size() << 
				" using '" << filter_descr.str() << "'\n"; 
			m_src_scaled = static_cast<const C2DUBImage&>(*scaler->filter(*m_src)); 
                        m_ref_scaled = static_cast<const C2DUBImage&>(*scaler->filter(*m_ref)); 
		}
		prepare_distance_fields(m_ref_scaled); 
	}

}

void C2DLabelFullCost::prepare_distance_fields( const C2DUBImage &image )
{
        assert(image.get_pixel_type() == it_ubyte); 

	static int step = 0; 
        
        for (size_t i = 0; i < m_ref_label_exists.size(); ++i) {
                bool exist = false; 
                
                stringstream filter_descr; 
                filter_descr << "binarize:min=" << i << ",max=" << i; 
                auto bin = run_filter(image, filter_descr.str().c_str()); 
        
                const C2DBitImage& bool_bin = static_cast<const C2DBitImage&>(*bin); 
                for (auto i = bool_bin.begin(); i != bool_bin.end() && !exist; ++i) 
                        exist = *i; 
                
                m_ref_label_exists[i] = exist; 
                if (exist) {
			C2DFImage prep(bool_bin.get_size()); 
			distance_transform_prepare(bool_bin.begin(), bool_bin.end(), 
						   prep.begin(), true);
			
                        m_ref_distances[i] = distance_transform(prep); 
			transform(m_ref_distances[i].begin(), m_ref_distances[i].end(), 
				  m_ref_distances[i].begin(), [](float& x){ return sqrt(x);}); 
			
                }
        }
	if (m_debug) {
		for (size_t i = 0; i < m_ref_label_exists.size(); ++i) {
			if (m_ref_label_exists[i]) {
				stringstream ofname; 
				ofname << "dt" << setw(3) << setfill('0') << step 
				       << "_" << setw(3) << setfill('0') << i << ".v";
				save_image(ofname.str(), m_ref_distances[i]); 
			}
		}
	}
}

double C2DLabelFullCost::value(int idx, int label) const
{
        // non existent labels will be ignored, since they never can be 
        // aligned 
        return m_ref_label_exists[label] ? m_ref_distances[label][idx] : 0.0;
}

double C2DLabelFullCost::value_and_gradient(int idx, int label, C2DFVector& gradient, 
					    const C2DBounds& pos, int boundaries) const
{
        double result; 
        
        if (m_ref_label_exists[label]) {
		const auto & dref = m_ref_distances[label]; 
                result = dref[idx]; 
		if (result > 0.0) {
			if (boundaries == eb_none) 
				gradient = dref.get_gradient(idx); 
			else { // emulate repeat boundary conditions 
				switch (boundaries & eb_x) {
				case eb_xlow: 
					gradient.x = 0.5 * (dref(pos.x + 1, pos.y) - dref(pos.x, pos.y)); 
					break; 
				case eb_xhigh: 
					gradient.x = 0.5 * (dref(pos.x, pos.y) - dref(pos.x - 1, pos.y)); 
					break; 
				default:
					gradient.x = 0.5 * (dref(pos.x + 1, pos.y) - dref(pos.x - 1, pos.y)); 
				}
				
				switch (boundaries & eb_y) {
				case eb_ylow: 
					gradient.y = 0.5 * (dref(pos.x, pos.y + 1) - dref(pos.x, pos.y)); 
					break; 
				case eb_yhigh: 
					gradient.y = 0.5 * (dref(pos.x, pos.y) - dref(pos.x, pos.y - 1)); 
					break; 
				default:
					gradient.y = 0.5 * (dref(pos.x, pos.y + 1) - dref(pos.x, pos.y - 1)); 
				}
			}
		}else{
			gradient = C2DFVector::_0;
		}
	}else {
                result = 0.0; 
                gradient = C2DFVector::_0;
        }
        return result; 
}


void C2DLabelFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	m_src = get_from_pool(m_src_key);
	m_ref = get_from_pool(m_ref_key);
	m_src_scaled = C2DUBImage(); 
	m_ref_scaled = C2DUBImage(); 

	// is this true? Actually the deformed image is used and it is always interpolated on the full 
	// space of the reference image 
	if (m_src->get_size() != m_ref->get_size()) 
		throw runtime_error("C2DLabelFullCost only works with images of equal size"); 
	
	if (m_src->get_pixel_size() != m_ref->get_pixel_size()) {
		cvwarn() << "C2DLabelFullCost: src and reference image are of differnet pixel dimensions:"
			<< m_src->get_pixel_size() << " vs " << m_ref->get_pixel_size() 
			<< " This code doesn't honour this.\n"; 
	}
        if (m_src->get_pixel_type() != it_ubyte) 
                throw create_exception<runtime_error>("C2DLabelFullCost only works with (label) "
                                                      "images of pixel type ubyte, but src has type ", 
                                                      CPixelTypeDict.get_name(m_src->get_pixel_type()));

        if (m_ref->get_pixel_type() != it_ubyte) 
                throw create_exception<runtime_error>("C2DLabelFullCost only works with (label) "
                                                      "images of pixel type ubyte, but ref has type ", 
                                                      CPixelTypeDict.get_name(m_src->get_pixel_type()));
}

P2DImage C2DLabelFullCost::get_from_pool(const C2DImageDataKey& key)
{
	C2DImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C2DLabelFullCost: no image available in data pool");

	return (*in_image_list)[0];
}


C2DLabelFullCostPlugin::C2DLabelFullCostPlugin():
        C2DFullCostPlugin("labelimage"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
        m_maxlabel(256), 
	m_debug(0)
{
	add_parameter("src", new CStringParameter(m_src_name, CCmdOptionFlags::input, "Study image", 
			      &C2DImageIOPluginHandler::instance()));
	add_parameter("ref", new CStringParameter(m_ref_name, CCmdOptionFlags::input, "Reference image", 
			      &C2DImageIOPluginHandler::instance()));
	add_parameter("maxlabel", make_ci_param(m_maxlabel, 2, 32000, false, 
						 "maximum number of labels to consider"));
	add_parameter("debug", make_ci_param(m_debug, 0, 1, false, 
					      "write the distance transforms to a 3D image"));

}

C2DFullCost *C2DLabelFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C2DLabelFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
		  << " naxlabels=" << m_maxlabel << "\n";
	return 	new C2DLabelFullCost(m_src_name, m_ref_name, 
				     weight, m_maxlabel, m_debug); 
}

const std::string C2DLabelFullCostPlugin::do_get_descr() const
{
	return "Similarity cost function that maps labels of two images and handles "
                "label-preserving multi-resolution processing."; 

}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DLabelFullCostPlugin();
}

NS_MIA_END
