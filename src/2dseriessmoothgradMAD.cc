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

	{pdi_short, "Evaluate pixel-wise time-intensity gradient of a 2D image series."}, 
	
	{pdi_description, "Given a set of images of temporal sucession, evaluate the temporal "
	 "pixel-wise gaussian and evaluate pixel-wise its MAD." 
	 "A spacial pre-filtering may be applied by specifying additional plugins "
	 "(filter/2dimage)"}, 
	
	{pdi_example_descr, "Evaluate the MAD-image of the bounding box surrounding the segmentation "
	 "from a series segment.set after applying a temporal Gaussian "
	 "filter of width 5. No spacial filtering will be applied. "
	 "The bounding box will be enlarged by 3 pixels in all directions. "
	 "Store the image in OpenEXR format."}, 
	 
	{pdi_example_code, "-i segment.set -o mad.exr -g 2 -c -e 3"}
};

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
		m_initialized(false)	{
		stringstream gauss_descr;
		gauss_descr << "gauss:w=" << gauss_width;
		gauss = C1DSpacialKernelPluginHandler::instance().produce(gauss_descr.str().c_str());
		
	}
	
	template <typename T>
	bool operator ()(const T2DImage<T>& image){
		if (!m_initialized) {
			m_field.resize(image.size());
			m_size = image.get_size();
			m_initialized = true;
		}
		
		if (image.get_size() != m_size)
			throw invalid_argument("Input images are not all of same size");
		
		auto i = image.begin();
		auto e = image.end();
		auto v = m_field.begin();
		
		for(; i != e; ++i, ++v)
			v->push_back(*i);
		
		auto src_minmax = minmax_element(image.begin(), image.end());
		if (m_min > *src_minmax.first)
			m_min = *src_minmax.first;
		if (m_max < *src_minmax.second)
			m_max = *src_minmax.second;
		
		return true;
	}
	
	void evaluate_gradients(){
		auto v = m_field.begin();
		auto e = m_field.end();
		
		while ( v != e ) {
			gauss->apply_inplace(*v);
			CBuffer gradient(v->size() - 1);
			transform(v->begin() + 1, v->end(), v->begin(), gradient.begin(),
				  [](double x, double y) {return fabs(x - y);});
			*v = gradient;
			++v;
		}
	}
	
	static float median( CBuffer::iterator begin, CBuffer::iterator end, size_t len){
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
		evaluate_gradients();
			
		float range = 255.0 / (m_max - m_min);
			
		C2DFImage *variation = new C2DFImage(m_size);
			
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
	C2DBounds m_size;
	float m_min;
	float m_max;
	vector<CBuffer> m_field;
	size_t m_n;
	bool m_initialized;
	std::shared_ptr<C1DFilterKernel > gauss;
};
	
int do_main( int argc, char *argv[] )
{

	string in_filename;
	string out_filename;
	string out_type;
	bool crop;
	size_t skip = 0;
	size_t enlarge_boundary = 5;
	size_t gauss_width = 1;

	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.add(make_opt( in_filename, "in-file", 'i', "input segmentation set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output file name", CCmdOption::required, &imageio));
	options.add(make_opt( skip, "skip", 'k', "Skip files at the beginning"));
	options.add(make_opt( enlarge_boundary,  "enlarge-boundary", 'e', "Enlarge cropbox by number of pixels"));
	options.add(make_opt( crop, "crop", 'c', "crop image before running statistics"));
	options.add(make_opt( gauss_width, "gauss", 'g', "gauss filter width for moothing the gradient"));

	if (options.parse(argc, argv, "filter", &C2DFilterPluginHandler::instance()) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	C2DImageFilterChain filter_chain(options.get_remaining());

	cvdebug() << "IO supported types: " << imageio.get_plugin_names() << "\n";
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

	auto iimages = segset.get_images().begin();
	auto  eimages = segset.get_images().end();
	advance(iimages, skip);

	C2DVarAccumulator acc(distance(iimages, eimages), gauss_width);
	for (; iimages != eimages; ++iimages) {
		
		P2DImage in_image = *iimages;
		if (!filter_chain.empty() )
			in_image = filter_chain.run(in_image);
		mia::accumulate(acc, *in_image);
	}
	
	if (save_image(out_filename, acc.result()))
		return  EXIT_SUCCESS;
	return EXIT_FAILURE; 
}

MIA_MAIN(do_main)
