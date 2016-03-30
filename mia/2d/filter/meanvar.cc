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

#include <mia/2d/filter/meanvar.hh>
#include <mia/2d/imageio.hh>

#include <limits>

#include <gsl/gsl_cblas.h>

NS_BEGIN(meanvar_2dimage_filter);

NS_MIA_USE;
using namespace std;

C2DMeanVar::C2DMeanVar(unsigned hw, double thresh, const std::string& varfilename):
	m_hw(hw),
	m_thresh(thresh),
	m_varfilename(varfilename)
{
}

template <typename T>
std::pair<double, double> evaluate_pixel(const T2DImage<T>& data, unsigned x, unsigned y, unsigned hw, double thresh)
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


	std::pair<double, double> result = {0.0f, 0.0f};

	float orig_value = data(x,y);
	
	if (orig_value >=  thresh) {
		double sum = 0.0f;
		double sum2 = 0.0f;
		unsigned n = 0; 
		
		for (unsigned iy = starty; iy < endy; ++iy) {
			for (unsigned ix = startx; ix < endx; ++ix) {
				double v = data(ix, iy);
				if (v >= thresh) {
					sum += v;
					sum2 += v*v;
					++n;
				}
			}
		}
		if (n > 1) {
			result.first = sum / n;
			result.second = sqrt((sum2 - sum * result.first) / (n - 1));
		}else{
			result.first = orig_value; 
		}
	}
	return result; 
}

template <typename T>
C2DMeanVar::result_type C2DMeanVar::operator () (const T2DImage<T>& data) const
{
	TRACE_FUNCTION;

	cvdebug() << "Running 2D meanvar with hw=" << m_hw
		  << ", thresh=" << m_thresh
		  << ", and varfile=" << m_varfilename
		  << "\n";

	C2DFImage *result_mu = new C2DFImage(data.get_size(), data);
	C2DFImage *result_sigma = new C2DFImage(data.get_size(), data);

	P2DImage presult_sigma(result_sigma);
	
	auto evaluate_row = [&](const C1DParallelRange& range){
		for (int y = range.begin(); y != range.end(); ++y){
			auto omu = result_mu->begin_at(0,y);
			auto osigma = result_sigma->begin_at(0,y);
			for (unsigned x = 0; x < data.get_size().x; ++x, ++omu, ++osigma){
				auto r = evaluate_pixel(data, x, y, m_hw, m_thresh);
				*omu = r.first;
				*osigma = r.second; 
			}
		}
	};

	pfor(C1DParallelRange(0, data.get_size().y, 1), evaluate_row);
	
	if (!m_varfilename.empty()) {
		if (!save_image(m_varfilename, presult_sigma)) {
			cverr() << "C2DMeanVar: Unable to save variation data to '" << m_varfilename <<"'"; 
		}
	
	}
	return P2DImage(result_mu);
}

P2DImage C2DMeanVar::do_filter(const C2DImage& image) const
{
	return mia::filter(*this, image);
}

C2DMeanVarImageFilterFactory::C2DMeanVarImageFilterFactory():
	C2DFilterPlugin("meanvar"),
	m_hw(1),
	m_thresh(0.0)
{
	add_parameter("w", make_lc_param(m_hw, 1, false, "filter width parameter"));
	add_parameter("thresh", make_lc_param(m_thresh, 0.0, false, "Intensity threshholding parameter: Pixels with intensities "
					      "below this threshhold will be set to zero, and also not used when evaluating mean "
					      "and variation"));
	add_parameter("varfile", new CStringParameter(m_varfilename, CCmdOptionFlags::required_output,
						      "name of the output file to save the variation image too.", 
						      &C2DImageIOPluginHandler::instance()));
}

C2DFilter *C2DMeanVarImageFilterFactory::do_create()const
{
	return new C2DMeanVar(m_hw, m_thresh, m_varfilename);
}

const string C2DMeanVarImageFilterFactory::do_get_descr()const
{
	return "Filter that evaluates simultaniously the pixel wise mean and the variance of an image in a given window. "
		"Pixel intensities below the given threshold will be ignored and at their loctions the output mean and "
		"variation are set to zero. The mean intensity image is directly passed as float image to the pipeline, "
		"the variation image is saved to a file given with the varfile parameter.";
}

struct FCompare {
	bool operator () (float a, float b){
		return ::fabs(a-b) < 0.0001;
	}
};

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DMeanVarImageFilterFactory();
}

NS_END
