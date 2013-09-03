/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "SERGRADVAR"

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>
//#include <boost/algorithm/minmax_element.hpp>

#include <mia/2d/filter.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
	{pdi_group, "Tools for Myocardial Perfusion Analysis"}, 

	{pdi_short, "Evaluate the time-intensity gradient variation in a series of images."}, 
	
	{pdi_description, "Given a set of images of temporal sucession, this program evaluates the "
	 "gradient variation of the pixel-wise time-intensity curves of this series. "
	 "If the input image set provides a segmentation, then this segmentation can "
	 "be used to create a bounding box and restrict evaluation to this box. "}, 

	{pdi_example_descr, "Evaluate the gradient-variation image of the bounding box surrounding the segmentation "
	 "from a series 'segment.set'. The bounding box will be enlarged by 3 pixels in all directions. "
	 "Store the image in OpenEXR format."}, 
	
	{pdi_example_code,	" -i segment.set -o gradvar.exr -c -e 3"}
}; 


struct C2DVarAccumulator : public TFilter<bool> {

	C2DVarAccumulator():
		m_min(numeric_limits<float>::max()),
		m_max(-numeric_limits<float>::max()),
		m_n(0)
	{
	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image)
	{


		if (m_n) {
			if (image.get_size() != m_old.get_size())
				throw invalid_argument("Input image and mask differ in size");

			transform(image.begin(), image.end(), m_old.begin(), m_delta.begin(),
				  [](float x, float y){return fabs(x - y);}); 
			
			transform(m_delta.begin(), m_delta.end(), m_sum.begin(), m_sum.begin(),  
				  [](float x, float y){return x + y;}); 
			
			transform(m_delta.begin(), m_delta.end(), m_sum2.begin(), m_sum2.begin(), 
				  [](float x, float y){return x * x + y;}); 
			
			auto src_minmax = minmax_element(image.begin(), image.end());
			if (m_min > *src_minmax.first)
				m_min = *src_minmax.first;
			if (m_max < *src_minmax.second)
				m_max = *src_minmax.second;
		}else{
			m_old = C2DFImage(image.get_size());
			m_delta = C2DFImage(image.get_size());
			m_sum = C2DFImage(image.get_size());
			m_sum2 = C2DFImage(image.get_size());

		}
		copy(image.begin(), image.end(), m_old.begin());

		++m_n;
		return true;
	}

	P2DImage  result() const {
		float range = 255.0 / (m_max - m_min);
		cvdebug() << "all over range = " << range << ": (" << m_min << ", " << m_max << ")\n";
		C2DFImage *variation = new C2DFImage(m_sum2.get_size());
		float n = m_n - 1;
		transform(m_sum2.begin(), m_sum2.end(), m_sum.begin(), variation->begin(),
			  [n](float sum2, float sum){ return (sum2 - sum * sum / n )/ (n - 1);}); 

		transform(variation->begin(), variation->end(), variation->begin(), 
			  [range](float x) {return range * sqrt(x);});

		return P2DImage(variation);
	}

private:
	float m_min;
	float m_max;
	C2DFImage m_old;
	C2DFImage m_delta;
	C2DFImage m_sum;
	C2DFImage m_sum2;
	size_t m_n;
};


int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;
	bool crop;
	size_t skip = 0;
	size_t enlarge_boundary = 5;

	const auto& imageio = C2DImageIOPluginHandler::instance();


	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required, &imageio));
	options.add(make_opt( skip, "skip", 'k', "Skip files at the beginning"));
	options.add(make_opt( enlarge_boundary,  "enlarge-boundary", 'e', "Enlarge cropbox by number of pixels"));
	options.add(make_opt( crop, "crop", 'c', "crop image before running statistics"));

	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";

	if ( in_filename.empty() )
		throw runtime_error("'--in-file' ('i') option required");

	if ( out_filename.empty() )
		throw runtime_error("'--out-base' ('o') option required");

	CSegSetWithImages  segset(in_filename, true);

	P2DFilter  crop_filter;
	if (crop) {
		C2DBoundingBox box = segset.get_boundingbox();
		box.enlarge(enlarge_boundary);
		stringstream crop_descr;
		crop_descr << "crop:"
			   << "start=[" << box.get_grid_begin()
			   << "],end=[" << box.get_grid_end() << "]";
		cvdebug() << "Crop with " << crop_descr.str() << "\r";


		crop_filter = C2DFilterPluginHandler::instance().
			produce(crop_descr.str().c_str());
	}


	if (skip >= segset.get_images().size())
		throw invalid_argument("Skip is equal or larger then image series");

	C2DImageSeries::const_iterator iimages = segset.get_images().begin();
	C2DImageSeries::const_iterator eimages = segset.get_images().end();
	advance(iimages, skip);

	C2DVarAccumulator acc;
	for (; iimages != eimages; ++iimages) {

		P2DImage in_image = *iimages;
		if (crop)
			in_image = crop_filter->filter(*in_image);
		mia::accumulate(acc, *in_image);
	}

	if (save_image(out_filename, acc.result()))
		return  EXIT_SUCCESS;
	
	return  EXIT_FAILURE;
	
}
	
#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
