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
#include <boost/filesystem.hpp>


//#include <mia/core/fft1d_r2c.hh>
#include <queue>
#include <libxml++/libxml++.h>


#include <mia/core.hh>
#include <mia/core/meanvar.hh>
#include <mia/core/bfsv23dispatch.hh>
#include <mia/core/tools.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/SegSetWithImages.hh>
#include <mia/2d/fuzzyseg.hh>
#include <mia/2d/perfusion.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/2d/2DDatafield.cxx>

NS_MIA_USE;
using namespace std; 

const SProgramDescription g_description = {
	{pdi_group, "Work in progress"}, 
	{pdi_short, "Work in progress"}, 
	{pdi_description, "This program is work in progress"}, 
	{pdi_example_descr, "Example"}, 
	{pdi_example_code, "Example"}
}; 

namespace bfs=boost::filesystem; 

class CEvaluateSeriesCorrelationToMask: public TFilter<bool> {
public: 
	CEvaluateSeriesCorrelationToMask(const C2DBitImage& mask, size_t len); 
	
	template <typename T> 
	bool operator () (const T2DImage<T>& image); 


	template <typename Iterator>
	C2DFImage get_correlation_image(Iterator begin, Iterator end); 	


private: 
	C2DBounds m_size; 
	const C2DBitImage& m_mask; 

	int m_mask_points; 
	vector<float> m_mask_mean_series; 
	T2DDatafield< vector<float> > m_series; 
}; 


CEvaluateSeriesCorrelationToMask::CEvaluateSeriesCorrelationToMask(const C2DBitImage& mask, size_t len):
	m_size(mask.get_size()), 
	m_mask(mask), 
	m_mask_points(0), 
	m_series(mask.get_size())
{
	m_mask_mean_series.reserve(len); 

	// count the mask pixels
	for_each(mask.begin(), mask.end(), [this](bool pixel){if (pixel) ++m_mask_points; }); 
	

	// fill the image representation of the series 
	for_each(m_series.begin(), m_series.end(), [&len](vector<float>& v){v.reserve(len);}); 
	
	if (!m_mask_points) 
		throw invalid_argument("CEvaluateSeriesCorrelationToMask: got empty mask"); 
	

}

template <typename T> 
bool CEvaluateSeriesCorrelationToMask::operator () (const T2DImage<T>& image)
{
	float mask_mean = 0.0; 
	if (image.get_size() != m_size) {
		throw create_exception<invalid_argument>( "CEvaluateSeriesCorrelationToMask: got image of size ", image.get_size(), 
						" but expected ", m_size); 
	}

	auto ii = image.begin(); 
	auto ie = image.end(); 
	
	auto im = m_mask.begin(); 
	auto is = m_series.begin(); 

	while (ii != ie) {
		if (*im)
			mask_mean += *ii; 
		is->push_back(*ii); 
		++im; 
		++is; 
		++ii; 
	}
	m_mask_mean_series.push_back(mask_mean / m_mask_points); 
	return true; 
}

float correlation(const vector<float>& x, const vector<float>& y) 
{
	assert(x.size() == y.size()); 
	assert(x.size() > 1); 

	float sxx = inner_product(x.begin(),x.end(),x.begin(),0.0f); 
	float syy = inner_product(y.begin(),y.end(),y.begin(),0.0f); 
	float sxy = inner_product(y.begin(),y.end(),x.begin(),0.0f); 
	float sx = accumulate(x.begin(),x.end(), 0.0f); 
	float sy = accumulate(y.begin(),y.end(), 0.0f); 

	auto value = (x.size() * sxy - sx * sy) / 
		(sqrt(x.size() * sxx - sx *sx) * sqrt(x.size() * syy - sy *sy)); 
	return value; 
}

template <typename Iterator>
C2DFImage CEvaluateSeriesCorrelationToMask::get_correlation_image(Iterator begin, Iterator end) 
{

	CEvaluateSeriesCorrelationToMask *self = this; 
	for_each(begin, end, [&self](P2DImage x) {mia::accumulate(*self, *x);});
	
	C2DFImage result(m_size); 

	transform(m_series.begin(), m_series.end(), result.begin(), 
		  [this](const vector<float>& v){ return correlation(m_mask_mean_series, v); });
	return result; 
}



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

class FGetMaxIntensity: public TFilter<float>  {
public: 
	template <typename T> 
	float operator () (const T2DImage<T>& image) const {
		auto max_it = max_element(image.begin(), image.end()); 
		return *max_it; 
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
		throw create_exception<invalid_argument>( "Input image has size ", image.get_size(), 
						" but expect ", m_sum.get_size()); 
	}
	auto vf = get_gradient(image);
	transform(m_sum.begin(), m_sum.end(), vf.begin(), m_sum.begin(), 
		  [](float s, const C2DFVector& v) -> float {float vn = v.norm(); 
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
		throw create_exception<invalid_argument>( "Input image has size ", image.get_size(), 
						" but expect ", m_sum.get_size()); 
	}
	auto vf = get_gradient(image);
	transform(m_sum.begin(), m_sum.end(), vf.begin(), m_sum.begin(), 
		  [](float s, const C2DFVector& v) -> float {float vn = v.norm(); 
			  return s > vn ? s : vn;});
	return 0;
}

C2DFImage FMaxGradients::get_result() const
{
	return m_sum; 
}

class FMaxIntensity: public TFilter<int>  {
public:
	FMaxIntensity(const C2DBounds& size); 

	template <typename T> 
	int operator () (const T2DImage<T>& image); 

	C2DFImage get_result() const; 
private: 
	C2DFImage m_sum; 
}; 

FMaxIntensity::FMaxIntensity(const C2DBounds& size):
	m_sum(size)
{
}

template <typename T> 
int FMaxIntensity::operator () (const T2DImage<T>& image)
{
	if (m_sum.get_size() != image.get_size()) {
		throw create_exception<invalid_argument>( "Input image has size ", image.get_size(), 
						" but expect ", m_sum.get_size()); 
	}

	transform(m_sum.begin(), m_sum.end(), image.begin(), m_sum.begin(), 
		  [](float s, T i){return s > i ? s : i;});
	return 0;
}

C2DFImage FMaxIntensity::get_result() const
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

template <typename Iterator1, typename Iterator2, typename Function>
void for_each_pair(Iterator1 begin1, Iterator1 end1, Iterator2 begin2, Function f)
{
	while (begin1 != end1)
		f(*begin1++, *begin2++); 
}

/*
   Combine input mask ad add use distinct label for each mask. 
   In addition add the boundary of the image as an additional label 
*/
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

/**
   Evaluate the cavity aof a ventricle basd on an enhancement feature image 
   first run a series of filters to extract a seed, and then use a seeded 
   watershed to obtain a better aproximation of the cavity. 
 */

P2DImage evaluate_cavity(P2DImage feature, const C2DImageFilterChain& seed_chain)
{
	auto seed = seed_chain.run(feature); 
	C2DImageSeries prep(1); 
	prep[0] = seed;
	P2DImage new_seed = combine_with_boundary(prep); 
	save_image("prep_seed.@", new_seed); 
	return run_filter_chain(feature, {"sws:seed=prep_seed.@", "binarize:min=1,max=1"});
}

class FMeanVar: public TFilter<pair<double, double> > {
public: 
	FMeanVar(double thresh): m_thresh(thresh){}; 
	
	template <typename Image> 
	pair<double, double> operator () (const Image& image) const {
		pair<double, double> result; 
		int n = 0; 
		auto begin = image.begin(); 
		auto end = image.end(); 
		while (begin != end)  {
			const double help = *begin; 
			if (help != 0.0) {
				result.first += help; 
				result.second += help * help; 
				++n; 
			}
			++begin; 
		}

		if (n > 0)
			result.first /= n; 
		
		if (n > 1) 
			result.second = sqrt((result.second - n * result.first * result.first) / (n - 1));
		else 
			result.second = 0.0; 
		return result; 
	}
private: 
	double m_thresh; 
}; 

static void create_and_save_evaluation_shapes(const string& save_feature, P2DImage final_myocard_mask)
{
	auto inverse_labeled =  run_filter_chain(final_myocard_mask, {"invert", "label", "sort-label"});
	auto label1 = run_filter(inverse_labeled, "binarize:min=1,max=1"); 
	auto label2 = run_filter(inverse_labeled, "binarize:min=2,max=2");
	
	// the "outside" label must have the pixel (0,0) set 
	auto label1_bit = dynamic_cast<const C2DBitImage&>( *label1 ); 
	if ( !label1_bit(0,0) ) 
		swap(label1, label2); 
	
	save_image(save_feature+"-2-LV-endo.png", label2);
	save_image(save_feature+"-2-LV-exo.png", run_filter(label1, "invert")); 

	save_image("LV-endo.@", label2);
	auto endo_line = run_filter_chain(label2, {"dilate:shape=4n", "mask:input=LV-endo.@,inverse=1"}); 
	save_image(save_feature+"-2-LV-endo-line.png", endo_line);
	save_image(save_feature+"-2-LV-endo-distance.v", run_filter(endo_line, "distance"));

	save_image("LV-exo.@", label1);
	auto exo_line = run_filter_chain(label1, {"dilate:shape=4n", "mask:input=LV-exo.@,inverse=1"}); 
	save_image(save_feature+"-2-LV-exo-line.png", exo_line);
	save_image(save_feature+"-2-LV-exo-distance.v", run_filter(exo_line, "distance"));
	
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

	const auto& imageio = C2DImageIOPluginHandler::instance();

	CCmdOptionList options(g_description);
	options.set_group("File-IO"); 
	options.add(make_opt( in_filename, "in-file", 'i', "input perfusion data set", CCmdOption::required));
	options.add(make_opt( out_filename, "out-file", 'o', "output myocardial mask", 
			      CCmdOption::required, &imageio));
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
		  series.begin(), FCopy2DImageToFloatRepn()); 
	
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

	auto conv2ubyte = produce_2dimage_filter("convert");
	
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
	

	// after running ICA get the related feature images 
	auto RV_feature = ica->get_feature_image(rv_idx); 
	auto LV_feature = ica->get_feature_image(lv_idx); 
	auto perf_feature = ica->get_feature_image(perf_idx); 
	auto mean_feature = ica->get_feature_image(-1); 


	// evaluate the RV and LV cavities 
	C2DImageFilterChain seed_chain({"kmeans:c=3", "binarize:min=2", "close:shape=[sphere:r=4]", 
				"label",  "selectbig" });


	auto LV_mask = evaluate_cavity(LV_feature, seed_chain); 
	save_image("LV_mask.@", LV_mask); 

	auto RV_mask = evaluate_cavity(RV_feature, seed_chain); 
	save_image("RV_mask.@", RV_mask); 

	
	
	// evaluate the bridge between RV and LV cavity and store it in the data pool 
	auto RV_LV_bridge_mask = evaluate_bridge_mask(RV_mask, LV_mask); 
	save_image("bridge.@", RV_LV_bridge_mask); 
	
	// evaluate the true myocardial seeds based on the bridge. 
	auto image_subtractor = C2DImageCombinerPluginHandler::instance().produce("sub"); 
	auto perf_feature_minus_cavities = image_subtractor->combine(*perf_feature, *RV_feature); 
	perf_feature_minus_cavities = image_subtractor->combine(*perf_feature_minus_cavities, *LV_feature); 
	auto myocard_seeds = run_filter_chain(perf_feature_minus_cavities, 
					      {	"mask:input=bridge.@,fill=min,inverse=0", 
							      "kmeans:c=5","tee:file=feature_bridge_kmeans.@",
							      "binarize:min=4,max=4", "close:shape=4n" });


	// prepare the seed image for a 4-class seeded watershed 
	C2DImageSeries myo_prep(3); 
	myo_prep[0] = myocard_seeds; 
	myo_prep[1] = LV_mask; 
	myo_prep[2] = RV_mask; 
	P2DImage seed = combine_with_boundary(myo_prep); 
	save_image("seed.@", seed); 

	// prepare the gradient images used for initial watershed segmentation 
	// of the myocardium and the RV-LV cavities. 
	auto image_adder = C2DImageCombinerPluginHandler::instance().produce("add"); 

	auto evalgrad = produce_2dimage_filter("gradnorm:normalize=1"); 
	auto perf_grad = evalgrad->filter(*perf_feature); 
	auto mean_grad = evalgrad->filter(*mean_feature); 
	auto perf_plus_mean_grad = image_adder->combine(*perf_grad, *mean_grad); 

	
	// prepare the filter chain for circle evaluation and test
	C2DImageFilterChain make_circle({"binarize:min=1,max=2", 
				"mask:input=LV_mask.@,inverse=1", 
				"thinning", "pruning", "thinning", "pruning"}); 
	CHasNonzeroPixels count_pixels; 

	// we will run the watershed first on the mean image, then if this fails, on the 
	// perfusion feature image, if if that also fails, we will try the 
	// gradient sum of the both. 
	vector<P2DImage> test_images = {mean_grad, perf_grad, perf_plus_mean_grad}; 
	bool circle_pixels = 0; 
	
	P2DImage ws_from_mean_grad_1; 
	P2DImage myo_binmask_circle; 
	

	for(auto ti = test_images.begin(); !circle_pixels && ti != test_images.end(); ++ti) {
		ws_from_mean_grad_1 = run_filter(**ti, "sws:seed=seed.@,grad=1");
		myo_binmask_circle = make_circle.run(ws_from_mean_grad_1); 
		circle_pixels = mia::filter(count_pixels, *myo_binmask_circle); 
	}

	if (!save_feature.empty()) {
		save_image(save_feature+"-0-RV.png", conv2ubyte->filter(*RV_feature)); 
		save_image(save_feature+"-0-LV.png", conv2ubyte->filter(*LV_feature)); 
		save_image(save_feature+"-0-perf.png", conv2ubyte->filter(*perf_feature)); 
		save_image(save_feature+"-0-mean.png", conv2ubyte->filter(*mean_feature)); 
		
		save_image(save_feature+"-1-RV_mask.png", RV_mask); 
		save_image(save_feature+"-1-LV_mask.png", LV_mask); 
		save_image(save_feature+"-1-RV-LV_bridge.png", RV_LV_bridge_mask); 
		save_image(save_feature+"-1-myocard_seed.png", myocard_seeds); 
		save_image(save_feature+"-1-ws-seed.png", conv2ubyte->filter(*seed)); 
		
		save_image(save_feature+"-1-ws.png", conv2ubyte->filter(*ws_from_mean_grad_1)); 
		save_image(save_feature+"-1-circle.png",  myo_binmask_circle); 
	}

	// we didn't get the circle, so bye bye 
	if (!circle_pixels)
		throw runtime_error("Basic estimation of the myocardial shape failed"); 

	//
	// end of 1st pass, we got the circular shape of the myocardium 
	// 

	//
	// 2nd pass 
	// 

	// 
	// first we improve the LV cavity segmentation by using the time-intensity correlation 
	
	// evaluate the average time-intensity curve for the given LV mask 
	auto LV_bit_mask = dynamic_cast<const C2DBitImage&>(*LV_mask); 
	
	// evaluate a correlation image representing the correlation of each pixel 
	// w.r.t. the average time-intensity curve above 
	CEvaluateSeriesCorrelationToMask correval_lv(LV_bit_mask, input_images.size() - skip_images); 
	C2DFImage LV_corr_image = correval_lv.get_correlation_image(input_images.begin() + skip_images, 
								    input_images.end()); 
	
	// 
	// now do a two-class watershed to improve the LV segmentation
	// use the original LV mask as LV seed, and the myocard-circle as outer seed
	// 
	C2DImageSeries LV_prep(2); 
	LV_prep[0] = LV_mask; 
	LV_prep[1] = myo_binmask_circle; 
	P2DImage LV_seed = combine_with_boundary(LV_prep); 
	
	save_image("lv_seed.@", LV_seed); 
	save_image("lv_mask.@", LV_mask); 
	auto ws_LV_cavity_from_corr = run_filter(LV_corr_image, "sws:seed=lv_seed.@,grad=0");

	// 
	// use the new seed for the LV and the circle for the 2nd pass watershed segmentation 
	// of the myocardium
	// 
	myo_prep[1] = LV_seed = run_filter(*ws_LV_cavity_from_corr, "binarize:min=1,max=1");
	save_image("lv_seed2.@", LV_seed); 
	myo_prep[0] = myo_binmask_circle; 

	seed = combine_with_boundary(myo_prep); 
	save_image("seed.@", seed); 	
	

	// run the watershed on the mean feature and on the perfusion feature 
	auto ws_from_mean_grad = run_filter(*mean_grad, "sws:seed=seed.@,grad=1");
	auto from_mean_binmask = run_filter(*ws_from_mean_grad, "binarize:min=1,max=1");
	auto ws_from_perf_grad = run_filter(*perf_grad, "sws:seed=seed.@,grad=1");

	
	// 
	// final step, clean out the fat at the outer border of the myocardium 
	// 
	// to do so, we use the RV peak image to identify the fatty tissue 
	// 
	auto rv_peak_index = ica->get_RV_peak_time(); 
	if (rv_peak_index < 0) 
		throw runtime_error("For some reason the RV peak index could not be identified"); 
	auto RV_peak_image = input_images[rv_peak_index]; 


	// now we try to get a good segmentation of the fat and subsequently of the 
	// myocardium 
	// we cluster the RV feature image by using a 10-class k-means, 
	// and then we create a mask by trying out various numbers of 
	// classes to be considered background. 

	auto outer_mask_kmeans = run_filter_chain(RV_peak_image, {"kmeans:c=10"});
	
	P2DImage final_myocard_mask; 
	P2DImage test_mask; 
	P2DImage outer_mask; 
	P2DImage from_perf_binmask; 
	int max_class = 2; 

	C2DImageFilterChain final_mask_chain({"binarize:min=1,max=2", 
				"open:shape=[sphere:r=5]", 
				"mask:input=outer_mask.@,inverse=0", "open", "label", "selectbig", 
				"close:shape=[sphere:r=3]"}); 

	do  {
		// first create the background mask by binarizing the k-means classification 
		stringstream mask_binarize; 
		mask_binarize <<  "binarize:min=0,max=" << max_class; 
		outer_mask = run_filter(*outer_mask_kmeans, mask_binarize.str().c_str()); 
		save_image("outer_mask.@", outer_mask); 
		
		// from the  watershed based on the perfusion we create a mask consisting on the 
		// LV and perfusion label, combine it with the above RV-peak enhancement background mask 
		// and clean this mask 
		from_perf_binmask = final_mask_chain.run(ws_from_perf_grad); 
		save_image("mask.@", from_mean_binmask); 	

		// evaluate the myocardium mask by combining the watershed segmentation obtained from 
		// the mean feature image with above mask
		// if the circle obtained from this operation is not empty 
		// the segmentation was successfull
		final_myocard_mask = run_filter(*from_perf_binmask, "mask:input=mask.@,inverse=0");
		test_mask = make_circle.run(final_myocard_mask); 
		max_class++; 	
	}while(!mia::filter(count_pixels, *test_mask) && max_class < 6); 

	

	if (!save_feature.empty()) {
		create_and_save_evaluation_shapes(save_feature, final_myocard_mask); 

		save_image(save_feature+"-2-LV-corr.png", conv2ubyte->filter(LV_corr_image)); 
		save_image(save_feature+"-2-LV_cavity_from_corr.png", LV_seed);
		save_image(save_feature+"-2-seed.png", conv2ubyte->filter(*seed));
		save_image(save_feature+"-2-ws_from_mean_grad.png",  conv2ubyte->filter(*ws_from_mean_grad)); 
		save_image(save_feature+"-2-myocard-from_mean_binmask.png", from_mean_binmask); 
		save_image(save_feature+"-2-ws_from_perf_grad.png", conv2ubyte->filter(*ws_from_perf_grad));
		
		
		save_image(save_feature+"-2-RV_peak_image.png", conv2ubyte->filter(*RV_peak_image)); 
		save_image(save_feature+"-2-RV_peak_kmeans.png", conv2ubyte->filter(*outer_mask_kmeans)); 
		save_image(save_feature+"-2a-outer_mask.png", outer_mask); 
		save_image(save_feature+"-2a-from_perf_binmask.png",  from_perf_binmask); 
		save_image(save_feature+"-2a-myomask-using-kmeans.png",  final_myocard_mask); 

		auto lv_peak_index = ica->get_LV_peak_time(); 
		if (lv_peak_index >= 0) 
			save_image(save_feature+"-2-LV-peak.png", run_filter(*input_images[lv_peak_index], "convert:map=linear,b=20"));
	}

	return save_image(out_filename, final_myocard_mask) ?  EXIT_SUCCESS : EXIT_FAILURE; 
}

#include <mia/internal/main.hh>
MIA_MAIN(do_main); 

