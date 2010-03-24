/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 *
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

#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <boost/lambda/lambda.hpp>
#include <mia/core.hh>
#include <mia/core/fft1d_r2c.hh>
#include <queue> 

#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/ica.hh>


NS_MIA_USE; 

using boost::lambda::_1; 
using boost::lambda::_2; 

unique_ptr<C2DImageSeriesICA> get_ica(vector<C2DFImage>& series, bool strip_mean, size_t& components, bool ica_normalize )
{
	unique_ptr<C2DImageSeriesICA> ica(new C2DImageSeriesICA(series, false)); 
	if (components > 0) {
		ica->run(components);
		if (strip_mean) 
			ica->normalize_Mix();
		if (ica_normalize) 
			ica->normalize();
	} else {
		float min_cor = 0.0; 
		for (int i = 7; i > 3; --i) {
			unique_ptr<C2DImageSeriesICA> l_ica(new C2DImageSeriesICA(series, false)); 
			l_ica->run(i);
			
			if (strip_mean) 
				l_ica->normalize_Mix();
			if (ica_normalize) 
				l_ica->normalize();
			
			CSlopeClassifier cls(l_ica->get_mixing_curves(), strip_mean); 
			float max_slope = log2(i) * cls.get_max_slope_length_diff(); 
			cvinfo() << "Components = " << i << " max_slope = " << max_slope << "\n"; 
			if (min_cor < max_slope) {
				min_cor = max_slope; 
				components = i; 
				ica.swap(l_ica); 
			}
		}
	}
	return ica; 
}

void save_feature_image_unnamed(const string& base, int id, size_t max_comp, 
			const C2DImageSeriesICA& ica) 
{
	stringstream fname; 
	fname << base << "-" << "-" << max_comp <<  id << ".exr"; 
	if (!save_image2d(fname.str(), ica.get_feature_image(id))) 
		THROW(runtime_error, "unable to save " << fname.str() << "\n"); 

}


void save_feature_image(const string& base, const string& descr, int id, size_t max_comp, 
			const C2DImageSeriesICA& ica) 
{
	stringstream fname; 
	if (id >= 0) {
		fname << base << "-" << descr << "-" << max_comp << ".exr"; 
		if (!save_image2d(fname.str(), ica.get_feature_image(id))) 
			THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
	}else{
		fname << base << "-" << descr << "-" << max_comp << ".exr"; 
		P2DImage image(ica.get_mean_image().clone()); 
		if (!save_image2d(fname.str(), image)) 
			THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
	}
}

void save_feature_image(const string& base, const string& descr, size_t max_comp, P2DImage image) 
{
	stringstream fname; 
	fname << base << "-" << descr << "-" << max_comp << ".exr"; 
	if (!save_image2d(fname.str(), image)) 
		THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
}

void save_feature_image_png(const string& base, const string& descr, P2DImage image) 
{
	stringstream fname; 
	fname << base << "-" << descr << ".png"; 
	if (!save_image2d(fname.str(), image)) 
		THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
}


void save_coefs(const string&  coefs_name, const C2DImageSeriesICA& ica)
{
	CSlopeClassifier::Columns mix = ica.get_mixing_curves(); 
	ofstream coef_file(coefs_name.c_str()); 
	
	for (size_t r = 0; r < mix[0].size(); ++r) {
		for (size_t c = 0; c < mix.size(); ++c) {
			coef_file   << setw(10) << mix[c][r] << " "; 
		}
		coef_file << "\n"; 
	}
	if (!coef_file.good()) 
		THROW(runtime_error, "unable to save coefficients to " << coefs_name); 
}

CICAAnalysis::IndexSet get_all_without_periodic(const CSlopeClassifier::Columns& curves, bool strip_mean)
{
	CSlopeClassifier cls(curves, strip_mean); 
	int periodic_index = cls.get_periodic_idx();

	CICAAnalysis::IndexSet result; 
	for (int i = 0; i < (int)curves.size(); ++i) {
		if (i != periodic_index) 
			result.insert(i); 
	}
	return result; 
}

class GetRegionCenter: public TFilter<C2DFVector> {
public: 
	template <typename T> 
	C2DFVector operator() (const T2DImage<T>& image) const {
		C2DFVector result; 
		size_t n = 0; 
		typename T2DImage<T>::const_iterator i = image.begin(); 
		for (size_t y = 0; y < image.get_size().y; ++y) 
			for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
				if (*i) {
					result.x += x; 
					result.y += y; 
					++n; 
				}
			}
		if (!n) 
			THROW(invalid_argument, "GetRegionCenter: provided an empty region"); 
		return result / float(n); 
	}; 
}; 


class GetRegionSize: public TFilter<size_t> {
public: 
	template <typename T> 
	size_t operator() (const T2DImage<T>& image) const {
		size_t n = 0; 
		typename T2DImage<T>::const_iterator i = image.begin(); 
		for (size_t y = 0; y < image.get_size().y; ++y) 
			for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
				if (*i)
					++n; 
			}
		return n; 
	}; 
}; 

class GetClosestRegionLabel: public TFilter<int> {
public: 
	GetClosestRegionLabel(const C2DFVector& point):m_point(point){}
	template <typename T> 
	int operator() (const T2DImage<T>& image) const;
private:
	C2DFVector m_point; 
	struct Collector {
		size_t size; 
		C2DFVector center; 
	}; 
	
}; 

typedef pair<float, size_t> element; 

C2DFilterPlugin::ProductPtr create_LV_cropper(const C2DImageSeriesICA& ica, 
					      const CSlopeClassifier& cls, 
					      float LV_mask_amplify, 
					      const string& feature_image_base, 
					      C2DBounds& crop_start
					      )
{
	C2DImageSeriesICA::IndexSet plus; 
	C2DImageSeriesICA::IndexSet minus; 
	
	const char *kmeans_filter_chain[] = {
		"close:shape=[sphere:r=2]", 
		"open:shape=[sphere:r=2]"
	}; 

	const char *RV_filter_chain[] = {
		"label", 
		"selectbig"
	};

	const char *LVcandidate_filter_chain[] = {
		"binarize:min=0,max=0", 
		"label", 
	};

	plus.insert(cls.get_RV_idx()); 
	minus.insert(cls.get_LV_idx()); 
	
	
	P2DImage rvlv_feature = ica.get_delta_feature(plus, minus); 
	
	P2DImage pre_kmeans = run_filter_chain(rvlv_feature, 2, kmeans_filter_chain); 
	
	P2DImage RV; 
	P2DImage kmeans; 
	size_t nc = 1; 
	size_t npixels; 
	do {
		++nc; 
		stringstream kfd; 
		kfd << "kmeans:c=" << 2*nc + 1; 
		kmeans = run_filter(*pre_kmeans, kfd.str().c_str());
		stringstream kfb; 
		kfb << "binarize:min="<<2*nc<<",max="<<2*nc; 
		P2DImage kmeans_binarized = run_filter(*kmeans, kfb.str().c_str());
		
		RV = run_filter_chain(kmeans_binarized, 2, RV_filter_chain); 
		

		npixels = ::mia::filter(GetRegionSize(), *RV);
		
	} while (10 * npixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y && nc < 5); 
	if (nc == 5) 
		throw runtime_error("Unable to get a usefull segmentation of the right ventricle"); 
			
	
	P2DImage LV_candidates = run_filter_chain(kmeans, 2, LVcandidate_filter_chain);

	C2DFVector RV_center = ::mia::filter(GetRegionCenter(), *RV);
	int label = ::mia::filter(GetClosestRegionLabel(RV_center), *LV_candidates);
	
	stringstream binarize_lv;  
	binarize_lv << "binarize:min="<< label << ",max=" << label; 
	cvinfo() << "LV label=  " << label << "\n"; 
	P2DImage LV = run_filter(*LV_candidates, binarize_lv.str().c_str()); 

	C2DFVector LV_center = ::mia::filter(GetRegionCenter(), *LV);

	cvinfo() << "RV center = " << RV_center << "\n"; 
	cvinfo() << "LV center = " << LV_center << "\n"; 
	
	float r = LV_mask_amplify * (LV_center - RV_center).norm(); 
	cvinfo() << "r = " << r << "\n"; 
	stringstream mask_lv;
	crop_start = C2DBounds(int(LV_center.x - r), int(LV_center.y - r)); 

	mask_lv << "crop:start=[" << crop_start
		<< "],end=[" << C2DBounds(int(LV_center.x + r), int(LV_center.y + r)) << "]"; 
	cvinfo() << "crop region = '" << mask_lv.str() << "'\n"; 

	if (!feature_image_base.empty()) {
		save_feature_image_png("feature", "kmeans", kmeans); 
		save_feature_image_png(feature_image_base, "RV", RV); 
		save_feature_image_png(feature_image_base, "LV", LV);
	}
	
	return C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());
}


CICAAnalysis::IndexSet get_LV_RV_Perfusion(const CSlopeClassifier::Columns& curves)
{

	priority_queue<element> sorted; 
	
	element e; 
	for(CSlopeClassifier::Columns::const_iterator i = curves.begin(); 
	    i != curves.end(); ++i, ++e.second) {
		CSlopeStatistics stat(*i); 
		e.first = - stat.get_curve_length() / stat.get_range(); 
		sorted.push(e);
	}

	CICAAnalysis::IndexSet result; 
	
	result.insert(sorted.top().second); 
	cvinfo() << "add " << sorted.top().first << " as " << sorted.top().second << "\n"; 
	sorted.pop(); 
	result.insert(sorted.top().second); 
	cvinfo() << "add " << sorted.top().first << " as " << sorted.top().second << "\n"; 
	sorted.pop(); 
	cvinfo() << "add " << sorted.top().first << " as " << sorted.top().second << "\n"; 
	result.insert(sorted.top().second); 
	return result;
}


int do_main( int argc, const char *argv[] )
{
	string src_name("data0000.exr"); 
	string out_name("ref"); 
	string crop_name("crop"); 
	string coefs_name; 
	size_t first =  2;
	size_t last  = 60;
	size_t components = 0; 
	bool strip_mean = false; 
	bool ica_normalize = false; 
	bool skip_only_periodic = false; 
	string feature_image_base; 
	string numbered_feature_image; 
	float LV_mask = 0.0; // no mask 


	CCmdOptionList options;
	options.push_back(make_opt( src_name, "in-base", 'i', "input file name base", "input", false));
	options.push_back(make_opt( coefs_name, "coefs", 0, "output mixing coefficients to this file", "coefs", false)); 
	options.push_back(make_opt( out_name, "out-base", 'o', "output file name base", "output", false)); 
	options.push_back(make_opt( first, "skip", 's', "skip images at beginning of series", "skip", false));
	options.push_back(make_opt( last, "end", 'e', "last image in series", "end", false));
	options.push_back(make_opt( components, "components", 'c', "nr. of components, 0=estimate automatically", 
				    "components", false));
	options.push_back(make_opt( strip_mean, "strip-mean", 'm', "strip mean image from series", "strip-mean", false));
	options.push_back(make_opt( feature_image_base, "save-features", 'f', "save feature image", 
				    "save feature", false)); 
	
	options.push_back(make_opt(skip_only_periodic,"strip_periodic",'p', "strip only periodic component", "periodic", 
				   false)); 
	options.push_back(make_opt( ica_normalize, "ica_normalize", 'n', "ica_normalize feature images", 
				    "ica_normalize", false)); 
	options.push_back(make_opt( numbered_feature_image, "all-features", 'a', "save all feature images to",
				    "all", false)); 
	
	options.push_back(make_opt( LV_mask, "LV-crop-amp", 'L', "LV crop mask amplification, 0.0 = don't crop", 
				    "LV-crop", false)); 

	options.parse(argc, argv);

	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(src_name, start_filenum, end_filenum, format_width);

	if (start_filenum < first)
		start_filenum = first; 
	if (end_filenum > last) 
		end_filenum = last; 

	if (start_filenum + components >= end_filenum) {
		THROW(invalid_argument, "require at least " << components << " images");  
	}

	// load images
	vector<C2DFImage> series; 
	FConvert2DImage2float converter; 
	for (size_t i = start_filenum; i < end_filenum; ++i) {	
		string src_name = create_filename(src_basename.c_str(), i);
		P2DImage image = load_image2d(src_name);
		if (!image) 
			THROW(runtime_error, "image " << src_name << " not found");  
		
		cvdebug() << "read '" << src_name << "\n"; 
		series.push_back(::mia::filter(converter, *image));
	}

	

	cvmsg()<< "Got series of " << series.size() << " images\n"; 
	// always strip mean
	unique_ptr<C2DImageSeriesICA> ica = get_ica(series, strip_mean, components, ica_normalize); 
	CSlopeClassifier::Columns curves = ica->get_mixing_curves(); 

	CICAAnalysis::IndexSet component_set = skip_only_periodic ? 
		get_all_without_periodic(curves, strip_mean): 
		get_LV_RV_Perfusion(curves); 
	

	// important: mean_stripped applies only if it was done before ICA 
	CSlopeClassifier cls(curves, false); 

	cvinfo() << "LV series       = " << cls.get_LV_idx() << "\n"; 
	cvinfo() << "RV series       = " << cls.get_RV_idx() << "\n"; 
	cvinfo() << "Periodic series = " << cls.get_periodic_idx() << "\n"; 
	cvinfo() << "Perfusion series = " << cls.get_perfusion_idx() << "\n"; 
	cvinfo() << "Baseline series = " << cls.get_baseline_idx() << "\n"; 
	
	C2DFilterPlugin::ProductPtr cropper; 
	C2DBounds crop_start; 
	try {
		// create crop filter and store source files too.
		if (LV_mask > 0.0) {
			cropper = create_LV_cropper(*ica, cls, LV_mask, feature_image_base, crop_start); 
			ofstream crop_idx((crop_name + ".txt").c_str()); 
			crop_idx << crop_start;  
		}
	}
	catch (std::exception& x) {
		cvwarn() << "unable to create cropping filter:'"<< x.what() << "'\n"; 
		cvwarn() << "use original images instead\n"; 
	}

	for (size_t i = start_filenum; i < end_filenum; ++i) {
		P2DImage reference(new C2DFImage(ica->get_partial_mix(i - start_filenum, component_set)));
		if (LV_mask > 0.0) {
			stringstream scrop_name; 
			scrop_name << crop_name << setw(format_width) << setfill('0') << i << ".exr"; 
			string src_name = create_filename(src_basename.c_str(), i);
			if (cropper) {
				reference = cropper->filter(*reference); 
				P2DImage crop_source = cropper->filter(*load_image2d(src_name));
				if (!save_image2d(scrop_name.str(), crop_source)) 
					THROW(runtime_error, "unable to save " << scrop_name.str() << "\n"); 
			}else  {
				if (!save_image2d(scrop_name.str(), load_image2d(src_name))) 
					THROW(runtime_error, "unable to save " << scrop_name.str() << "\n"); 
			}
		}
		stringstream fname; 
		fname << out_name << setw(format_width) << setfill('0') << i << ".exr"; 

		if (!save_image2d(fname.str(), reference)) 
			THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
		cvdebug() << "wrote '" << fname.str() << "\n"; 
	}


	if (!feature_image_base.empty()) {
		if (cls.get_periodic_idx() >= 0) 
			save_feature_image(feature_image_base, "periodic", cls.get_periodic_idx(), components, *ica); 
		save_feature_image(feature_image_base, "LV", cls.get_LV_idx(), components, *ica); 
		save_feature_image(feature_image_base, "RV", cls.get_RV_idx(), components, *ica); 
		{
			C2DImageSeriesICA::IndexSet plus; 
			plus.insert(cls.get_RV_idx()); 
			C2DImageSeriesICA::IndexSet minus; 
			minus.insert(cls.get_LV_idx()); 
			save_feature_image(feature_image_base, "RV-LV", components, 
					   ica->get_delta_feature(plus, minus) );
		}
		if ((strip_mean && components > 4) || (!strip_mean && components > 5)) {
			save_feature_image(feature_image_base, "perfusion", cls.get_perfusion_idx(), 
					   components, *ica);
			C2DImageSeriesICA::IndexSet plus; 
			plus.insert(cls.get_RV_idx()); 
			C2DImageSeriesICA::IndexSet minus; 
			minus.insert(cls.get_LV_idx()); 
			minus.insert(cls.get_perfusion_idx()); 
			save_feature_image(feature_image_base, "RV-perf-RVLV", components, 
					   ica->get_delta_feature(plus, minus)); 
		}
		if ( cls.get_baseline_idx() >= 0) {
			save_feature_image(feature_image_base, "baseline", cls.get_baseline_idx(), components, *ica);
		}else if (strip_mean) 
			save_feature_image(feature_image_base, "mean", -1, components, *ica);
	}
	if (!coefs_name.empty()) 
		save_coefs(coefs_name, *ica); 

	if (!numbered_feature_image.empty()) 
		for (size_t i = 0; i < components; ++i) 
			save_feature_image_unnamed(numbered_feature_image, i, components, *ica);
	
	return EXIT_SUCCESS; 

}; 

int main( int argc, const char *argv[] )
{


	try {
		return do_main(argc, argv); 
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


template <typename T> 
int GetClosestRegionLabel::operator() (const T2DImage<T>& image) const 
{
	map<int, Collector> collector_map; 
	typedef map<int, Collector>::iterator coll_iterator; 
	typename T2DImage<T>::const_iterator i = image.begin(); 
	for (size_t y = 0; y < image.get_size().y; ++y) 
		for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
			if (!*i) 
				continue; 
			coll_iterator ic = collector_map.find(*i); 
			if (ic ==  collector_map.end()) {
				Collector col; 
				col.size = 1; 
				col.center = C2DFVector(x,y); 
				collector_map[*i] = col; 
			} else {
				++ic->second.size; 
				ic->second.center.x += x; 
				ic->second.center.y += y; 
			}
		}
	
	float min_weighted_distance = numeric_limits<float>::max(); 
	int label = 0; 
	for (coll_iterator i = collector_map.begin(); 
	     i != collector_map.end(); ++i) {
		i->second.center /= i->second.size; 
		float wdist = (i->second.center - m_point).norm() / i->second.size; 
		if ( min_weighted_distance > wdist ) {
			min_weighted_distance = wdist;
			label = i->first; 
		}
	}
	return label; 
}; 
