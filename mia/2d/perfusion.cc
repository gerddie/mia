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

#include <memory>
#include <fstream>
#include <mia/2d/perfusion.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/2dimageio.hh>

NS_MIA_BEGIN
using namespace std; 


/* Implementation class */
struct C2DPerfusionAnalysisImpl {
	C2DPerfusionAnalysisImpl(size_t components, bool normalize, 
				 bool meanstrip); 
	
	vector<C2DFImage> get_references() const; 
	bool run_ica(const vector<C2DFImage>& series);
	P2DFilter get_crop_filter(float scale, C2DBounds& crop_start,
				  C2DPerfusionAnalysis::EBoxSegmentation approach, 
				  const std::string& save_features) const; 
	P2DFilter create_LV_cropper_from_delta(P2DImage rvlv_feature,
					       float LV_mask_amplify,
					       C2DBounds& crop_start,
					       const std::string& save_features)const; 
	
	P2DFilter create_LV_cropper_from_features(float LV_mask_amplify,
						  C2DBounds& crop_start, 
						  const string& save_features)const; 
	
	CICAAnalysis::IndexSet get_all_without_periodic()const; 
	void save_feature(const string& base, const string& feature, const C2DImage& image)const; 
	P2DImage get_rvlv_delta_from_feature(const string& save_features)const; 
	P2DImage get_rvlv_delta_from_peaks(const string& save_features)const; 
	vector<vector<float> > create_guess(size_t rows); 
	void save_coefs(const string&  coefs_name)const; 

	size_t m_components;
	bool m_normalize;  
	bool m_meanstrip; 
	int m_max_iterations; 
	unique_ptr<C2DImageSeriesICA> m_ica; 
	vector<C2DFImage> m_series; 
	C2DBounds m_image_size; 
	CWaveletSlopeClassifier m_cls; 
	size_t m_length; 
	int m_ica_approach; 
	bool m_use_guess_model; 
};



C2DPerfusionAnalysis::C2DPerfusionAnalysis(size_t components, bool normalize, 
					   bool meanstrip):
	impl(new C2DPerfusionAnalysisImpl(components, normalize,  meanstrip))
{
}

C2DPerfusionAnalysis::~C2DPerfusionAnalysis()
{
	delete impl; 
}

void C2DPerfusionAnalysis::set_use_guess_model()
{
	impl->m_use_guess_model = true; 
}

bool C2DPerfusionAnalysis::has_movement() const
{
	return  impl->m_cls.result() == CWaveletSlopeClassifier::wsc_normal ||
		impl->m_cls.result() == CWaveletSlopeClassifier::wsc_low_movement; 
}

void C2DPerfusionAnalysis::set_max_ica_iterations(size_t maxiter)
{
	assert(impl); 
	impl->m_max_iterations = maxiter; 
}

void C2DPerfusionAnalysis::set_approach(size_t approach)
{
	assert(impl); 
	impl->m_ica_approach = approach; 
}


void C2DPerfusionAnalysis::save_coefs(const string&  coefs_name)const
{
	assert(impl); 
	impl->save_coefs(coefs_name); 
}

P2DFilter C2DPerfusionAnalysis::get_crop_filter(float scale, C2DBounds& crop_start,
						EBoxSegmentation approach, 
						const std::string& save_features) const

{
	assert(impl); 
	return impl->get_crop_filter(scale, crop_start, approach, save_features); 
}

bool C2DPerfusionAnalysis::run(const vector<C2DFImage>& series)
{
	assert(impl); 
	return impl->run_ica(series); 
}

vector<C2DFImage> C2DPerfusionAnalysis::get_references() const
{
	assert(impl); 
	return impl->get_references(); 
}


C2DPerfusionAnalysisImpl::C2DPerfusionAnalysisImpl(size_t components, 
						   bool normalize, 
						   bool meanstrip):
	m_components(components), 
	m_normalize(normalize), 
	m_meanstrip(meanstrip),
	m_max_iterations(0),
	m_length(0), 
	m_ica_approach(FICA_APPROACH_DEFL), 
	m_use_guess_model(false)
{
}

P2DImage C2DPerfusionAnalysisImpl::get_rvlv_delta_from_feature(const string& save_features) const 
{
	C2DImageSeriesICA::IndexSet plus;
	C2DImageSeriesICA::IndexSet minus;
	
	plus.insert(m_cls.get_RV_idx());
	minus.insert(m_cls.get_LV_idx());
	
	P2DImage result = m_ica->get_delta_feature(plus, minus); 
	if (!save_features.empty()) {
		save_feature(save_features, "RVic", *m_ica->get_feature_image(m_cls.get_RV_idx())); 
		save_feature(save_features, "LVic", *m_ica->get_feature_image(m_cls.get_LV_idx())); 
		save_feature(save_features, "RVLVica", *result); 
	}
	
	return result; 
}

P2DImage C2DPerfusionAnalysisImpl::get_rvlv_delta_from_peaks(const string& save_features) const 
{
	const int RV_peak = m_cls.get_RV_peak();
	const int LV_peak = m_cls.get_LV_peak();
	
	if (RV_peak < 0 || LV_peak < 0)
		return P2DImage(); 

	C2DFImage *prvlv_diff= new C2DFImage(m_image_size);
	P2DImage result(prvlv_diff); 

	transform(m_series[RV_peak].begin(), m_series[RV_peak].end(),
		  m_series[LV_peak].begin(), prvlv_diff->begin(), 
		  [](float a, float b){return a - b;});

	if (!save_features.empty()) {
		save_feature(save_features, "RVpeak", m_series[RV_peak]); 
		save_feature(save_features, "LVpeak", m_series[LV_peak]); 
		save_feature(save_features, "RVLVdelta", *result); 
	}

	return result; 
}

P2DFilter C2DPerfusionAnalysisImpl::get_crop_filter(float scale, C2DBounds& crop_start,
						    C2DPerfusionAnalysis::EBoxSegmentation approach, 
						    const string& save_features) const
{
	switch (approach) {
	case C2DPerfusionAnalysis::bs_delta_feature: 
		return create_LV_cropper_from_delta(get_rvlv_delta_from_feature(save_features), 
					     scale, crop_start, save_features);
		break; 
	case C2DPerfusionAnalysis::bs_delta_peak: 
		return create_LV_cropper_from_delta(get_rvlv_delta_from_peaks(save_features), 
						    scale, crop_start, save_features);
	case C2DPerfusionAnalysis::bs_features: 
		return create_LV_cropper_from_features(scale, crop_start, save_features);
	default: 
		throw invalid_argument("C2DPerfusionAnalysis::get_crop_filter: unkonwn segmentatoin method given");
	}
}


vector<C2DFImage> C2DPerfusionAnalysisImpl::get_references() const
{
	vector<C2DFImage> result(m_length); 
	CICAAnalysis::IndexSet component_set = get_all_without_periodic(); 
	
	for (size_t i = 0; i < m_length; ++i) 
		result[i] = m_ica->get_partial_mix(i, component_set); 
	return result; 
}

vector<vector<float> > C2DPerfusionAnalysisImpl::create_guess(size_t rows)
{
	vector<vector<float> > result(3); 
	
	float rv_shift = 0.25 * rows;
	float lv_shift = 0.4 * rows;

	vector<float> lv_slope(rows); 
	vector<float> rv_slope(rows); 
	vector<float> perf_slope(rows); 
	for (size_t x = 0; x < rows; ++x) {
		rv_slope[x] = (tanh(12 * x /rows - 0.05 * rows) +   
			       4.0 * exp(-(x-rv_shift) * (x-rv_shift) / rows) - 1)/5.0;
		lv_slope[x] = (tanh(12 * x /rows - 0.05 * rows) +   
			       4.0 * exp(-(x-lv_shift) * (x-lv_shift) / rows) - 1)/5.0;
		perf_slope[x] = tanh(6 * x /rows - 0.05 * rows) / 5.0; 
	}
	result[0] = rv_slope; 
	result[1] = lv_slope; 
	result[2] = perf_slope; 
	return result; 
}

bool C2DPerfusionAnalysisImpl::run_ica(const vector<C2DFImage>& series) 
{
	m_series = series; 
	m_length = series.size(); 
	if (m_length < m_components) 
		throw invalid_argument("C2DPerfusionAnalysis::run_ica: input series too short"); 

	srand(time(NULL));
	m_image_size = series[0].get_size(); 
	bool has_one = false; 
	unique_ptr<C2DImageSeriesICA> ica(new C2DImageSeriesICA(series, false));

	vector<vector<float> > guess; 
	if (m_use_guess_model) 
		guess = create_guess(series.size()); 
	if (m_components > 0) {
		ica->set_max_iterations(m_max_iterations);
		ica->set_approach(m_ica_approach); 
			
		if (!ica->run(m_components, m_meanstrip, m_normalize, guess) && 
		    (m_ica_approach == FICA_APPROACH_DEFL))
			return false; 
		m_cls = CWaveletSlopeClassifier(ica->get_mixing_curves(), false);
	} else {

		float max_energy = -1.0;
		for (int i = 4; i <= 7; ++i) {
			unique_ptr<C2DImageSeriesICA> l_ica(new C2DImageSeriesICA(series, false));
			ica->set_approach(m_ica_approach); 
			l_ica->set_max_iterations(m_max_iterations);

			if (!l_ica->run(i, m_meanstrip, m_normalize, guess) && (m_ica_approach == FICA_APPROACH_DEFL)) {
				cvwarn() << "run_ica: " << i << " components didn't return a result\n"; 
				continue; 
			}

			CWaveletSlopeClassifier cls(l_ica->get_mixing_curves(), false);
			if (cls.result() == CWaveletSlopeClassifier::wsc_fail) {
				cvwarn() << "Classification: for " << i << " components failed\n"; 
				continue; 
			}
			
			float movement_energy = cls.get_movement_indicator();
			cvmsg() << "Components = " << i << " energy = " << movement_energy << "\n";
			if (max_energy < movement_energy) {
				max_energy = movement_energy;
				m_components = i;
				ica.swap(l_ica);
				m_cls = cls; 
			}
			has_one = true; 
		}
	}
	m_ica.swap(ica);
	cvmsg() << "Components: " << m_components << "\n"; 
	cvmsg() << "  Movement: " << m_cls.get_movement_idx() << "\n"; 
	cvmsg() << "  RV:       " << m_cls.get_RV_idx() << "\n"; 
	cvmsg() << "  LV:       " << m_cls.get_LV_idx() << "\n"; 
	cvmsg() << "  Baseline: " << m_cls.get_baseline_idx() << "\n"; 
	cvmsg() << "  Perf    : " << m_cls.get_perfusion_idx() << "\n"; 
	return has_one; 
}

CICAAnalysis::IndexSet C2DPerfusionAnalysisImpl::get_all_without_periodic()const 
{
	assert(m_ica); 
	int movement_index = m_cls.get_movement_idx();

	CICAAnalysis::IndexSet result;
	for (int i = 0; i < (int)m_components; ++i) {
		if (i != movement_index)
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
	GetRegionSize(int label):m_label(label) {
	}
	template <typename T>
	size_t operator() (const T2DImage<T>& image) const {
		size_t n = 0;
		typename T2DImage<T>::const_iterator i = image.begin();
		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
				if (*i == (T)m_label)
					++n;
			}
		return n;
	};
private: 
	int m_label; 
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

void C2DPerfusionAnalysisImpl::save_coefs(const string&  coefs_name) const 
{
	assert(m_ica); 
	auto mix = m_ica->get_mixing_curves();
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


P2DFilter C2DPerfusionAnalysisImpl::create_LV_cropper_from_delta(P2DImage rvlv_feature,
								 float LV_mask_amplify,
								 C2DBounds& crop_start, 
								 const string& save_features)const 
{
	P2DFilter result; 
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

		npixels = ::mia::filter(GetRegionSize(1), *RV);

	} while (10 * npixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y && nc < 5);

	if (!save_features.empty()) {
		save_feature(save_features, "kmeans", *kmeans); 
		save_coefs(save_features + ".txt"); 
	}
	if (nc == 5) {
		cvmsg() << "RV classification failed\n"; 
		return result; 
	}

	C2DFVector RV_center = ::mia::filter(GetRegionCenter(), *RV);

	P2DImage LV_candidates = run_filter_chain(kmeans, 2, LVcandidate_filter_chain);


	int label = ::mia::filter(GetClosestRegionLabel(RV_center), *LV_candidates);
	size_t lv_pixels = ::mia::filter(GetRegionSize(label), *LV_candidates);

	if (10 * lv_pixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y) {
		cvmsg() << "LV classification failed\n"; 
		return result;
	}

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
	if (crop_start.x > LV_center.x ||crop_start.y > LV_center.y) {
		cvmsg() << "Directional assumtion failed\n"; 
		return result;
	}

	mask_lv << "crop:start=[" << crop_start
		<< "],end=[" << C2DBounds(int(LV_center.x + r), int(LV_center.y + r)) << "]";
	cvmsg() << "crop region = '" << mask_lv.str() << "'\n";

	if (!save_features.empty()) {
		save_feature(save_features, "LV_candidates", *LV_candidates); 
		save_feature(save_features, "LV", *LV); 
		save_feature(save_features, "RV", *RV); 
	}
	result = C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());
	return result; 
}


P2DFilter C2DPerfusionAnalysisImpl::create_LV_cropper_from_features(float LV_mask_amplify,
								    C2DBounds& crop_start, 
								    const string& save_features)const 
{
	const char *segment_filter_chain[] = {
		"close:shape=[sphere:r=2]",
		"open:shape=[sphere:r=2]",
		"kmeans:c=7",
		"binarize:min=6,max=6", 
		"label"
	};

	P2DImage RV_candidates = run_filter_chain(m_ica->get_feature_image(m_cls.get_RV_idx()),
						  sizeof(segment_filter_chain)/sizeof(const char*), 
						  segment_filter_chain);
	
	P2DImage RV = run_filter(*RV_candidates, "selectbig"); 
	size_t npixels = ::mia::filter(GetRegionSize(1), *RV);
	
	if (10 * npixels > RV->get_size().x * RV->get_size().y)
		return P2DFilter();

	C2DFVector RV_center = ::mia::filter(GetRegionCenter(), *RV);

	P2DImage LV_candidates = run_filter_chain(m_ica->get_feature_image(m_cls.get_LV_idx()),
						  sizeof(segment_filter_chain)/sizeof(const char*), 
						  segment_filter_chain);

	int label = ::mia::filter(GetClosestRegionLabel(RV_center), *LV_candidates);
	size_t lv_pixels = ::mia::filter(GetRegionSize(label), *LV_candidates);

	if (!save_features.empty()) {
		save_feature(save_features, "RVic", *m_ica->get_feature_image(m_cls.get_RV_idx())); 
		save_feature(save_features, "LVic", *m_ica->get_feature_image(m_cls.get_LV_idx())); 
		save_coefs(save_features + ".txt"); 
	}

	if (10 * lv_pixels > RV->get_size().x * RV->get_size().y) {
		cvmsg() << "RV segmentation failed\n"; 
		return P2DFilter();
	}

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
	if (crop_start.x > LV_center.x ||crop_start.y > LV_center.y) {
		cvmsg() << "LV segmentation failed\n"; 
		return P2DFilter();
	}

	mask_lv << "crop:start=[" << crop_start
		<< "],end=[" << C2DBounds(int(LV_center.x + r), int(LV_center.y + r)) << "]";
	cvmsg() << "crop region = '" << mask_lv.str() << "'\n";

	if (!save_features.empty()) {
		save_feature(save_features, "RV_candidates", *RV_candidates); 
		save_feature(save_features, "LV_candidates", *LV_candidates); 
		save_feature(save_features, "LV", *LV); 
		save_feature(save_features, "RV", *RV); 
	}

	return C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());
}

int C2DPerfusionAnalysis::get_RV_peak_idx() const
{
	return impl->m_cls.get_RV_peak();
}

int C2DPerfusionAnalysis::get_LV_peak_idx() const
{
	return impl->m_cls.get_LV_peak();
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


void C2DPerfusionAnalysisImpl::save_feature(const string& base, const string& feature, const C2DImage& image)const
{
	save_image(base + feature + ".png", run_filter(image, "convert")); 
}

TDictMap<C2DPerfusionAnalysis::EBoxSegmentation>::Table segmethod_table[] ={
	{"delta-feature", C2DPerfusionAnalysis::bs_delta_feature},
	{"delta-peak", C2DPerfusionAnalysis::bs_delta_peak},
	{"features", C2DPerfusionAnalysis::bs_features},      
	{NULL, C2DPerfusionAnalysis::bs_unknown}
};

TDictMap<C2DPerfusionAnalysis::EBoxSegmentation> C2DPerfusionAnalysis::segmethod_dict(segmethod_table); 

NS_MIA_END
