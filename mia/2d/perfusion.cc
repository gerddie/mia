/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <memory>
#include <fstream>
#include <mia/2d/perfusion.hh>
#include <mia/2d/ica.hh>
#include <mia/2d/imageio.hh>

NS_MIA_BEGIN
using namespace std; 


/* Implementation class */
struct C2DPerfusionAnalysisImpl {
	C2DPerfusionAnalysisImpl(size_t components, bool normalize, 
				 bool meanstrip); 
	
	vector<C2DFImage> get_references() const; 
    bool run_ica(const vector<C2DFImage>& series, const CICAAnalysisFactory& icatool);
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
	void save_feature_images(const string&  base_name)const; 
	P2DImage get_feature_image(int index) const; 

	size_t m_components;
	bool m_normalize;  
	bool m_meanstrip; 
	int m_max_iterations; 
	unique_ptr<C2DImageSeriesICA> m_ica; 
	vector<C2DFImage> m_series; 
	C2DBounds m_image_size; 
	CWaveletSlopeClassifier m_cls; 
	size_t m_length; 
    CICAAnalysis::EApproach m_ica_approach;
	bool m_use_guess_model; 
	C2DFImage m_image_attributes; 
	float m_min_movement_frequency;  
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

void C2DPerfusionAnalysis::set_min_movement_frequency(float min_freq) 
{
	assert(impl); 
	impl->m_min_movement_frequency = min_freq; 
}



void C2DPerfusionAnalysis::set_approach(CICAAnalysis::EApproach approach)
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

bool C2DPerfusionAnalysis::run(const vector<C2DFImage>& series, const CICAAnalysisFactory& icatool)
{
	assert(impl); 
    return impl->run_ica(series, icatool);
}

vector<C2DFImage> C2DPerfusionAnalysis::get_references() const
{
	assert(impl); 
	return impl->get_references(); 
}

void C2DPerfusionAnalysis::save_feature_images(const string&  base_name)const
{
	assert(impl); 
	impl->save_feature_images(base_name); 
}

C2DPerfusionAnalysisImpl::C2DPerfusionAnalysisImpl(size_t components, 
						   bool normalize, 
						   bool meanstrip):
	m_components(components), 
	m_normalize(normalize), 
	m_meanstrip(meanstrip),
	m_max_iterations(0),
	m_length(0), 
    m_ica_approach(CICAAnalysis::appr_defl),
	m_use_guess_model(false), 
	m_min_movement_frequency(-1)
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
		save_feature(save_features, "RVic", *get_feature_image(m_cls.get_RV_idx())); 
		save_feature(save_features, "LVic", *get_feature_image(m_cls.get_LV_idx())); 
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
	
	for (size_t i = 0; i < m_length; ++i) {
		result[i] = m_ica->get_partial_mix(i, component_set); 
		result[i].set_attributes(m_image_attributes.begin_attributes(),
					 m_image_attributes.end_attributes()); 
	}
	return result; 
}

vector<vector<float> > C2DPerfusionAnalysisImpl::create_guess(size_t rows)
{
	vector<vector<float> > result(3); 
	
    float rv_shift = 0.25f * rows;
    float lv_shift = 0.4f * rows;

	vector<float> lv_slope(rows); 
	vector<float> rv_slope(rows); 
	vector<float> perf_slope(rows); 
	for (size_t x = 0; x < rows; ++x) {
        rv_slope[x] = (tanh(12 * x /rows - 0.05f * rows) +
                   4.0f * exp(-(x-rv_shift) * (x-rv_shift) / rows) - 1)/5.0f;
        lv_slope[x] = (tanh(12 * x /rows - 0.05f * rows) +
                   4.0f * exp(-(x-lv_shift) * (x-lv_shift) / rows) - 1)/5.0f;
        perf_slope[x] = tanh(6 * x /rows - 0.05f * rows) / 5.0f;
	}
	result[0] = rv_slope; 
	result[1] = lv_slope; 
	result[2] = perf_slope; 
	return result; 
}

bool C2DPerfusionAnalysisImpl::run_ica(const vector<C2DFImage>& series, const CICAAnalysisFactory& icatool)
{
	m_series = series; 
	m_length = series.size(); 
	if (m_length < m_components) 
		throw invalid_argument("C2DPerfusionAnalysis::run_ica: input series too short"); 

	m_image_attributes = series[0]; 
		
    srand(static_cast<unsigned>(time(NULL)));
	m_image_size = series[0].get_size(); 
	bool has_one = false; 
    unique_ptr<C2DImageSeriesICA> ica(new C2DImageSeriesICA(icatool, series, false));

	vector<vector<float> > guess; 
	if (m_use_guess_model) 
		guess = create_guess(series.size()); 
	if (m_components > 0) {
		ica->set_max_iterations(m_max_iterations);
		ica->set_approach(m_ica_approach); 
			
		if (!ica->run(m_components, m_meanstrip, m_normalize, guess) && 
            (m_ica_approach == CICAAnalysis::appr_defl))
			return false; 
		m_cls = CWaveletSlopeClassifier(ica->get_mixing_curves(), false, m_min_movement_frequency);
		if (m_cls.result() != CWaveletSlopeClassifier::wsc_fail)
			has_one = true;
	} else {

		size_t min_components_nonzero = 100;
		for (int i = 6; i >= 4; --i) {
            unique_ptr<C2DImageSeriesICA> l_ica(new C2DImageSeriesICA(icatool, series, false));
			ica->set_approach(m_ica_approach); 
			l_ica->set_max_iterations(m_max_iterations);

            if (!l_ica->run(i, m_meanstrip, m_normalize, guess) && (m_ica_approach == CICAAnalysis::appr_defl)) {
				cvwarn() << "run_ica: " << i << " components didn't return a result\n"; 
				continue; 
			}

			CWaveletSlopeClassifier cls(l_ica->get_mixing_curves(), false, m_min_movement_frequency);
			size_t movement_components  = cls.get_number_of_movement_components();
			if (cls.result() == CWaveletSlopeClassifier::wsc_fail) {
				// save this since we can still process without bounding box creation 
				if (min_components_nonzero == 100) {
					min_components_nonzero = movement_components + 10;
					m_components = i;
					m_cls = cls; 
					ica.swap(l_ica);
					has_one = true; 
				}
				cvwarn() << "LV/RV Classification: for " << i << " components failed\n"; 
				continue; 
			}
			


			cvmsg() << "Components = " << i << " number of components = " << movement_components << "\n";
			if (min_components_nonzero == 100 || 
			    (movement_components < min_components_nonzero && movement_components != 0)) {
				min_components_nonzero = movement_components;
				m_components = i;
				ica.swap(l_ica);
				m_cls = cls; 
				has_one = true; 
			}
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

CICAAnalysis::IndexSet C2DPerfusionAnalysisImpl::get_all_without_periodic() const
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
		unsigned int right= image.get_size().x; 
		unsigned int left = 0; 
		unsigned int top = image.get_size().y;
		unsigned int bottom = 0; 
		typename T2DImage<T>::const_iterator i = image.begin();
		for (size_t y = 0; y < image.get_size().y; ++y)
			for (size_t x = 0; x < image.get_size().x; ++x, ++i) {
				if (*i) {
					if (top > y) top = y; 
					if (bottom < y) bottom = y; 
					if (left < x) left = x; 
					if (right > x) right = x; 
				}
			}
		return C2DFVector((left + right)/ 2.0, (bottom + top) / 2.0);
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
		C2DFVector topleft;
		C2DFVector bottomright;
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
		throw create_exception<runtime_error>("unable to save coefficients to '", coefs_name, "'");
}


P2DFilter C2DPerfusionAnalysisImpl::create_LV_cropper_from_delta(P2DImage rvlv_feature,
								 float LV_mask_amplify,
								 C2DBounds& crop_start, 
								 const string& save_features)const 
{
	P2DFilter result; 
	C2DImageFilterChain kmeans_filter_chain({"close:shape=[sphere:r=2]",
				"open:shape=[sphere:r=2]"});

	C2DImageFilterChain RV_filter_chain({"label", "selectbig"});

	C2DImageFilterChain  LVcandidate_filter_chain({"binarize:min=0,max=0","label"}); 
	
	P2DImage pre_kmeans = kmeans_filter_chain.run(rvlv_feature);

	P2DImage RV;
	P2DImage kmeans;
	size_t nc = 3;
	size_t npixels;
	C2DFVector RV_center; 
	P2DImage LV_candidates; 
	size_t lv_pixels;
	int label;  
retry:
	do {
		do {
			++nc;
			stringstream kfd;
			kfd << "kmeans:c=" << 2*nc + 1;
			kmeans = run_filter(*pre_kmeans, kfd.str().c_str());
			stringstream kfb;
			kfb << "binarize:min="<<2*nc<<",max="<<2*nc;
			P2DImage kmeans_binarized = run_filter(*kmeans, kfb.str().c_str());
			
			RV = RV_filter_chain.run(kmeans_binarized);
			
			npixels = ::mia::filter(GetRegionSize(1), *RV);
			
		} while (10 * npixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y && nc < 7);
		
		if (!save_features.empty()) {
			save_feature(save_features, "kmeans", *kmeans); 
			save_coefs(save_features + ".txt"); 
		}
		if (nc == 7) {
			cvmsg() << "RV classification failed\n"; 
			return result; 
		}
		
		RV_center = ::mia::filter(GetRegionCenter(), *RV);

		LV_candidates = LVcandidate_filter_chain.run(kmeans);
		
		
		label = ::mia::filter(GetClosestRegionLabel(RV_center), *LV_candidates);
		lv_pixels = ::mia::filter(GetRegionSize(label), *LV_candidates);
		
	} while (10 * lv_pixels > rvlv_feature->get_size().x * rvlv_feature->get_size().y && nc < 7);
	
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

	C2DFVector delta_center = LV_center - RV_center; 
	
	C2DFVector physical_distance2d = m_image_attributes.get_pixel_size() * delta_center; 
	float physical_distance = physical_distance2d.norm(); 

	cvinfo() << "LV-RV center distance = " << physical_distance << "\n";
	if ( physical_distance < 30.0) {
		cvwarn() << "Distance between LV and RV centers < 3cm, assuming bad segmentation\n"; 
		return P2DFilter();		
	}
	


	float r = LV_mask_amplify * delta_center.norm();
	cvinfo() << "r = " << r << "\n";
	stringstream mask_lv;
	crop_start = C2DBounds(int(LV_center.x - r), int(LV_center.y - r));

	// this is ugly and should be replaced
	if (crop_start.x > LV_center.x ||crop_start.y > LV_center.y) {
		if (nc < 7) 
			goto retry; 
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
	C2DImageFilterChain  segment_filter_chain({
			"close:shape=[sphere:r=2]",
				"open:shape=[sphere:r=2]",
				"kmeans:c=7",
				"binarize:min=6,max=6", 
				"label"	});
	
	P2DImage RV_candidates = segment_filter_chain.run(get_feature_image(m_cls.get_RV_idx())); 
	
	P2DImage RV = run_filter(*RV_candidates, "selectbig"); 
	size_t npixels = ::mia::filter(GetRegionSize(1), *RV);
	
	if (10 * npixels > RV->get_size().x * RV->get_size().y)
		return P2DFilter();

	C2DFVector RV_center = ::mia::filter(GetRegionCenter(), *RV);

	P2DImage LV_candidates = segment_filter_chain.run(get_feature_image(m_cls.get_LV_idx()));

	int label = ::mia::filter(GetClosestRegionLabel(RV_center), *LV_candidates);
	size_t lv_pixels = ::mia::filter(GetRegionSize(label), *LV_candidates);

	if (!save_features.empty()) {
		save_feature(save_features, "RVic", *get_feature_image(m_cls.get_RV_idx())); 
		save_feature(save_features, "LVic", *get_feature_image(m_cls.get_LV_idx())); 
		save_feature(save_features, "RV_candidates", *RV_candidates); 
		save_feature(save_features, "LV_candidates", *LV_candidates); 

		save_coefs(save_features + ".txt"); 
	}

	if (10 * lv_pixels > RV->get_size().x * RV->get_size().y) {
		cvmsg() << "LV segmentation failed\n"; 
		return P2DFilter();
	}

	stringstream binarize_lv;
	binarize_lv << "binarize:min="<< label << ",max=" << label;
	cvinfo() << "LV label=  " << label << "\n";
	P2DImage LV = run_filter(*LV_candidates, binarize_lv.str().c_str());

	C2DFVector LV_center = ::mia::filter(GetRegionCenter(), *LV);

	cvinfo() << "RV center = " << RV_center << "\n";
	cvinfo() << "LV center = " << LV_center << "\n";

	C2DFVector delta_center = LV_center - RV_center; 
	
	C2DFVector physical_distance2d = m_image_attributes.get_pixel_size() * delta_center; 
	float physical_distance = physical_distance2d.norm(); 
	cvinfo() << "LV-RV center distance = " << physical_distance << "\n";

	if ( physical_distance < 30.0) {
		cvwarn() << "Distance between LV and RV centers < 3cm, assuming bad segmentation\n"; 
		return P2DFilter();		
	}


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
		save_feature(save_features, "LV", *LV); 
		save_feature(save_features, "RV", *RV); 
	}

	return C2DFilterPluginHandler::instance().produce(mask_lv.str().c_str());
}


int C2DPerfusionAnalysis::get_RV_peak_time() const
{
	return impl->m_cls.get_RV_peak(); 
}

int C2DPerfusionAnalysis::get_RV_peak_idx() const
{
	return impl->m_cls.get_RV_idx();
}

int C2DPerfusionAnalysis::get_RV_idx() const
{
	return impl->m_cls.get_RV_idx();
}


int C2DPerfusionAnalysis::get_LV_peak_time() const
{
	return impl->m_cls.get_LV_peak(); 
}

int C2DPerfusionAnalysis::get_LV_peak_idx() const
{
	return impl->m_cls.get_LV_idx();
}

int C2DPerfusionAnalysis::get_LV_idx() const
{
	return impl->m_cls.get_LV_idx();
}

int C2DPerfusionAnalysis::get_perfusion_idx() const
{
	return impl->m_cls.get_perfusion_idx();
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
				col.bottomright = col.topleft = C2DFVector(x,y);
				collector_map[*i] = col;
			} else {
				++ic->second.size;
				if (ic->second.bottomright.x < x) 
					ic->second.bottomright.x = x; 
				if (ic->second.bottomright.y < y) 
					ic->second.bottomright.y = y; 

				if (ic->second.topleft.x > x) 
					ic->second.topleft.x = x; 
				if (ic->second.topleft.y > y) 
					ic->second.topleft.y = y; 
			}
		}

	float min_weighted_distance = numeric_limits<float>::max();
	int label = 0;
	for (coll_iterator i = collector_map.begin();
	     i != collector_map.end(); ++i) {
		i->second.center = 0.5f * (i->second.bottomright + i->second.topleft);
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
	save_image(base + feature + ".png", run_filter(image, "convert:repn=ubyte,map=opt")); 
}

void C2DPerfusionAnalysisImpl::save_feature_images(const string&  base_name)const
{
	for (size_t i = 0; i < m_ica->get_mixing_curves().size(); ++i) {
		stringstream feat; 
		feat << "_" << i; 
		save_feature(base_name, feat.str(), *get_feature_image(i)); 
	}
	save_feature(base_name, "_mean", *get_feature_image(-1));
}

P2DImage C2DPerfusionAnalysisImpl::get_feature_image(int index) const
{
	auto img = (index < 0) ?  P2DImage(m_ica->get_mean_image().clone()):
		m_ica->get_feature_image(index); 
	img->set_attributes(m_image_attributes.begin_attributes(),
			     m_image_attributes.end_attributes()); 
	return img; 
}

P2DImage C2DPerfusionAnalysis::get_feature_image(int index) const
{
	return impl->get_feature_image(index); 
}


static TDictMap<C2DPerfusionAnalysis::EBoxSegmentation>::Table segmethod_table[] ={
	{"delta-feature", C2DPerfusionAnalysis::bs_delta_feature, "difference of the feature images"},
	{"delta-peak", C2DPerfusionAnalysis::bs_delta_peak, "difference of the peak enhancement images"},
	{"features", C2DPerfusionAnalysis::bs_features, "feature images"},      
	{NULL, C2DPerfusionAnalysis::bs_unknown, ""}
};

TDictMap<C2DPerfusionAnalysis::EBoxSegmentation> C2DPerfusionAnalysis::segmethod_dict(segmethod_table); 

NS_MIA_END
