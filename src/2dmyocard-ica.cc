/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#define VSTREAM_DOMAIN "2dmyocard"
#include <iomanip>
#include <ostream>
#include <fstream>
#include <map>
#include <ctime>
#include <cstdlib>
#include <mia/core.hh>
#include <mia/core/fft1d_r2c.hh>
#include <queue>

#include <mia/2d/imageio.hh>
#include <mia/2d/filter.hh>
#include <mia/2d/ica.hh>
#include <mia/core/slopeclassifier.hh>

NS_MIA_USE;
using namespace std; 

const SProgramDescription g_description = {
	{pdi_group, "Tools for Myocardial Perfusion Analysis"}, 
	{pdi_short, "Run an ICA analysis on a series of 2D images"}, 

	{pdi_description, "This program is used to run a ICA on a series of myocardial "
	 "perfusion images to create sythetic references that can be used for motion correction "
	 "by image registration. If the aim is to run a full motion compensation then it is "
	 "better to create a segmentation set and use mia-2dmyoica-nonrigid. If the input data "
	 "is given by means of a segmentation set, then on can also use mia-2dmyocard-icaseries.\n"
	 "This program is essentially used to test different options on how to run the ICA for" 
	 "reference image creation."}, 
	
	{pdi_example_descr, "Evaluate the synthetic references from images imageXXXX.exr and save them to "
	 "refXXXX.exr by using five independend components, mean stripping, normalizing, "
	 "and skipping 2 images."}, 
	
	
	{pdi_example_code, "-i imageXXXX.exr -o ref -k 2 -C 5 -m -n"}
}; 

unique_ptr<C2DImageSeriesICA> get_ica(vector<C2DFImage>& series, bool strip_mean,
				      size_t& components, bool ica_normalize, int max_iterations)
{
	srand(time(NULL));
	unique_ptr<C2DImageSeriesICA> ica(new C2DImageSeriesICA(series, false));
	if (components > 0) {
		ica->set_max_iterations(max_iterations);
		ica->run(components, strip_mean, ica_normalize);
	} else {
		// maybe one can use the correlation and create an initial guess by combining
		// highly correlated curves.
		float min_cor = 0.0;
		for (int i = 7; i > 3; --i) {
			unique_ptr<C2DImageSeriesICA> l_ica(new C2DImageSeriesICA(series, false));
			l_ica->set_max_iterations(max_iterations);
			l_ica->run(i, strip_mean, ica_normalize);

			CSlopeClassifier cls(l_ica->get_mixing_curves(), strip_mean);
			float max_slope = /*log2(i) * */ cls.get_max_slope_length_diff();
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


unique_ptr<C2DImageSeriesICA> get_ica_auto(vector<C2DFImage>& series, size_t& max_components )
{
	unique_ptr<C2DImageSeriesICA> ica(new C2DImageSeriesICA(series, false));
	max_components = ica->run_auto(max_components, 3, 0.7);
	return ica;
}

void save_feature_image_unnamed(const string& base, int id, size_t max_comp,
			const C2DImageSeriesICA& ica)
{
	stringstream fname;
	fname << base << "-" << "-" << max_comp <<  id << ".exr";
	if (!save_image(fname.str(), ica.get_feature_image(id)))
		throw create_exception<runtime_error>( "unable to save ", fname.str(), "\n");

}


void save_feature_image(const string& base, const string& descr, int id, size_t max_comp,
			const C2DImageSeriesICA& ica)
{
	stringstream fname;
	if (id >= 0) {
		fname << base << "-" << descr << "-" << max_comp << ".exr";
		cvinfo() << "save id=" << id << " as feature '" << descr << "'\n";
		if (!save_image(fname.str(), ica.get_feature_image(id)))
			throw create_exception<runtime_error>( "unable to save ", fname.str(), "\n");
	}else{
		fname << base << "-" << descr << "-" << max_comp << ".exr";
		P2DImage image(ica.get_mean_image().clone());
		if (!save_image(fname.str(), image))
			throw create_exception<runtime_error>( "unable to save ", fname.str(), "\n");
	}
}

void save_feature_image(const string& base, const string& descr, size_t max_comp, P2DImage image)
{
	stringstream fname;
	fname << base << "-" << descr << "-" << max_comp << ".exr";
	if (!save_image(fname.str(), image))
		throw create_exception<runtime_error>( "unable to save '", fname.str(), "'");
}

void save_feature_image_png(const string& base, const string& descr, P2DImage image)
{
	stringstream fname;
	fname << base << "-" << descr << ".png";
	if (!save_image(fname.str(), image))
		throw create_exception<runtime_error>( "unable to save ", fname.str(), "\n");
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
		throw create_exception<runtime_error>( "unable to save coefficients to '", coefs_name, "'");
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
			throw create_exception<invalid_argument>( "GetRegionCenter: provided an empty region");
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

P2DFilter create_LV_cropper(P2DImage rvlv_feature,
					      float LV_mask_amplify,
					      const string& feature_image_base,
					      C2DBounds& crop_start
					      )
{
	C2DImageFilterChain kmeans_filter_chain({"close:shape=[sphere:r=2]",
				"open:shape=[sphere:r=2]"}); 

	C2DImageFilterChain RV_filter_chain({"label","selectbig"});
	C2DImageFilterChain LVcandidate_filter_chain({"binarize:min=0,max=0","label"});

	P2DImage pre_kmeans = kmeans_filter_chain.run(rvlv_feature);

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

		RV = RV_filter_chain.run(kmeans_binarized);


		npixels = ::mia::filter(GetRegionSize(), *RV);

	} while (10 * npixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y && nc < 5);

	if (nc == 5)
		return P2DFilter();


	P2DImage LV_candidates = LVcandidate_filter_chain.run(kmeans);

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

	// this is ugly and should be replaced
	if (crop_start.x > LV_center.x ||crop_start.y > LV_center.y)
		return P2DFilter();

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
		CSlopeStatistics stat(*i, 0);
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

struct convert2byte {
	unsigned char operator() (float x) const {
		if ( x < 0)
			return 0;
		if ( x > 255)
			return 255;
		return (unsigned char)x;
	}
};

P2DImage convert_to_ubyte(const C2DFImage&  pref)
{
	C2DUBImage *result = new C2DUBImage(pref.get_size());
	transform(pref.begin(), pref.end(), result->begin(), convert2byte());
	return P2DImage(result);
}

int do_main( int argc, char *argv[] )
{
	string src_name("data0000.exr");
	string out_name("ref");
	string out_type("exr");
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
	bool auto_comp = false;
	int max_iterations = 0;

	const auto& imageio = C2DImageIOPluginHandler::instance();
	
	CCmdOptionList options(g_description); 
	options.add(make_opt( src_name, "in-base", 'i', "input file name ofolloing pattern nameXXXX.ext X=numbers" , 
			      CCmdOption::required, &imageio));
	options.add(make_opt( coefs_name, "coefs", 0, "output mixing coefficients to this file"));
	options.add(make_opt( out_name, "out-base", 'o', "output file name base"));

	options.add(make_opt( out_type, imageio.get_supported_suffix_set(), "type", 't',
				    "output file type"));

	options.add(make_opt( first, "skip", 'k', "skip images at beginning of series"));
	options.add(make_opt( last, "end", 'e', "last image in series"));
	options.add(make_opt( components, "components", 'C', "nr. of components, 0=estimate automatically"));
	options.add(make_opt( strip_mean, "strip-mean", 'm', "strip mean image from series"));
	options.add(make_opt( feature_image_base, "save-features", 'f', "save feature image"));

	options.add(make_opt(skip_only_periodic,"strip-periodic",'p', "strip only periodic component"));

	options.add(make_opt(max_iterations,"max-ica-iterations",'x', "max ICA solver iterations"));

	options.add(make_opt( ica_normalize, "ica-normalize", 'n', "ica_normalize feature images"));
	options.add(make_opt( numbered_feature_image, "all-features", 0, "save all feature images to"));

	options.add(make_opt( LV_mask, "LV-crop-amp", 'L', "LV crop mask amplification, 0.0 = don't crop"));


	options.add(make_opt( auto_comp, "auto-components", 'a',
				    "automatic esitmation of number of components based on correlation."
				    " Implies -m and -n (Experimental)"));
	if (options.parse(argc, argv) != CCmdOptionList::hr_no)
		return EXIT_SUCCESS; 


	size_t start_filenum = 0;
	size_t end_filenum  = 0;
	size_t format_width = 0;

	string src_basename = get_filename_pattern_and_range(src_name, start_filenum, end_filenum, format_width);

	if (start_filenum < first)
		start_filenum = first;
	if (end_filenum > last)
		end_filenum = last;

	if (start_filenum + components >= end_filenum) {
		throw create_exception<invalid_argument>( "require at least ", components, " images");
	}

	// load images
	vector<C2DFImage> series;
	FCopy2DImageToFloatRepn converter;
	for (size_t i = start_filenum; i < end_filenum; ++i) {
		string src_name = create_filename(src_basename.c_str(), i);
		P2DImage image = load_image<P2DImage>(src_name);
		if (!image)
			throw create_exception<runtime_error>( "image ", src_name, " not found");

		cvdebug() << "read '" << src_name << "\n";
		series.push_back(converter(*image));
	}



	cvmsg()<< "Got series of " << series.size() << " images\n";
	// always strip mean
	unique_ptr<C2DImageSeriesICA> ica = auto_comp ?
		get_ica_auto(series, components ):
		get_ica(series, strip_mean, components, ica_normalize, max_iterations);
	CSlopeClassifier::Columns curves = ica->get_mixing_curves();

	CICAAnalysis::IndexSet component_set = skip_only_periodic ?
		get_all_without_periodic(curves, strip_mean || auto_comp):
		get_LV_RV_Perfusion(curves);


	// important: mean_stripped applies only if it was done before ICA
	CSlopeClassifier cls(curves, false);

	cvinfo() << "LV series       = " << cls.get_LV_idx() << "\n";
	cvinfo() << "RV series       = " << cls.get_RV_idx() << "\n";
	cvinfo() << "Periodic series = " << cls.get_periodic_idx() << "\n";
	cvinfo() << "Perfusion series = " << cls.get_perfusion_idx() << "\n";
	cvinfo() << "Baseline series = " << cls.get_baseline_idx() << "\n";

	P2DFilter cropper;
	C2DBounds crop_start;
	try {
		// create crop filter and store source files too.
		if (LV_mask > 0.0) {
			C2DImageSeriesICA::IndexSet plus;
			C2DImageSeriesICA::IndexSet minus;

			plus.insert(cls.get_RV_idx());
			minus.insert(cls.get_LV_idx());

			P2DImage rvlv_feature = ica->get_delta_feature(plus, minus);
			cropper = create_LV_cropper(rvlv_feature, LV_mask, feature_image_base, crop_start);

			// feature images do not work, try peak images
			if (!cropper) {
				int RV_peak = cls.get_RV_peak();
				int LV_peak = cls.get_LV_peak();
				if (RV_peak < 0 || LV_peak < 0)
					throw std::runtime_error("Feature images doen't work, and peaks could not be identified");

				cvinfo() << "Using RV peaks =" << RV_peak << " and " << LV_peak << "\n";
				C2DFImage *prvlv_diff = new C2DFImage(series[0].get_size());
				P2DImage rvlv_feature(prvlv_diff);

				transform(series[RV_peak].begin(), series[RV_peak].end(),
					  series[LV_peak].begin(), prvlv_diff->begin(), 
					  [](double x, double y){return y - x;});  
				cropper = create_LV_cropper(rvlv_feature, LV_mask, feature_image_base, crop_start);
			}
			if (!cropper)
				throw std::runtime_error("Neither feature images nor peak images work");
			ofstream crop_idx((crop_name + ".txt").c_str());
			crop_idx << crop_start;
		}
	}
	catch (std::exception& x) {
		cvwarn() << "unable to create cropping filter:'"<< x.what() << "'\n";
		cvwarn() << "use original images instead\n";
	}

	for (size_t i = start_filenum; i < end_filenum; ++i) {
		C2DFImage *pref = new C2DFImage(ica->get_partial_mix(i - start_filenum, component_set));
		P2DImage reference(pref);

		// this is a quick hack that should be improved
		if (out_type == "png")
			reference = convert_to_ubyte(*pref);

		if (LV_mask > 0.0) {
			stringstream scrop_name;
			scrop_name << crop_name << setw(format_width) << setfill('0') << i << "." << out_type;
			string src_name = create_filename(src_basename.c_str(), i);
			if (cropper) {
				reference = cropper->filter(*reference);
				P2DImage crop_source = cropper->filter(*load_image2d(src_name));
				if (!save_image(scrop_name.str(), crop_source))
					throw create_exception<runtime_error>( "unable to save ", scrop_name.str(), "\n");
			}else  {
				if (!save_image(scrop_name.str(), load_image2d(src_name)))
					throw create_exception<runtime_error>( "unable to save ", scrop_name.str(), "\n");
			}
		}
		stringstream fname;
		fname << out_name << setw(format_width) << setfill('0') << i << "." << out_type;

		if (!save_image(fname.str(), reference))
			throw create_exception<runtime_error>( "unable to save ", fname.str(), "\n");
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

		if (cls.get_perfusion_idx() >= 0) {
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
		}
		if (strip_mean||auto_comp)
			save_feature_image(feature_image_base, "mean", -1, components, *ica);
	}
	if (!coefs_name.empty())
		save_coefs(coefs_name, *ica);

	if (!numbered_feature_image.empty())
		for (size_t i = 0; i < components; ++i)
			save_feature_image_unnamed(numbered_feature_image, i, components, *ica);

	return EXIT_SUCCESS;

};

template <typename T>
int GetClosestRegionLabel::operator() (const T2DImage<T>& image) const
{
	map<int, Collector> collector_map;
	auto i = image.begin();
	for (size_t y = 0; y < image.get_size().y; ++y)
		for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
			if (!*i)
				continue;
			auto ic = collector_map.find(*i);
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
	for (auto i = collector_map.begin(); i != collector_map.end(); ++i) {
		i->second.center /= i->second.size;
		float wdist = (i->second.center - m_point).norm() / i->second.size;
		if ( min_weighted_distance > wdist ) {
			min_weighted_distance = wdist;
			label = i->first;
		}
	}
	return label;
};


#include <mia/internal/main.hh>
MIA_MAIN(do_main); 
