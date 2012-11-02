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


#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>


#include <queue>
#include <libxml++/libxml++.h>


#include <mia/core.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/transformfactory.hh>
NS_MIA_USE;

const SProgramDescription g_description = {
        {pdi_group, "Work in progress"}, 
	{pdi_short, "Work in progress."}, 
	{pdi_description, "This program is work in progress"}, 
	{pdi_example_descr, "Example"}, 
	{pdi_example_code, "Example"}
}; 



namespace bfs=boost::filesystem; 

class C2DFImage2PImage {
public: 
	P2DImage operator () (const C2DFImage& image) const {
		return P2DImage(new C2DFImage(image)); 
	}
}; 

void add_feature_mask(P2DImage feature_image, int id, C2DUBImage& mask) 
{
	const char *filters[5] ={
		"kmeans:c=5", 
		"binarize:min=4,max=4", 
		"close:shape=[sphere:r=4]", 
		"label", 
		"selectbig"
	};
	
	// run cmeans + binarize + get largest connected component
	auto feature_mask = run_filter_chain(feature_image, 5, filters); 
	auto bitmask = dynamic_cast<const C2DBitImage&>(*feature_mask);
	
	assert(bitmask.get_size() == mask.get_size()); 
	
	transform(bitmask.begin(), bitmask.end(), mask.begin(), mask.begin(), 
		  [id](bool feature, unsigned char m) {return feature ? id : m;}); 
}


int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// ICA parameters 
	size_t components = 0;
	bool normalize = false; 
	bool no_meanstrip = false; 
	size_t skip_images = 0; 
	size_t max_ica_iterations = 400; 

	int reference = -1; 
	int nsegments = 12; 

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output file with curves", CCmdOption::required));

	options.set_group("ICA");
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation"));
	options.add(make_opt( normalize, "normalize", 0, "normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	options.set_group("Evaluation");
	options.add(make_opt( reference, "reference", 'r', "reference frame for curve mask (-1 = use LV peak)")); 
	options.add(make_opt( nsegments, "nsegments", 'n', "number of myocardial segments to use (0=use segmented)"));
	

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), Convert2Float()); 
	

	// run ICA
	C2DPerfusionAnalysis ica(components, normalize, !no_meanstrip); 
	if (max_ica_iterations) 
		ica.set_max_ica_iterations(max_ica_iterations); 
	

	if (!ica.run(series)) {
		ica.set_approach(FICA_APPROACH_SYMM); 
		ica.run(series); 
	}

	if (reference == -1) 
		reference = ica.get_LV_peak_time(); 
	if (reference == -1) 
		throw runtime_error("LV peak time based reference requested, but it can't be identified"); 
	
	
	const auto& ref = input_set.get_frames()[reference]; 
	C2DUBImage mask = ref.get_section_masks(nsegments);
	
	int rv_id = nsegments > 0 ? nsegments + 1 : ref.get_nsections(); 
	int lv_id = rv_id + 1; 
	
	int rv_idx = ica.get_RV_peak_idx(); 
	if (rv_idx < 0) 
		throw runtime_error(" no RV feature identified"); 
	add_feature_mask(ica.get_feature_image(rv_idx), rv_id, mask); 

	int lv_idx = ica.get_LV_peak_idx(); 
	if (lv_idx < 0) 
		throw runtime_error(" no LV feature identified"); 
	add_feature_mask(ica.get_feature_image(lv_idx), lv_id, mask);
	
	// evaluate time-intensity curves with skipping initial frames 
	vector<CSegFrame::SectionsStats> intensity_curves(series.size()); 
	transform(input_set.get_frames().begin() + skip_images, input_set.get_frames().end(), intensity_curves.begin(), 
		  [&mask](const CSegFrame& f){return f.get_stats(mask);});
	
	ofstream output(out_filename.c_str()); 
	int k = skip_images; 
	for (auto i = intensity_curves.begin(); i != intensity_curves.end(); ++i, ++k) {
		output << k << " "; 
		for (auto ii = i->begin(); ii != i->end(); ++ii)
			output << ii->first << " " << ii->second << " "; 
		output << "\n"; 
	}
	return EXIT_SUCCESS; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

