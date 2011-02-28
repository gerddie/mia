/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
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

// $Id: 2dimagefilterstack.cc,v 1.19 2006-07-28 09:37:05 wollny Exp $

/*! \brief eva-2dimagefilter

\sa 3va-2dimagefilter.cc

\file mask.cc
\author G. Wollny, wollny eva.mpg.de, 2005
*/
#define VSTREAM_DOMAIN "SERGRADVAR"

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <boost/lambda/lambda.hpp>
#include <boost/algorithm/minmax_element.hpp>

#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;
using namespace boost::lambda;

static const char *program_info = 
	"This program is used to evaluate the temporal pixel wise gradient variation\n"
	"of a set of images in temporal sucession\n"
	"Usage:\n"
	"  mia-2dseriesgradvariation -i <input set> -o <output image> [<options>]\n";

template <typename T>
struct fabsdelta {
	T operator () (T x, T y) const {
		return fabs(x - y);
	}
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
				  fabsdelta<float>());
			transform(m_delta.begin(), m_delta.end(), m_sum.begin(), m_sum.begin(),  _1 + _2);
			transform(m_delta.begin(), m_delta.end(), m_sum2.begin(), m_sum2.begin(),  _1 * _1 + _2);

			pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator>
				src_minmax = ::boost::minmax_element(image.begin(), image.end());
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
			  (_1 - _2 * _2 / n )/ (n - 1));

		for (C2DFImage::iterator i = variation->begin(); i!= variation->end(); ++i)
			*i = range * sqrt(*i);

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


/* Revision string */
const char revision[] = "not specified";

int main( int argc, const char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;
	bool crop;
	size_t skip = 0;
	size_t enlarge_boundary = 5;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();


	CCmdOptionList options(program_info);
	options.push_back(make_opt( in_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required));
	options.push_back(make_opt( skip, "skip", 'p', "Skip files at the beginning"));
	options.push_back(make_opt( enlarge_boundary,  "enlarge-boundary", 'e', "Enlarge cropbox by number of pixels"));
	options.push_back(make_opt( crop, "crop", 'c', "crop image before running statistics"));

	try {


		options.parse(argc, argv);
		cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";

		if ( in_filename.empty() )
			throw runtime_error("'--in-file' ('i') option required");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");

		CSegSetWithImages  segset(in_filename, true);

		C2DFilterPlugin::ProductPtr  crop_filter;
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

	}
	catch (const runtime_error &e){
		cerr << argv[0] << " runtime: " << e.what() << endl;
	}
	catch (const invalid_argument &e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (const exception& e){
		cerr << argv[0] << " error: " << e.what() << endl;
	}
	catch (...){
		cerr << argv[0] << " unknown exception" << endl;
	}

	return EXIT_FAILURE;
}
