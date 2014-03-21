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

#define VSTREAM_DOMAIN "SERGRADVAR"

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sstream>
#include <stdexcept>

#include <mia/internal/main.hh>
#include <mia/2d/filterchain.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/core.hh>

using namespace std;
using namespace mia;

const SProgramDescription g_description = {
        {pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Evaluate the time-intensity gradient MAD in a series of images."}, 
	{pdi_description, "Given a set of images of temporal sucession, evaluates the "
	 "pixel-wise temporal gradient and then its median average distance (MAD) "
	 "and stores the result in an image. Spacial pre-filtering may be applied "
	 "as given additional plugin(s) (filter/2dimage)."}, 
	{pdi_example_descr, "Evaluate the MAD-image of the bounding box surrounding "
	 "the segmentation from a series segment.set. No spacial filtering will be "
	 "applied. The bounding box will be enlarged by 3 pixels in all directions. "
	 "Store the image in OpenEXR format."}, 
	{pdi_example_code, " -i segment.set -o mad.exr -c -e 3"}
}; 

template <typename T>
struct fabsdelta {
	T operator () (T x, T y) const {
		return fabs(x - y);
	}
};
struct C2DVarAccumulator : public TFilter<bool> {
	typedef vector<float> CBuffer;

	C2DVarAccumulator(size_t n):
		m_min(numeric_limits<float>::max()),
		m_max(-numeric_limits<float>::max()),
		m_n(n - 1),
		m_initialized(false)
	{
	}

	template <typename T>
	bool operator ()(const T2DImage<T>& image)
	{


		if (m_initialized) {
			if (image.get_size() != m_old.get_size())
				throw invalid_argument("Input image and mask differ in size");

			transform(image.begin(), image.end(), m_old.begin(), m_delta.begin(),
				  fabsdelta<float>());
			auto i = m_delta.begin();
			auto e = m_delta.end();
			auto v = m_field.begin();

			for(; i != e; ++i, ++v)
				v->push_back(*i);

			auto src_minmax = minmax_element(image.begin(), image.end());
			if (m_min > *src_minmax.first)
				m_min = *src_minmax.first;
			if (m_max < *src_minmax.second)
				m_max = *src_minmax.second;

		}else{
			m_old = C2DFImage(image.get_size());
			m_delta = C2DFImage(image.get_size());
			m_field.resize(image.size());
			m_initialized = true;
		}
		copy(image.begin(), image.end(), m_old.begin());

		return true;
	}
	static float median( CBuffer::iterator begin,
			     CBuffer::iterator end, size_t len)
	{
		if (len & 1) {
			auto i = begin + (len - 1) / 2;
			nth_element(begin, i, end);
			return *i;
		}else {
			auto i1 = begin + len / 2 - 1;
			auto i2 = begin + len / 2;
			nth_element(begin, i1, end);
			nth_element(begin, i2, end);
			return (*i1 + *i2) / 2.0;
		}
	}

	P2DImage  result() {
		float range = 255.0 / (m_max - m_min);

		C2DFImage *variation = new C2DFImage(m_delta.get_size());

		auto ii = variation->begin();
		auto iv = m_field.begin();
		auto ev = m_field.end();
		while (iv != ev) {
			*ii++ = median(iv->begin(), iv->end(), m_n);
			++iv;
		}

		ii = variation->begin();
		iv = m_field.begin();

		while (iv != ev) {
			auto ip = iv->begin();
			auto ep = iv->end();

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
	float m_min;
	float m_max;
	C2DFImage m_old;
	C2DFImage m_delta;
	vector<CBuffer> m_field;
	size_t m_n;
	bool m_initialized;
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
	options.add(make_opt( in_filename, "in-file", 'i', "input segmentation set", CCmdOptionFlags::required_input));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOptionFlags::required_output, &imageio));
	options.add(make_opt( skip, "skip", 'k', "Skip files at the beginning"));
	options.add(make_opt( enlarge_boundary,  "enlarge-boundary", 'e', 
				    "Enlarge cropbox by number of pixels"));
	options.add(make_opt( crop, "crop", 'c', "crop image before running statistics"));

	if (options.parse(argc, argv, "filter", &C2DFilterPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
		
	C2DImageFilterChain filter_chain(options.get_remaining());

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


		filter_chain.push_front(crop_descr.str().c_str());
	}


	if (skip >= segset.get_images().size())
		throw invalid_argument("Skip is equal or larger then image series");

	C2DImageSeries::const_iterator iimages = segset.get_images().begin();
	C2DImageSeries::const_iterator eimages = segset.get_images().end();
	advance(iimages, skip);

	C2DVarAccumulator acc(distance(iimages, eimages));
	for (; iimages != eimages; ++iimages) {

		P2DImage in_image = *iimages;
		if (!filter_chain.empty() )
			in_image = filter_chain.run(in_image);
		mia::accumulate(acc, *in_image);
	}

	if (save_image(out_filename, acc.result()))
		return  EXIT_SUCCESS;
	return  EXIT_FAILURE;

}
	
MIA_MAIN(do_main); 
