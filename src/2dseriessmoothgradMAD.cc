/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#include <mia/2d/filterchain.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;
using namespace boost::lambda;

template <typename T>
struct fabsdelta {
	T operator () (T x, T y) const {
		return fabs(x - y);
	}
};
struct C2DVarAccumulator : public TFilter<bool> {
	typedef vector<double> CBuffer;

	C2DVarAccumulator(size_t n, size_t gauss_width):
		m_min(numeric_limits<float>::max()),
		m_max(-numeric_limits<float>::max()),
		m_n(n - 1),
		m_initialized(false)
	{
		stringstream gauss_descr;
		gauss_descr << "gauss:w=" << gauss_width;
		gauss = C1DSpacialKernelPluginHandler::instance().produce(gauss_descr.str().c_str());

	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image)
	{
		if (!m_initialized) {
			m_field.resize(image.size());
			m_size = image.get_size();
			m_initialized = true;
		}

		if (image.get_size() != m_size)
			throw invalid_argument("Input images are not all of same size");

		typename T2DImage<T>::const_iterator i = image.begin();
		typename T2DImage<T>::const_iterator e = image.end();
		vector<CBuffer>::iterator v = m_field.begin();

		for(; i != e; ++i, ++v)
			v->push_back(*i);

		pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator>
			src_minmax = ::boost::minmax_element(image.begin(), image.end());
		if (m_min > *src_minmax.first)
			m_min = *src_minmax.first;
		if (m_max < *src_minmax.second)
			m_max = *src_minmax.second;

		return true;
	}
	void evaluate_gradients()
	{
		vector<CBuffer>::iterator v = m_field.begin();
		vector<CBuffer>::iterator e = m_field.end();

		while ( v != e ) {
			gauss->apply_inplace(*v);
			CBuffer gradient(v->size() - 1);
			transform(v->begin() + 1, v->end(), v->begin(), gradient.begin(),
				  fabsdelta<double>());
			*v = gradient;
			++v;
		}
	}

	static float median( CBuffer::iterator begin,
			     CBuffer::iterator end, size_t len)
	{
		if (len & 1) {
			CBuffer::iterator i = begin + (len - 1) / 2;
			std::nth_element(begin, i, end);
			return *i;
		}else {
			CBuffer::iterator i1 = begin + len / 2 - 1;
			CBuffer::iterator i2 = begin + len / 2;
			std::nth_element(begin, i1, end);
			std::nth_element(begin, i2, end);
			return (*i1 + *i2) / 2.0;
		}
	}

	P2DImage  result() {
		evaluate_gradients();

		float range = 255.0 / (m_max - m_min);

		C2DFImage *variation = new C2DFImage(m_size);

		C2DFImage::iterator ii = variation->begin();
		vector<CBuffer>::iterator iv = m_field.begin();
		vector<CBuffer>::iterator ev = m_field.end();
		while (iv != ev) {
			*ii++ = median(iv->begin(), iv->end(), m_n);
			++iv;
		}

		ii = variation->begin();
		iv = m_field.begin();

		while (iv != ev) {
			CBuffer::iterator ip = iv->begin();
			CBuffer::iterator ep = iv->end();

			while (ip != ep) {
				*ip = fabs(*ip - *ii);
				++ip;
			}
			++iv;
			++ii;
		}
		ii = variation->begin();
		iv = m_field.begin();
		while (iv != ev) {
			*ii++ = median(iv->begin(), iv->end(), m_n) * range;
			++iv;
		}
		return P2DImage(variation);
	}

private:
	C2DBounds m_size;
	float m_min;
	float m_max;
	vector<CBuffer> m_field;
	size_t m_n;
	bool m_initialized;
	SHARED_PTR(C1DFilterKernel) gauss;
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
	size_t gauss_width = 1;

	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();





	CCmdOptionList options;
	options.push_back(make_opt( in_filename, "in-file", 'i', "input segmentation set", "input", true));
	options.push_back(make_opt( out_filename, "out-file", 'o', "output file name", "output", true));
	options.push_back(make_opt( skip, "skip", 'p', "Skip files at the beginning", "skip", false));
	options.push_back(make_opt( enlarge_boundary,  "enlarge-boundary", 'e', "Enlarge cropbox by number of pixels",
				    "enlarge", false));
	options.push_back(make_opt( crop, "crop", 'c', "crop image before running statistics", "crop", false));
	options.push_back(make_opt( gauss_width, "gauss", 'g', "gauss filter width for moothing the gradient", "gauss", false));

	try {

		options.parse(argc, argv);
		C2DFilterChain filter_chain(options.get_remaining());

		cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";


		if ( in_filename.empty() )
			throw runtime_error("'--in-file' ('i') option required");

		if ( out_filename.empty() )
			throw runtime_error("'--out-base' ('o') option required");

		CSegSetWithImages  segset(in_filename, true);

		if (crop) {
			C2DBoundingBox box = segset.get_boundingbox();
			box.enlarge(enlarge_boundary);
			stringstream crop_descr;
			crop_descr << "crop:"
				   << "start=[" << box.get_grid_begin()
				   << "],end=[" << box.get_grid_end() << "]";
			cvdebug() << "Crop with " << crop_descr.str() << "\r";


			filter_chain.push_front(C2DFilterPluginHandler::instance().
						produce(crop_descr.str().c_str()));
		}


		if (skip >= segset.get_images().size())
			throw invalid_argument("Skip is equal or larger then image series");

		C2DImageSeries::const_iterator iimages = segset.get_images().begin();
		C2DImageSeries::const_iterator eimages = segset.get_images().end();
		advance(iimages, skip);

		C2DVarAccumulator acc(distance(iimages, eimages), gauss_width);
		for (; iimages != eimages; ++iimages) {

			P2DImage in_image = *iimages;
			if (!filter_chain.empty() )
				in_image = filter_chain.filter(*in_image);
			mia::accumulate(acc, *in_image);
		}

		if (save_image2d(out_filename, acc.result()))
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
