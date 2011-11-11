/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
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


#define VSTREAM_DOMAIN "2dmyocard-segment"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <boost/lambda/lambda.hpp>
#include <boost/filesystem.hpp>


//#include <mia/core/fft1d_r2c.hh>
#include <queue>
#include <libxml++/libxml++.h>


#include <mia/core.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/transformfactory.hh>
NS_MIA_USE;

const SProgramDescrption g_description = {
	"Myocardial Perfusion Analysis", 

	"This program is work in progress", 
	
	NULL, NULL
}; 


namespace bfs=boost::filesystem; 

class C2DFImage2PImage {
public: 
	P2DImage operator () (const C2DFImage& image) const {
		return P2DImage(new C2DFImage(image)); 
	}
}; 

class Convert2Float {
public: 
	C2DFImage operator () (P2DImage image) const  {
		return ::mia::filter(m_converter, *image); 		
	}
private: 
	FConvert2DImage2float m_converter; 
}; 

P2DImage evaluate_bridge_mask(P2DImage RV_mask, P2DImage LV_mask)
{
	assert(RV_mask); 
	assert(LV_mask); 
	assert(LV_mask->get_size() == RV_mask->get_size()); 
	
	const C2DBitImage& RV_bin = dynamic_cast<const C2DBitImage&>(*RV_mask); 
	const C2DBitImage& LV_bin = dynamic_cast<const C2DBitImage&>(*LV_mask); 

	C2DBitImage tmp(RV_bin.get_size()); 
	transform(LV_bin.begin(), LV_bin.end(), RV_bin.begin(), tmp.begin(), logical_or<bool>()); 
	
	P2DImage closed  = run_filter(tmp, "close:shape=[sphere:r=10]");
	auto closed_bin = dynamic_cast<const C2DBitImage&>(*closed); 
	
	// substract the original from the closed mask 
	transform(closed_bin.begin(), closed_bin.end(), tmp.begin(), tmp.begin(), 
		  [](bool c, bool org){ return (c && !org);}); 
	
	closed = run_filter(tmp, "label"); 
	return run_filter(*closed, "selectbig");
}

int  get_myocard_minclass(const C2DImage& masked_label)
{
	const auto& label = dynamic_cast<const C2DUBImage&>(masked_label); 
	int result = 0; 
	for (auto l = label.begin(); l != label.end(); ++l) {
		if ( *l > result)
			result = *l; 
	}
	return result; 
}


int get_myocard_label(const C2DImage& masked_label)
{
	const auto& label = dynamic_cast<const C2DUBImage&>(masked_label); 
	int result = 0; 
	for (auto l = label.begin(); l != label.end(); ++l) {
		if (*l) {
			if (!result) 
				result = *l; 
			else {
				assert(result == *l); 
			}
		}
	}
	return result; 
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename; 
	string save_feature; 

	// this parameter is currently not exported - reading the image data is 
	// therefore done from the path given in the segmentation set 
	bool override_src_imagepath = true;

	// ICA parameters 
	size_t components = 0;
	bool normalize = false; 
	bool no_meanstrip = false; 
	size_t skip_images = 2; 
	size_t max_ica_iterations = 400; 

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output myocardial mask", CCmdOption::required));
	options.add(make_opt( save_feature, "save-features", 'f', "save ICA features to files with this name base")); 

	options.set_group("ICA");
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation testing 4 and 5", NULL));
	options.add(make_opt( normalize, "normalize", 0, "normalized ICs", NULL));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves", NULL));
	options.add(make_opt( skip_images, "skip", 'k', "skip images at the beginning of the series "
				    "as they are of other modalities")); 
	options.add(make_opt( max_ica_iterations, "max-ica-iter", 'm', "maximum number of iterations in ICA")); 

	if (options.parse(argc, argv) != CCmdOptionList::hr_no) 
		return EXIT_SUCCESS; 

	// load input data set
	CSegSetWithImages  input_set(in_filename, override_src_imagepath);
	C2DImageSeries input_images = input_set.get_images(); 
	
	vector<C2DFImage> series(input_images.size() - skip_images); 
	transform(input_images.begin() + skip_images, input_images.end(), 
		  series.begin(), Convert2Float()); 
	
	
	int rv_idx = -1; 
	int lv_idx = -1; 
	int perf_idx = -1; 

	
	unique_ptr<C2DPerfusionAnalysis> ica; 
	
	int test_components = components ? components - 1 : 3; 
	do {
		++test_components; 
		ica.reset(new C2DPerfusionAnalysis(test_components, normalize, !no_meanstrip)); 
		
		if (max_ica_iterations) 
			ica->set_max_ica_iterations(max_ica_iterations); 
		
		
		if (!ica->run(series)) {
			ica->set_approach(FICA_APPROACH_SYMM); 
			ica->run(series); 
		}
		
		rv_idx = ica->get_RV_idx(); 
		lv_idx = ica->get_LV_idx(); 
		perf_idx = ica->get_perfusion_idx(); 
		
	} while (!components && (rv_idx < 0 || lv_idx < 0 || perf_idx < 0) && test_components < 6); 

	if (!save_feature.empty()) {
		ica->save_coefs(save_feature + ".txt"); 
		ica->save_feature_images(save_feature); 
	}
	
	if (rv_idx < 0) 
		throw runtime_error("RV could not be identified"); 

	if (rv_idx < 0) 
		throw runtime_error("LV could not be identified"); 

	if (perf_idx < 0) 
		throw runtime_error("perfusion could not be identified"); 

	cvmsg() << "Using " << test_components << " independend components\n"; 
	
	vector<const char *> cavity_segmentation = {
		"kmeans:c=5", 
		"binarize:min=4", 
		"close:shape=[sphere:r=4]", 
		"label",  
		"selectbig"
	}; 
	C2DImageFilterChain cavity_filters(cavity_segmentation); 

	auto RV_feature = ica->get_feature_image(rv_idx); 
	auto LV_feature = ica->get_feature_image(lv_idx); 
	auto perf_feature = ica->get_feature_image(perf_idx); 
	auto mean_feature = ica->get_feature_image(-1); 

	auto RV_mask = cavity_filters.run(RV_feature); 
	auto LV_mask = cavity_filters.run(LV_feature); 

	save_image("rv_mask.png", RV_mask); 
	save_image("lv_mask.png", LV_mask); 
	
	auto RV_LV_bridge_mask = evaluate_bridge_mask(RV_mask, LV_mask); 
	save_image("bridge.@", RV_LV_bridge_mask); 

	auto image_subtractor = C2DImageCombinerPluginHandler::instance().produce("sub"); 
	
	perf_feature = image_subtractor->combine(*perf_feature, *RV_feature); 
	perf_feature = image_subtractor->combine(*perf_feature, *LV_feature); 

	save_image("perf_mrl.v", perf_feature); 

	vector<const char *> myocard_class = {
		"kmeans:c=36", 
		"tee:file=kmeans.@", 
		"open:shape=4n", 
		"mask:input=bridge.@"
	}; 
	C2DImageFilterChain myocard_class_filters(myocard_class); 
		
	auto myocard_kmeans_masked = myocard_class_filters.run(perf_feature);
	auto myocard_kmeans = load_image2d("kmeans.@"); 

	if (!save_feature.empty()) {
		auto kmeans_image_debug = run_filter(*myocard_kmeans, "convert"); 
		save_image(save_feature + "kmeans.png" ,kmeans_image_debug); 
	}
	
	
	int myocard_class_label = get_myocard_minclass(*myocard_kmeans_masked); 
	cvmsg() << "Myocard class id = " << myocard_class_label << "\n"; 
	if (myocard_class_label > 32) 	myocard_class_label = 32; 
	if (myocard_class_label < 4) 
		cverr() << "Myocardial segmentation has probably failed\n"; 
	
	ostringstream filter_descr; 
	filter_descr << "binarize:min=" << myocard_class_label; 

	vector<const char *> next_filters = {
		"replace", 
		"label", 
		"tee:file=label.@",
		"mask:input=bridge.@", 
		"selectbig",
		"mask:input=label.@"
	}; 

	const string help = filter_descr.str(); 
	next_filters[0] = help.c_str(); 
	C2DImageFilterChain myocard_class_filters2(next_filters); 
	
	auto myocard_label_mask = myocard_class_filters2.run(myocard_kmeans); 

	int myocard_label = get_myocard_label(*myocard_label_mask); 
	ostringstream filter_descr2; 
	filter_descr2 << "binarize:min=" << myocard_label << ",max=" << myocard_label; 
	
	// now we have a seed area within the myocardium 
	auto myocard_seed_mask = run_filter(*load_image2d("label.@"), filter_descr2.str().c_str()); 
	

	// classify mean image and save it to get some ideas 
	auto mean_feature_kmeans = run_filter(*mean_feature, "kmeans:c=18"); 
	
	auto myocard_mean_kmeans = image_subtractor->combine(*myocard_kmeans, *mean_feature_kmeans); 
	
	auto mean_feature_kmeans_debug = run_filter(*mean_feature_kmeans, "convert"); 
	save_image(save_feature + "mean_kmeans.png", mean_feature_kmeans_debug);
	save_image(save_feature + "myo-mean_kmeans.png", run_filter(*myocard_mean_kmeans, "convert"));
	

	return save_image(out_filename, myocard_seed_mask) ?  EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

