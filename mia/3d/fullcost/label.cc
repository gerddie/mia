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

#include <cmath>

#include <mia/3d/fullcost/label.hh>
#include <mia/3d/filter.hh>
#include <mia/core/distance.hh>
#include <mia/3d/distance.hh>

NS_MIA_BEGIN
using namespace std; 

C3DLabelFullCost::C3DLabelFullCost(const std::string& src, 
				   const std::string& ref, 
				   double weight, 
                                   int maxlabels):
	C3DFullCost(weight), 
	m_src_key(C3DImageIOPluginHandler::instance().load_to_pool(src)), 
	m_ref_key(C3DImageIOPluginHandler::instance().load_to_pool(ref)),
        m_ref_label_exists(maxlabels), 
        m_ref_distances(maxlabels)
{
	add(::mia::property_gradient);
}

bool C3DLabelFullCost::do_get_full_size(C3DBounds& size) const
{
	TRACE_FUNCTION; 
	assert(m_src); 
	if (size == C3DBounds::_0) {
		size = m_src->get_size(); 
		return true; 
	}else
		return 	size == m_src->get_size(); 
}


double C3DLabelFullCost::do_value(const C3DTransformation& t) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled.size()); 
                
	P3DImage temp  = t(m_src_scaled);
        const C3DUBImage& temp_ubyte = static_cast<const C3DUBImage&>(*temp); 
        
        double result = 0.0; 
        for (size_t i = 0; i < temp_ubyte.size(); ++i) {
		double v = value(i, temp_ubyte[i]);
                result += v; 
        }
        return result; 
}

double C3DLabelFullCost::do_value() const
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


double C3DLabelFullCost::do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const
{
	TRACE_FUNCTION; 
	assert(m_src_scaled.size()); 

	assert("add Z loop"); 
	P3DImage temp  = t(m_src_scaled);
        const C3DUBImage& temp_ubyte = static_cast<const C3DUBImage&>(*temp);

        C3DFVectorfield force(get_current_size()); 

	int idx = 0; 
	auto i = temp_ubyte.begin_range_with_boundary_flags(C3DBounds::_0, temp_ubyte.get_size()); 
	auto e = temp_ubyte.end_range_with_boundary_flags(C3DBounds::_0, temp_ubyte.get_size());
	auto ig  = force.begin(); 
	

	double result = 0.0; 
	while (i != e) {
		result += value_and_gradient(idx, i, *ig); 
		++i; 
		++ig; 
	}


	
	// at this point one could inject a hole-filling algorithm to 
	// add forces inside of the homogen overlapping label regions 
        
	t.translate(force, gradient); 

	cvdebug() << "Image cost =" << result << "\n"; 
	return result; 
	
}

void C3DLabelFullCost::do_set_size()
{
	TRACE_FUNCTION; 
	assert(m_src); 
	assert(m_ref); 

	if (m_src_scaled.size() || m_src_scaled.get_size() != get_current_size() ||
	    m_ref_scaled.size() || m_ref_scaled.get_size() != get_current_size() ) {
		if (get_current_size() == m_src->get_size()) {
			m_src_scaled = static_cast<const C3DUBImage&>(*m_src); 
			m_ref_scaled = static_cast<const C3DUBImage&>(*m_ref); 
		}else{
			stringstream filter_descr; 
			filter_descr << "labelscale:s=[" << get_current_size()<<"]"; 
			auto scaler = C3DFilterPluginHandler::instance().produce(filter_descr.str()); 
			assert(scaler); 
			cvdebug() << "C3DLabelFullCost:scale images to " << get_current_size() << 
				" using '" << filter_descr.str() << "'\n"; 
			m_src_scaled = static_cast<const C3DUBImage&>(*scaler->filter(*m_src)); 
                        m_ref_scaled = static_cast<const C3DUBImage&>(*scaler->filter(*m_ref)); 
		}
		prepare_distance_fields(m_ref_scaled); 
	}

}

void C3DLabelFullCost::prepare_distance_fields( const C3DUBImage &image )
{
        assert(image.get_pixel_type() == it_ubyte); 

        
       
        for (size_t i = 0; i < m_ref_label_exists.size(); ++i) {
                bool exist = false; 
                
                stringstream filter_descr; 
                filter_descr << "binarize:min=" << i << ",max=" << i; 
                auto bin = run_filter(image, filter_descr.str().c_str()); 
        
                const C3DBitImage& bool_bin = static_cast<const C3DBitImage&>(*bin); 
                for (auto i = bool_bin.begin(); i != bool_bin.end() && !exist; ++i) 
                        exist = *i; 
                
                m_ref_label_exists[i] = exist; 
                if (exist) {
			C3DFImage prep(bool_bin.get_size()); 
			distance_transform_prepare(bool_bin.begin(), bool_bin.end(), 
						   prep.begin());
			
                        m_ref_distances[i] = distance_transform(prep); 
			transform(m_ref_distances[i].begin(), m_ref_distances[i].end(), 
				  m_ref_distances[i].begin(), [](float& x){ return sqrt(x);}); 
			
                }
        }
}

double C3DLabelFullCost::value(int idx, int label) const
{
        // non existent labels will be ignored, since they never can be 
        // aligned 
        return m_ref_label_exists[label] ? m_ref_distances[label][idx] : 0.0;
}

double C3DLabelFullCost::value_and_gradient(int idx, const C3DUBImage::const_range_iterator_with_boundary_flag& i, C3DFVector& gradient) const
{
        double result; 
        
        if (m_ref_label_exists[*i]) {
		const auto & dref = m_ref_distances[*i]; 
                result = dref[idx]; 
		if (result > 0.0) {
			auto boundaries = i.get_boundary_flags(); 
			if (boundaries == eb_none) 
				gradient = dref.get_gradient(idx); 
			else { // emulate repeat boundary conditions 
				const auto&  pos = i.pos(); 
				switch (boundaries & eb_x) {
				case eb_xlow: 
					gradient.x = 0.5 * (dref(pos.x + 1, pos.y, pos.z) - dref(pos.x, pos.y, pos.z)); 
					break; 
				case eb_xhigh: 
					gradient.x = 0.5 * (dref(pos.x, pos.y, pos.z) - dref(pos.x - 1, pos.y, pos.z)); 
					break; 
				default:
					gradient.x = 0.5 * (dref(pos.x + 1, pos.y, pos.z) - dref(pos.x - 1, pos.y, pos.z)); 
				}
				
				switch (boundaries & eb_y) {
				case eb_ylow: 
					gradient.y = 0.5 * (dref(pos.x, pos.y + 1, pos.z) - dref(pos.x, pos.y, pos.z)); 
					break; 
				case eb_yhigh: 
					gradient.y = 0.5 * (dref(pos.x, pos.y, pos.z) - dref(pos.x, pos.y - 1, pos.z)); 
					break; 
				default:
					gradient.y = 0.5 * (dref(pos.x, pos.y + 1, pos.z) - dref(pos.x, pos.y - 1, pos.z)); 
				}
				
				switch (boundaries & eb_z) {
				case eb_zlow: 
					gradient.z = 0.5 * (dref(pos.x, pos.y, pos.z + 1) - dref(pos.x, pos.y, pos.z)); 
					break; 
				case eb_zhigh: 
					gradient.z = 0.5 * (dref(pos.x, pos.y, pos.z) - dref(pos.x, pos.y, pos.z - 1)); 
					break; 
				default:
					gradient.z = 0.5 * (dref(pos.x, pos.y, pos.z + 1) - dref(pos.x, pos.y, pos.z - 1)); 
				}

			}
		}else{
			gradient = C3DFVector::_0;
		}
	}else {
                result = 0.0; 
                gradient = C3DFVector::_0;
        }
        return result; 
}


void C3DLabelFullCost::do_reinit()
{
	TRACE_FUNCTION; 
	m_src = get_from_pool(m_src_key);
	m_ref = get_from_pool(m_ref_key);
	m_src_scaled = C3DUBImage(); 
	m_ref_scaled = C3DUBImage(); 

	// is this true? Actually the deformed image is used and it is always interpolated on the full 
	// space of the reference image 
	if (m_src->get_size() != m_ref->get_size()) 
		throw runtime_error("C3DLabelFullCost only works with images of equal size"); 
	
	if (m_src->get_voxel_size() != m_ref->get_voxel_size()) {
		cvwarn() << "C3DLabelFullCost: src and reference image are of differnet voxel dimensions:"
			<< m_src->get_voxel_size() << " vs " << m_ref->get_voxel_size() 
			<< " This code doesn't honour this.\n"; 
	}
        if (m_src->get_pixel_type() != it_ubyte) 
                throw create_exception<runtime_error>("C3DLabelFullCost only works with (label) "
                                                      "images of voxel type ubyte, but src has type ", 
                                                      CPixelTypeDict.get_name(m_src->get_pixel_type()));

        if (m_ref->get_pixel_type() != it_ubyte) 
                throw create_exception<runtime_error>("C3DLabelFullCost only works with (label) "
                                                      "images of voxel type ubyte, but ref has type ", 
                                                      CPixelTypeDict.get_name(m_src->get_pixel_type()));
}

P3DImage C3DLabelFullCost::get_from_pool(const C3DImageDataKey& key)
{
	C3DImageIOPlugin::PData in_image_list = key.get();
		
	if (!in_image_list || in_image_list->empty())
		throw invalid_argument("C3DLabelFullCost: no image available in data pool");

	return (*in_image_list)[0];
}


C3DLabelFullCostPlugin::C3DLabelFullCostPlugin():
        C3DFullCostPlugin("labelimage"), 
	m_src_name("src.@"), 
	m_ref_name("ref.@"), 
        m_maxlabel(256)
{
	add_parameter("src", new CStringParameter(m_src_name, CCmdOptionFlags::input, "Study image", 
			      &C3DImageIOPluginHandler::instance()));
	add_parameter("ref", new CStringParameter(m_ref_name, CCmdOptionFlags::input, "Reference image", 
			      &C3DImageIOPluginHandler::instance()));
	add_parameter("maxlabel", new CIntParameter(m_maxlabel, 2, 32000, false, 
						    "maximum number of labels to consider"));
}

C3DFullCost *C3DLabelFullCostPlugin::do_create(float weight) const
{
	cvdebug() << "create C3DLabelFullCostPlugin with weight= " << weight 
		  << " src=" << m_src_name << " ref=" << m_ref_name 
		  << " naxlabels=" << m_maxlabel << "\n";
	return 	new C3DLabelFullCost(m_src_name, m_ref_name, 
				     weight, m_maxlabel); 
}

const std::string C3DLabelFullCostPlugin::do_get_descr() const
{
	return "Similarity cost function that maps labels of two images and handles "
                "label-preserving multi-resolution processing."; 

}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DLabelFullCostPlugin();
}

NS_MIA_END
