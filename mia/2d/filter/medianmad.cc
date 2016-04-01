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
#include <mia/core/msgstream.hh>
#include <mia/core/threadedmsg.hh>
#include <mia/core/parallel.hh>

#include <mia/2d/filter/medianmad.hh>
#include <mia/2d/imageio.hh>

#include <limits>
#include <cmath>

#include <gsl/gsl_cblas.h>

NS_BEGIN(medianmad_2dimage_filter);

NS_MIA_USE;
using namespace std;

C2DMedianMad::C2DMedianMad(unsigned hw, double thresh, const std::string& madfilename):
	m_hw(hw),
	m_thresh(thresh),
	m_madfilename(madfilename)
{
}

template <typename T>
std::pair<T, T> evaluate_pixel(const T2DImage<T>& data,
					 unsigned x, unsigned y, unsigned hw, double thresh, vector<T>& buffer)
{
	unsigned startx = x - hw;
	if (startx > data.get_size().x)
		startx = 0;

	unsigned endx = x + hw + 1;
	if (endx > data.get_size().x)
		endx = data.get_size().x;

	unsigned starty = y - hw;
	if (starty > data.get_size().y)
		starty = 0;

	unsigned endy = y + hw + 1;
	if (endy > data.get_size().y)
		endy = data.get_size().y;


	std::pair<T, T> result = {0, 0};

	float orig_value = data(x,y);

	if (orig_value >=  thresh) {
		int n = 0; 
		auto i = buffer.begin(); 
		for (unsigned iy = starty; iy < endy; ++iy) {
			for (unsigned ix = startx; ix < endx; ++ix) {
				double v = data(ix, iy);
				if (v >= thresh) {
					*i++ = v;
					++n; 
				}
			}
		}
		if (i != buffer.begin()) {
			sort(buffer.begin(), i);
			if (n & 1) 
				result.first = buffer[n / 2];
			else
				result.first = static_cast<T>(0.5 * (buffer[n/2] + buffer[n/2 - 1]));

			transform (buffer.begin(), i, buffer.begin(),
				   [result](T x){ return x >  result.first ? x - result.first : result.first - x;});
			sort(buffer.begin(), i);
			if (n & 1) 
				result.second = buffer[n / 2];
			else
				result.second = static_cast<T>(0.5 * (buffer[n/2] + buffer[n/2 - 1]));
			
		}else{
			result.first = orig_value; 
		}
	}
	return result; 
}

template <typename T>
C2DMedianMad::result_type C2DMedianMad::operator () (const T2DImage<T>& data) const
{
	TRACE_FUNCTION;

	cvdebug() << "Running 2D MedianMad with hw=" << m_hw
		  << ", thresh=" << m_thresh
		  << ", and madfile=" << m_madfilename
		  << "\n";

	T2DImage<T> *result_mu = new T2DImage<T>(data.get_size(), data);
	T2DImage<T> *result_sigma = new T2DImage<T>(data.get_size(), data);

	P2DImage presult_sigma(result_sigma);
	
	auto evaluate_row = [&](const C1DParallelRange& range){
		int size = 2 * m_hw + 1; 
		vector<T> buffer(size * size); 
		for (int y = range.begin(); y != range.end(); ++y){
			auto omu = result_mu->begin_at(0,y);
			auto osigma = result_sigma->begin_at(0,y);
			for (unsigned x = 0; x < data.get_size().x; ++x, ++omu, ++osigma){
				auto r = evaluate_pixel(data, x, y, m_hw, m_thresh, buffer);
				*omu = r.first;
				*osigma = r.second; 
			}
		}
	};

	pfor(C1DParallelRange(0, data.get_size().y, 1), evaluate_row);
	
	if (!m_madfilename.empty()) {
		if (!save_image(m_madfilename, presult_sigma)) {
			cverr() << "C2DMedianMad: Unable to save median absolute distance data to '" << m_madfilename <<"'"; 
		}
	
	}
	return P2DImage(result_mu);
}

P2DImage C2DMedianMad::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMedianMadImageFilterFactory::C2DMedianMadImageFilterFactory():
	C2DFilterPlugin("medianmad"),
	m_hw(1),
	m_thresh(0.0)
{
	add_parameter("w", make_lc_param(m_hw, 1, false, "filter width parameter"));
	add_parameter("thresh", make_lc_param(m_thresh, 0.0, false, "Intensity threshholding parameter: Pixels with intensities "
					      "below this threshhold will be set to zero, and also not used when evaluating mean "
					      "and variation"));
	add_parameter("madfile", new CStringParameter(m_madfilename, CCmdOptionFlags::required_output,
						      "name of the output file to save the median absolute deviation image too.", 
						      &C2DImageIOPluginHandler::instance()));
}

C2DFilter *C2DMedianMadImageFilterFactory::do_create()const
{
	return new C2DMedianMad(m_hw, m_thresh, m_madfilename);
}

const string C2DMedianMadImageFilterFactory::do_get_descr()const
{
	return "Filter that evaluates simultaniously the pixel wise median and the median absolute deviation (MAD) of an image"
		" in a given window. Pixel intensities below the given threshold will be ignored "
		"and at their loctions the output median and MAD are set to zero. The median "
		"intensity image is directly passed to the pipeline, the variation image is "
		"saved to a file given with the varfile parameter. Both output images have the same pixel type like the input image.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMedianMadImageFilterFactory();
}

NS_END
