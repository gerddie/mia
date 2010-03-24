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
#include <mia/2d/SegSetWithImages.hh>
#include <libxml++/libxml++.h>

NS_MIA_USE; 

using boost::lambda::_1; 
using boost::lambda::_2; 

auto_ptr<C2DImageSeriesICA> get_ica(vector<C2DFImage>& series, bool strip_mean, size_t& components, bool ica_normalize )
{
	auto_ptr<C2DImageSeriesICA> ica; 
	if (components > 0) {
		ica.reset(new C2DImageSeriesICA(series, false)); 
		ica->run(components);
		if (strip_mean) 
			ica->normalize_Mix();
		if (ica_normalize) 
			ica->normalize();
		return ica; 
	} else {
		float min_cor = 0.0; 
		for (int i = 6; i > 3; --i) {
			auto_ptr<C2DImageSeriesICA> l_ica(new C2DImageSeriesICA(series, false)); 
			l_ica->run(i);
			
			if (strip_mean) 
				l_ica->normalize_Mix();
			if (ica_normalize) 
				l_ica->normalize();
			
			CSlopeClassifier cls(l_ica->get_mixing_curves(), strip_mean); 
			float max_slope = i * cls.get_max_slope_length_diff(); 
			cvinfo() << "Components = " << i << " max_slope = " << max_slope << "\n"; 
			if (min_cor < max_slope) {
				min_cor = max_slope; 
				components = i; 
				ica = l_ica; 
			}
		}
	}
	return ica; 
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
					      C2DIVector& crop_start,
					      C2DIVector& crop_end
					      )
{
	C2DImageSeriesICA::IndexSet plus; 
	C2DImageSeriesICA::IndexSet minus; 
	
	const char *kmeans_filter_chain[] = {
		"close:shape=[sphere:r=2]", 
		"open:shape=[sphere:r=2]", 
		"kmeans:c=5"
	}; 

	const char *RV_filter_chain[] = {
		"binarize:min=4,max=4", 
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
	
	P2DImage kmeans = run_filter_chain(rvlv_feature, 3, kmeans_filter_chain); 

	P2DImage RV = run_filter_chain(kmeans, 3, RV_filter_chain); 
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
	crop_end = C2DBounds(int(LV_center.x + r), int(LV_center.y + r)); 
	mask_lv << "crop:start=[" << crop_start << "],end=[" << crop_end << "]"; 
	cvinfo() << "crop region = '" << mask_lv.str() << "'\n"; 

	return C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());
}

int do_main( int argc, const char *argv[] )
{
	string src_name("segment.set"); 
	string out_name("cropped.set"); 
	string crop_name("crop");
	string coefs_name; 
	size_t first =  2;
	size_t components = 0; 
	bool strip_mean = false; 
	bool ica_normalize = false; 
	float LV_mask = 2.0; // no mask 


	CCmdOptionList options;
	options.push_back(make_opt( src_name, "in-set", 'i', "input segmentation set", "input", false));
	options.push_back(make_opt( out_name, "out-set", 'o', "output segmentation set", "output", false)); 
	options.push_back(make_opt( components, "components", 'c', "nr. of components, 0=estimate automatically", 
				    "components", false));

	options.push_back(make_opt( crop_name, "crop-image-base", 'b', "image name base for cropped images", 
				    "crop-image-base", false));

	options.push_back(make_opt( strip_mean, "strip-mean", 'm', "strip mean image from series", "strip-mean", false));
	options.push_back(make_opt( ica_normalize, "ica_normalize", 'n', "ica_normalize feature images", 
				    "ica_normalize", false)); 

	
	options.push_back(make_opt( first, "skip", 's', "skip images at beginning of series", "skip", false));
	
	options.push_back(make_opt( coefs_name, "coefs", 0, "output mixing coefficients to this file", "coefs", false)); 

	options.push_back(make_opt( LV_mask, "LV-crop-amp", 'L', "LV crop mask amplification, 0.0 = don't crop", 
				    "LV-crop", false)); 

	options.parse(argc, argv);


	CSegSetWithImages  segset(src_name, "");

	const C2DImageSeries& input_series = segset.get_images(); 
	
	vector<C2DFImage> series; 
	FConvert2DImage2float converter; 
	for (size_t i = first; i < input_series.size(); ++i) 
		series.push_back(::mia::filter(converter, *input_series[i]));
	

	cvmsg()<< "Got series of " << series.size() << " images\n"; 
	// always strip mean
	auto_ptr<C2DImageSeriesICA> ica = get_ica(series, strip_mean, components, ica_normalize); 
	CSlopeClassifier::Columns curves = ica->get_mixing_curves(); 

	CICAAnalysis::IndexSet component_set = get_all_without_periodic(curves, strip_mean); 
	
	CSlopeClassifier cls(curves, strip_mean); 
	
	C2DFilterPlugin::ProductPtr cropper; 
	
	// create crop filter and store source files too.
	if (LV_mask > 0.0) {
		C2DIVector crop_start(0,0); 
		C2DIVector crop_end(0,0); 
		
		cropper = create_LV_cropper(*ica, cls, LV_mask, crop_start, crop_end); 
		
		CSegSetWithImages cropped_set = segset.crop(crop_start, crop_end, crop_name); 
		
		auto_ptr<xmlpp::Document> outset(cropped_set.write());
		
		ofstream outfile(out_name.c_str(), ios_base::out );
		if (outfile.good())
			outfile << outset->write_to_string_formatted();
		else
			THROW(runtime_error, "Unable to save " << out_name << "'"); 
	}
	
	for (size_t i = first; i < input_series.size(); ++i) {
		P2DImage reference(new C2DFImage(ica->get_partial_mix(i - first, component_set)));
		if (cropper) 
			reference = cropper->filter(*reference); 
		
		stringstream fname; 
		fname << out_name << setw(4) << setfill('0') << i << ".png"; 

		if (!save_image2d(fname.str(), reference)) 
			THROW(runtime_error, "unable to save " << fname.str() << "\n"); 
		cvdebug() << "wrote '" << fname.str() << "\n"; 
	}

	if (!coefs_name.empty()) 
		save_coefs(coefs_name, *ica); 


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
