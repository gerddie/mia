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

const SProgramDescription g_description = {
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


class CHasNonzeroPixels: public TFilter<bool>  {
public: 
	template <typename T> 
	bool operator () (const T2DImage<T>& image) const {
		for (auto i = image.begin(); i != image.end(); ++i) {
			if (*i) 
				return true; 
		}
		return false; 
	}
}; 

class FAcuumulateGradients: public TFilter<int>  {
public:
	FAcuumulateGradients(const C2DBounds& size); 

	template <typename T> 
	int operator () (const T2DImage<T>& image); 

	C2DFImage get_result() const; 
private: 
	C2DFImage m_sum; 
}; 

FAcuumulateGradients::FAcuumulateGradients(const C2DBounds& size):
	m_sum(size)
{
}

template <typename T> 
int FAcuumulateGradients::operator () (const T2DImage<T>& image)
{
	if (m_sum.get_size() != image.get_size()) {
		THROW(invalid_argument, "Input image has size " << image.get_size() 
		      << " but expect " << m_sum.get_size()); 
	}
	auto vf = get_gradient(image);
	transform(m_sum.begin(), m_sum.end(), vf.begin(), m_sum.begin(), 
		  [](float s, const C2DFVector& v){float vn = v.norm(); 
			  return s + vn;});
	return 0;
}

C2DFImage FAcuumulateGradients::get_result() const
{
	return m_sum; 
}


class FMaxGradients: public TFilter<int>  {
public:
	FMaxGradients(const C2DBounds& size); 

	template <typename T> 
	int operator () (const T2DImage<T>& image); 

	C2DFImage get_result() const; 
private: 
	C2DFImage m_sum; 
}; 

FMaxGradients::FMaxGradients(const C2DBounds& size):
	m_sum(size)
{
}

template <typename T> 
int FMaxGradients::operator () (const T2DImage<T>& image)
{
	if (m_sum.get_size() != image.get_size()) {
		THROW(invalid_argument, "Input image has size " << image.get_size() 
		      << " but expect " << m_sum.get_size()); 
	}
	auto vf = get_gradient(image);
	transform(m_sum.begin(), m_sum.end(), vf.begin(), m_sum.begin(), 
		  [](float s, const C2DFVector& v){float vn = v.norm(); 
			  return s > vn ? s : vn;});
	return 0;
}

C2DFImage FMaxGradients::get_result() const
{
	return m_sum; 
}

class FAcuumulateImages: public TFilter<int>  {
public:
	FAcuumulateImages(const C2DBounds& size); 

	template <typename T> 
	int operator () (const T2DImage<T>& image); 

	C2DFImage get_result() const; 
private: 
	C2DFImage m_sum; 
}; 

FAcuumulateImages::FAcuumulateImages(const C2DBounds& size):
	m_sum(size)
{
}

template <typename T> 
int FAcuumulateImages::operator () (const T2DImage<T>& image)
{
	if (m_sum.get_size() != image.get_size()) {
		THROW(invalid_argument, "Input image has size " << image.get_size() 
		      << " but expect " << m_sum.get_size()); 
	}

	transform(m_sum.begin(), m_sum.end(), image.begin(), m_sum.begin(), 
		  [](float s, T i){return s + i;});
	return 0;
}

C2DFImage FAcuumulateImages::get_result() const
{
	return m_sum; 
}




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



P2DImage combine_with_boundary(const C2DImageSeries& images)
{
	assert(!images.empty()); 
	assert(images.size() < 254); 
	C2DBounds size = images[0]->get_size(); 

	unsigned char nimages = static_cast<unsigned char>(images.size()); 
	unsigned char boundary_label = nimages + 1; 
	
	cvinfo() << "boundary_label = " << (int)boundary_label << "\n"; 

	vector<C2DBitImage::const_iterator> it(images.size());
	transform(images.begin(), images.end(), it.begin(), 
		  [] (P2DImage pimage) { 
			  return dynamic_cast<C2DBitImage&> (*pimage).begin(); 
		  });
	
	C2DUBImage *seed = new C2DUBImage(size); 
	auto is = seed->begin(); 

	for (size_t y = 0; y < size.y; ++y) 
		for (size_t x = 0; x < size.x; ++x, ++is) {
			if ( y == 0  || x == 0 || y == size.y - 1 || x == size.x - 1) {
				*is = boundary_label;
			}else{
				for (unsigned char k = 0; k < nimages; ++k) {
					if ( *it[k] )
						*is = k + 1; 
				}
			}
			for_each(it.begin(), it.end(), [](C2DBitImage::const_iterator& i){++i;}); 
		}
	
	return P2DImage(seed); 
}

P2DImage grow_seed(P2DImage feature, P2DImage seed)
{
	C2DImageSeries prep(1); 
	prep[0] = seed;
	P2DImage new_seed = combine_with_boundary(prep); 
	save_image("prep_seed.@", new_seed); 
	return run_filter_chain(feature, {"sws:seed=prep_seed.@", "binarize:min=1,max=1"});
}

int do_main( int argc, char *argv[] )
{
	// IO parameters 
	string in_filename;
	string out_filename; 
	string out_filename2; 
	string out_filename3; 
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
	options.add(make_opt( out_filename2, "out-file-mean", 'O', "output myocardial mask created from meangrad + perfgrad "));
	options.add(make_opt( out_filename3, "out-file-gradsum", 'G', "output myocardial mask created from grad sum over all images"));
	options.add(make_opt( save_feature, "save-features", 'f', "save ICA features to files with this name base")); 

	options.set_group("ICA");
	options.add(make_opt( components, "components", 'C', "ICA components 0 = automatic estimation testing 4 and 5"));
	options.add(make_opt( normalize, "normalize", 0, "normalized ICs"));
	options.add(make_opt( no_meanstrip, "no-meanstrip", 0, 
				    "don't strip the mean from the mixing curves"));
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
		if (perf_idx >= 0) 
			components = test_components; 
		
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
	
	vector<const char*> cavity_filters_descr = {
		"kmeans:c=3", "binarize:min=2", "close:shape=[sphere:r=4]", 
		"label",  "selectbig" }; 
	C2DImageFilterChain cavity_filters(cavity_filters_descr); 
	
	auto RV_feature = ica->get_feature_image(rv_idx); 
	auto LV_feature = ica->get_feature_image(lv_idx); 
	auto perf_feature = ica->get_feature_image(perf_idx); 
	auto mean_feature = ica->get_feature_image(-1); 

	auto RV_mask = grow_seed(RV_feature, cavity_filters.run(RV_feature)); 
	auto LV_mask = grow_seed(LV_feature, cavity_filters.run(LV_feature)); 
	


	auto RV_LV_bridge_mask = evaluate_bridge_mask(RV_mask, LV_mask); 
	save_image("bridge.@", RV_LV_bridge_mask); 

	auto image_subtractor = C2DImageCombinerPluginHandler::instance().produce("sub"); 
	auto image_adder = C2DImageCombinerPluginHandler::instance().produce("add"); 

	auto perflvsum = image_adder->combine(*perf_feature, *LV_feature);
	perflvsum = image_adder->combine(*perflvsum, *mean_feature);

	auto evalgrad = produce_2dimage_filter("gradnorm:normalize=1"); 
	auto gperflvsum = evalgrad->filter(*perflvsum);
	auto perf_grad = evalgrad->filter(*perf_feature); 
	auto mean_grad = evalgrad->filter(*mean_feature); 
	auto perf_plus_mean_grad = image_adder->combine(*perf_grad, *mean_grad); 

	auto gperflvsumpmg = image_adder->combine(*gperflvsum, *mean_grad); 
	

	
	auto perf_feature_minus_cavities = image_subtractor->combine(*perf_feature, *RV_feature); 
	perf_feature_minus_cavities = image_subtractor->combine(*perf_feature_minus_cavities, *LV_feature); 
	
	auto myocard_seeds = run_filter_chain(perf_feature_minus_cavities, 
					      {	"mask:input=bridge.@,fill=min", 
						"kmeans:c=5", "binarize:min=4,max=4", "close:shape=4n" });
	
	
	C2DImageSeries myo_prep(3); 
	myo_prep[0] = myocard_seeds; 
	myo_prep[1] = LV_mask; 
	myo_prep[2] = RV_mask; 


	P2DImage seed = combine_with_boundary(myo_prep); 

	save_image("seed.@", seed); 
	
	auto ws_from_perf_plus_mean_grad = run_filter(*perf_plus_mean_grad, "sws:seed=seed.@,grad=1");
	
	vector<const char*> make_circle_descr =  {"thinning", "pruning", "thinning", "pruning"}; 
	C2DImageFilterChain make_circle(make_circle_descr); 
	CHasNonzeroPixels count_pixels; 

	// now run several approaches to get the shape of the myocardium 
	auto myo_binmask = run_filter(*ws_from_perf_plus_mean_grad, "binarize:min=1,max=1");
	auto myo_binmask_circle = make_circle.run(myo_binmask); 
	
	if (!mia::filter(count_pixels, *myo_binmask_circle)) {
		cvmsg() << "Try Myocard + LV\n"; 
		// no circular shape extracted, try something else
		auto myo_and_lv = run_filter(*ws_from_perf_plus_mean_grad, "binarize:min=1,max=2");
		// subtract lv mask 
		auto lv_inverse = run_filter(*LV_mask, "invert");
		save_image("lv_inverse.@", lv_inverse); 
		
		myo_binmask = run_filter(*myo_and_lv, "mask:input=lv_inverse.@,fill=zero");
		myo_binmask_circle = make_circle.run(myo_binmask);
		
		// last try add all non-background pixels and subtract RV and LV cavity 
		if (!mia::filter(count_pixels, *myo_binmask_circle)) {
			cvmsg() << "Try all sections\n"; 
			auto myo_and_lv_rv = run_filter(*ws_from_perf_plus_mean_grad, "binarize:min=1,max=3");
			auto rv_inverse = run_filter(*RV_mask, "invert");
			save_image("rv_inverse.@", rv_inverse);
			myo_binmask = run_filter_chain(myo_and_lv_rv, {"mask:input=rv_inverse.@,fill=zero", 
						"mask:input=lv_inverse.@,fill=zero"}); 
			myo_binmask_circle = make_circle.run(myo_binmask);

			if (!save_feature.empty()) {
				save_image(save_feature + "-myo_and_lv_rv.png", myo_and_lv_rv); 
				save_image(save_feature + "-lv_iask.png", lv_inverse); 
				save_image(save_feature + "-rv_iask.png", rv_inverse); 

			}
		}
	}

	auto convert_to_ubyte = produce_2dimage_filter("convert"); 
	if (!save_feature.empty()) {
		save_image(save_feature + "-all_seeds.png", convert_to_ubyte->filter(*myocard_seeds)); 
		save_image(save_feature + "-myo-seed.png", seed);
		save_image(save_feature + "-perf_mrl.v", perf_feature); 
		save_image(save_feature + "-bridge.png", RV_LV_bridge_mask); 
		save_image(save_feature + "-lv_mask.png", LV_mask); 
		save_image(save_feature + "-rv_mask.png", RV_mask); 
		save_image(save_feature + "_lv_seed.png", LV_mask); 
		save_image(save_feature + "_perflvsum.png", convert_to_ubyte->filter(*perflvsum)); 
		save_image(save_feature + "_mean_grad.png", convert_to_ubyte->filter(*mean_grad)); 
		save_image(save_feature + "_perf_grad.png", convert_to_ubyte->filter(*perf_grad)); 
		save_image(save_feature + "_perf_plus_mean_grad.png", convert_to_ubyte->filter(*perf_plus_mean_grad)); 
		save_image(save_feature + "_myo_circle.png", myo_binmask_circle);
		save_image(save_feature + "_ws_from_perf_plus_mean_grad.png", convert_to_ubyte->filter(*ws_from_perf_plus_mean_grad));
		
		
	}

	// we didn't get the circle, so bye bye 
	if (!mia::filter(count_pixels, *myo_binmask_circle))
		throw runtime_error("Basic estimation of the myocardial shape failed"); 

	// now redo the watershed with the circle shape 
	myo_prep[0] = myo_binmask_circle; 
	seed = combine_with_boundary(myo_prep); 
	save_image("seed.@", seed); 	
	
	auto ws_from_mean_grad = run_filter(*mean_grad, "sws:seed=seed.@,grad=1");
	auto ws_from_perf_grad = run_filter(*perf_grad, "sws:seed=seed.@,grad=1");
	auto from_mean_binmask = run_filter(*ws_from_mean_grad, "binarize:min=1,max=1");


	auto from_perf_binmask = run_filter_chain(ws_from_perf_grad, {"binarize:min=1,max=2", "open:shape=[sphere:r=5]"});
	save_image("mask.@", from_mean_binmask); 
	auto LV_and_myocard_mask = run_filter(*from_perf_binmask, "mask:input=mask.@");
	
	// LV_and_myocard_mask can be used to restrict the search to the LV and directly attached tissue
	// to do so a gradient based region growing will be done -i.e. grow from the circle as long 
	// as the gradient increased and we are inside the mask. 
//	auto grad_help = 


	if (!save_feature.empty()) {
		save_image(save_feature + "_ws_from_mean_grad.png", convert_to_ubyte->filter(*ws_from_mean_grad));
		save_image(save_feature + "_ws_from_perf_grad.png", convert_to_ubyte->filter(*ws_from_perf_grad));
		save_image(save_feature + "_mask_from_mean_grad.png", from_mean_binmask);
		save_image(save_feature + "_mask_from_perf_grad.png", from_perf_binmask);
	}
	

	return save_image(out_filename, LV_and_myocard_mask) ?  EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

