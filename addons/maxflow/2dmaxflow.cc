/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <mia/core/filter.hh>
#include <addons/maxflow/2dmaxflow.hh>

NS_BEGIN( maxflow_2dimage_filter)

using namespace mia; 
using std::string;
using std::invalid_argument; 
using std::minmax_element; 

C2DMaxflow::C2DMaxflow(const std::string& sink_flow_imagefile, const std::string& source_flow_imagefile):
        m_sink_flow_imagefile(sink_flow_imagefile), 
	m_source_flow_imagefile(sink_flow_imagefile)
{
}


C2DFImage load_flow_image(const string& imagefile, const string& type, const C2DBounds& expect_size)
{
	auto pimage = load_image2d(imagefile);
	if (pimage->get_size() != expect_size) {
		throw create_exception<invalid_argument>("Maxflow: ", type ," flow image '", imagefile,
							 "' has size (", pimage->get_size(),
							 "), but expect size (",expect_size, ")");
	}
	if (pimage->get_pixel_type() != it_float)
		throw create_exception<invalid_argument>("Maxflow: ", type ," flow image '", imagefile,
							 "' is not of type 'float'"); 

	return C2DFImage(static_cast<const C2DFImage&>(*pimage)); 
}


// this needs to become tunable

class FGradToFlow {

	FGradFlow grad_to_flow(float vmin, float vmax)
	{
		if (vmax <= vmin) {
			throw create_exception<invalid_argument>("Maxflow: input image seems to be of one value only"); 
		}

		m_scale = 1.0/(vmax - vmin); 

	}

	float operator() (float x, float y) const
	{
		float delta = (x - y) * m_scale;
		return 1.0f / (delta * delta + 1.0f); 
	}
private:
	float m_scale; 
};



template <class T>
typename C2DMaxflow::result_type C2DMaxflow::operator () (const mia::T2DImage<T>& data) const
{
	// load the sink and source flow images
	// throws if file not available 
	auto sink = load_flow_image(m_sink_flow_imagefile);
	auto source = load_flow_image(m_source_flow_imagefile);

	// create the maxflow object
	Graph_FFF graph(data.size(), 2*(data.get_size().x -1) * (data.get_size().y -1));

	graph.add_node(data.size());

	// add the capacities towards sinks ans sources 
	int idx = 0;
	auto isink = sink.begin();
	auto esink = sink.end(); 
	auto isource = source.begin();

	while (isink != esink) {
		if (*isink > 0 || *esink > 0) {
			graph.add_tweighs(idx, *isource, isink);  
		}
		++isink;
		++isource;
		++idx; 	
	}

	// this should become a configurable function 
	auto minmax_vals  = minmax_element(data.begin(), data.end()); 
	FGradToFlow grad_to_flow(*minmax_vals.first, *minmax_vals.second); 

	// add the inter-pixel capacities
	idx = 0;
	for (unsigned y = 0; y < data.get_size().y - 1; ++y) {
		for (unsigned x = 0; x < data.get_size().x - 1; ++x, ++idx) {
			auto flow = grad_to_flow(data(x,y), data(x+1,y));
			graph.add_edge(idx,idx +1, flow, flow);  
			
			auto flow = grad_to_flow(data(x,y), data(x,y+1));
			graph.add_edge(idx,idx + data.get_size().x, flow, flow);  

		}
		++idx;
	}

	int flow = g.maxflow();

	C2DBitImage *result = new C2DBitImage(data.get_size(), data);

	idx = 0; 
	for (auto ir = result->begin(); ir != result->end(); ++ir, ++idx) {
		*ir = g.what_segment(idx) == Graph_III::SOURCE; 
	}
       
	return PImage(result);
	
}

mia::P2DImage C2DMaxflow::do_filter(const mia::C2DImage& image) const
{
	return mia::filter(*this, image);  
}



class C2DMaxflowFilterPluginFactory: public mia::C2DFilterPlugin {
public:
	C2DMaxflowFilterPluginFactory();
private:
	virtual mia::C2DFilter *do_create()const;
	virtual const std::string do_get_descr()const;

	std::string m_sink_flow_imagefile;
	std::string m_source_flow_imagefile;
};

NS_END
