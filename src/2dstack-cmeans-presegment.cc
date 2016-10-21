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

#include <sstream>
#include <ostream>
#include <fstream>
#include <mia/core.hh>
#include <mia/core/cmeans.hh>
#include <mia/core/cmdlineparser.hh>
#include <mia/internal/main.hh>
#include <mia/2d.hh>

NS_MIA_USE;
using namespace std;

const SProgramDescription g_description = {
	
	{pdi_group, "Processing of series of 2D images in a 3D fashion (out-of-core)"}, 
	{pdi_short, "Pre-classify the input image series by using a c-means estimator"}, 
	
	
	{pdi_description, "This program first evaluates a sparse histogram of an input image "
         "series, then runs a c-means classification over the histogram, and then estimates the "
         "mask for one (given) class based on class probabilities. "
         "This program accepts only images of eight or 16 bit integer pixels."
        }, 
	
	{pdi_example_descr,"Run the program over images imageXXXX.png with the sparse histogram, "
         "threshold the lower 30% bins (if available), run cmeans with two classes on the non-zero "
         "pixels and then create the mask for class 1 as foregroundXXXX.png."}, 
	
	{pdi_example_code, "-i imageXXXX.png -o foreground -t png --histogram-tresh=30 --classes 2 --label 1"}
}; 

typedef map<short, CMeans::DVector> Probmap; 
class FGetClassSeedMask: public TFilter<P2DImage> {
public:

	
	FGetClassSeedMask(const Probmap& map,
			  int low_end, int low_label, int high_end, int high_label,
			  const int label, float prob_thresh);

	template <typename T> 
	P2DImage operator() (const T2DImage<T>& image) const;

	P2DImage operator() (const C2DBitImage& image) const;
private:
	const Probmap& m_map;

	int m_low_end;
	int m_low_label;
	int m_high_end;
	int m_high_label;
	int m_label;
	float m_prob_thresh; 
}; 


FGetClassSeedMask::FGetClassSeedMask(const Probmap& map,
				     int low_end, int low_label, int high_end, int high_label,
				     const int label, float prob_thresh):
	m_map(map),
	m_low_end(low_end),
	m_low_label(low_label),
	m_high_end(high_end),
	m_high_label(high_label),  
	m_label(label),
	m_prob_thresh(prob_thresh)
{
}

P2DImage FGetClassSeedMask::operator() (const C2DBitImage& MIA_PARAM_UNUSED(image)) const
{
	throw invalid_argument("Unsupported input pixel type: This classification doesn't make sense for binary images"); 
}
					    
template <typename T> 
P2DImage FGetClassSeedMask::operator() (const T2DImage<T>& image) const
{
	C2DBitImage *result = new C2DBitImage(image.get_size(), image);
	P2DImage presult(result);
	
	auto run_pixel = [this](T pixel) -> bool {

		const T low_end = static_cast<T>(m_low_end);
		const T high_end = static_cast<T>(m_high_end); 
		// 
		// check boundaries of probability map whether they 
		// correspond to the label
		// outside the range probability of that label is always 1.0 
		if (pixel <= low_end)
			return (m_label == m_low_label);
		if (pixel >= high_end)
			return (m_label == m_high_label);

		auto l = m_map.find(pixel);
		if (l != m_map.end()) {
			return (l->second[m_label] >= m_prob_thresh);                          
		} else {
			// this should not happen
			cvwarn() << "Unmapped value " << pixel << "\n";
			return false; 
		}

	}; 
	
	transform(image.begin(), image.end(), result->begin(), run_pixel); 
	return presult; 
}

int do_main( int argc, char *argv[] )
{
        string out_mask;
	string out_probmap;
	string in_filename;
        string out_type("png");

	float seed_threshold = 0.95; 
        float histogram_thresh = 5;

	int label = -1; 
	
	CMeans::PInitializer class_center_initializer;

	
	const C2DImageIOPluginHandler::Instance& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);

        options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input image(s) to be filtered", 
			      CCmdOptionFlags::required_input, &imageio));
	options.add(make_opt( out_probmap, "out-probmap", 'p', "Save probability map to this file", 
			      CCmdOptionFlags::output));
        options.add(make_opt( out_type, "type", 't', "output file name type"));

	options.add(make_opt( out_mask, "out-mask", 'o', "output file name base", 
			      CCmdOptionFlags::required_output));


        options.set_group("Parameters");
        options.add(make_opt( histogram_thresh, EParameterBounds::bf_closed_interval, {0,50}, "histogram-thresh", 'T',
                              "Percent of the extrem parts of the histogram to be collapsed into the respective last histogram bin."));
        
        options.add(make_opt( class_center_initializer, "kmeans:nc=3", "classes", 'C', "C-means class initializer"));
	
	options.add(make_opt( seed_threshold, EParameterBounds::bf_open_interval, {0.0f,1.0f}, "seed-threshold", 'S',
                              "Probability threshold value to consider a pixel as seed pixel."));

	options.add(make_opt( label, EParameterBounds::bf_closed_interval, {0,10}, "label", 'L',
                              "Class label to create the mask from", CCmdOptionFlags::required));

        
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 
	
	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(in_filename, start_filenum, end_filenum, format_width);
		
	if (start_filenum >= end_filenum)
		throw invalid_argument(string("no files match pattern ") + src_basename);


        CSparseHistogram histo;
        size_t n_pixels = 0; 
        for (size_t i = start_filenum; i < end_filenum; ++i) {
                string src_name = create_filename(src_basename.c_str(), i);
                C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
                cvmsg() << "Read:" << src_name << "\r";
                if (in_image_list.get() && in_image_list->size()) {
                        for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k)
                                n_pixels += accumulate(histo, **k);
                }
        }
	
        auto chistogram = histo.get_compressed_histogram();
	
        size_t n_cut_off = static_cast<size_t>(floor(n_pixels / 100.0 * histogram_thresh));
	
	
	size_t ni = 0; 
	auto ii = chistogram.begin(); 
        while ( ni < n_cut_off && ii != chistogram.end()) {
		ni += ii->second;
                ++ii; 
        }

	if (ii == chistogram.end()) {
		// should be impossible but just be on the save side
		throw create_exception<invalid_argument>("The provided histogram thresh ", histogram_thresh,
							 " results in an empty histogram, select a lower value"); 
	}
	
	size_t ne = 0; 
	auto ie = chistogram.end() - 1;
        while ( ne < n_cut_off && ie !=  ii) {
		ne += ie->second;
                --ie;
        }

	CSparseHistogram::Compressed threshed_histo(ii, ie);

	CMeans::DVector class_centers; 
	
	CMeans cmeans(0.00001, class_center_initializer);
	CMeans::SparseProbmap pv = cmeans.run(threshed_histo,  class_centers);

	Probmap pmap; 
	for(auto ipv : pv) {
		pmap[ipv.first] = ipv.second; 
	}
	
	
	if ( ! out_probmap.empty() ) {
		ofstream outstr(out_probmap.c_str());
		for (auto ipv: pv) {
			outstr << ipv.first << " " << ipv.second << "\n"; 
		}
	}

	if (class_centers.size() > 65535) {
		throw create_exception<runtime_error>("This code only allows 65535 classes, initializer created ",  class_centers.size()); 
	}

	if (class_centers.size() <= static_cast<unsigned>(label)) {
		throw create_exception<runtime_error>("Try to segment class ", label, " but only ",
						      class_centers.size(), "classes available"); 
	}

	
	FGetClassSeedMask seeder(pmap, ii->first, 0, ie->first, class_centers.size() - 1,
				 label, seed_threshold);
	
        for (size_t i = start_filenum; i < end_filenum; ++i) {
                string src_name = create_filename(src_basename.c_str(), i);
                C2DImageIOPluginHandler::Instance::PData  in_image_list = imageio.load(src_name);
                cvmsg() << "Read:" << src_name << "\r";
                if (in_image_list.get() && in_image_list->size()) {
                        for (auto k = in_image_list->begin(); k != in_image_list->end(); ++k) {
				// create label image
				*k = mia::filter (seeder, **k);
			}
                }
		stringstream ss;
		ss << out_mask << setw(format_width) << setfill('0') << i << "." << out_type;
		
		imageio.save(ss.str(), *in_image_list);
        }
	return EXIT_SUCCESS;
}

MIA_MAIN(do_main); 

        
