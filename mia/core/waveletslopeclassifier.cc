/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2014 Gert Wollny
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

#include <map>
#include <algorithm>
#include <stdexcept>
#include <cmath>

#include <mia/core/msgstream.hh>

#include <boost/algorithm/minmax_element.hpp>


#include <mia/core/waveletslopeclassifier.hh>
#include <mia/core/slopestatistics.hh>


NS_MIA_BEGIN

using namespace std;
using namespace boost;


struct CWaveletSlopeClassifierImpl {
	int RV_peak;
	int LV_peak;
	int RV_idx;
	int LV_idx;
	int Movement_idx;
	int Perfusion_idx;
	int Baseline_idx;
	bool free_breathing; 
	float max_movment_energy; 
	size_t n_movement_components; 
	
	CWaveletSlopeClassifier::EAnalysisResult result; 

	typedef vector<float>::const_iterator position;
	typedef pair<position, position> extrems;
	typedef pair<size_t, size_t> extrems_pos;
	CWaveletSlopeClassifierImpl(const CWaveletSlopeClassifier::Columns& series, bool mean_stripped, float min_freq);
	CWaveletSlopeClassifierImpl(); 
};


CWaveletSlopeClassifier::CWaveletSlopeClassifier(const CWaveletSlopeClassifier::Columns& m, bool mean_stripped, float min_freq)
{
	if (m.size() < 3)
		throw invalid_argument("CWaveletSlopeClassifier: require at least 3 curves");
	impl = new CWaveletSlopeClassifierImpl(m, mean_stripped, min_freq);
}

CWaveletSlopeClassifier::CWaveletSlopeClassifier(const CWaveletSlopeClassifier& other):
	impl(new CWaveletSlopeClassifierImpl(*other.impl)) 
{
}
CWaveletSlopeClassifier::CWaveletSlopeClassifier():
	impl(new CWaveletSlopeClassifierImpl())
{
}
	     
CWaveletSlopeClassifier& CWaveletSlopeClassifier::operator =(const CWaveletSlopeClassifier& other)
{
	if (this != &other) {
		auto help = new CWaveletSlopeClassifierImpl(*other.impl); 
		delete impl; 
		impl = help; 
	}
	return *this;
}

CWaveletSlopeClassifier::~CWaveletSlopeClassifier()
{
	delete impl;
}

CWaveletSlopeClassifier::EAnalysisResult CWaveletSlopeClassifier::result() const
{
	assert(impl); 
	return impl->result;
}


int CWaveletSlopeClassifier::get_movement_idx() const
{
	return impl->Movement_idx;
}

int CWaveletSlopeClassifier::get_perfusion_idx() const
{
	return impl->Perfusion_idx;
}


int CWaveletSlopeClassifier::get_RV_idx()const
{
	return impl->RV_idx;
}

int CWaveletSlopeClassifier::get_LV_idx() const
{
	return impl->LV_idx;
}

int CWaveletSlopeClassifier::get_RV_peak()const
{
	return impl->RV_peak;
}

int CWaveletSlopeClassifier::get_LV_peak() const
{
	return impl->LV_peak;
}

int CWaveletSlopeClassifier::get_baseline_idx() const
{
	return impl->Baseline_idx;
}


float CWaveletSlopeClassifier::get_movement_indicator() const 
{
	return impl->max_movment_energy;
}

size_t CWaveletSlopeClassifier::get_number_of_movement_components() const
{
	return impl->n_movement_components; 
}

CWaveletSlopeClassifierImpl::CWaveletSlopeClassifierImpl():
	RV_peak(-1), 
	LV_peak(-1), 
	RV_idx(-1), 
	LV_idx(-1), 
	Movement_idx(-1), 
	Perfusion_idx(-1), 
	Baseline_idx(-1), 
	free_breathing(false), 
	max_movment_energy(0.0), 
	n_movement_components(0), 
	result(CWaveletSlopeClassifier::wsc_fail)
{
}

CWaveletSlopeClassifierImpl::CWaveletSlopeClassifierImpl(const CWaveletSlopeClassifier::Columns& series, bool mean_stripped, float min_freq):
	RV_peak(-1), 
	LV_peak(-1), 
	RV_idx(-1), 
	LV_idx(-1), 
	Movement_idx(-1),
	Perfusion_idx(-1),
	Baseline_idx(-1), 
	free_breathing(false), 
	max_movment_energy(0.0), 
	n_movement_components(0), 
	result(CWaveletSlopeClassifier::wsc_fail)

{
	vector<PSlopeStatistics> vstats; 
	for (unsigned int i = 0; i < series.size(); ++i)
		vstats.push_back(PSlopeStatistics(new CSlopeStatistics(series[i], i))); 
	
	// 
	// the movement wavelet level is the second higest one (this is opposite to the 
	// usual wavelet notation) 
	int movement_idx = vstats[0]->get_level_coefficient_sums().size() - 2;
	
	// first estimate if this is free breathing or breath holding 
	vector<float> movement_pos(4, 0);

	for (size_t i = 0; i < series.size(); ++i) {
		movement_pos[vstats[i]->get_level_mean_energy_position()[movement_idx]] += vstats[i]->get_level_coefficient_sums()[movement_idx];
		
	}
	
	cvinfo() << "Energy sums = " << movement_pos << "\n"; 


	cvdebug() << "Movement coeff weights:" << movement_pos << "\n"; 
	
	int skip_idx = 0; 
	bool ifree_breathing = false; 
	if (min_freq != 0.0) {
		((movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_begin]) &&
		 (movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_end]));
		
		// handle free breathing and series that only end with breath holding in the same way 
		if  ((!ifree_breathing) && (movement_pos[CSlopeStatistics::ecp_end] < movement_pos[CSlopeStatistics::ecp_begin]))
			ifree_breathing = true; 
	}else 
		skip_idx = 1; 
	

	if (ifree_breathing) {
		cvmsg() << "Detected data set with (initial) free breathing.\n"; 
	}else {
		cvmsg() << "Detected data set with initial breath holding.\n"; 
	}
	
	int low_energy_start_idx = 1; //ifree_breathing ? 0 : 1;
	
	// 
	// get movement components based on wavelet coefficient sums
	//
	vector<PSlopeStatistics> movement_indices;
	vector<PSlopeStatistics> remaining_indices;

	int min_range_idx = -1; 
	float min_range  = numeric_limits<float>::max(); 

	bool got_movement = false; 
	vector<bool> is_high_freq(series.size()); 
	for (size_t i = 0; i < series.size(); ++i) {
		auto e = vstats[i]->get_level_coefficient_sums(); 
		cvdebug() << "energies: "  << e 
			  << " start= "<< low_energy_start_idx
			  << " end= " << movement_idx - 1
			  << "\n"; 

		float low_freq = accumulate(e.begin() + low_energy_start_idx, e.begin() + movement_idx-skip_idx, 0.0); 
		float high_freq = e[movement_idx] + e[movement_idx + 1];
		
		if (min_range > vstats[i]->get_range()) {
			min_range = vstats[i]->get_range(); 
			min_range_idx = vstats[i]->get_index(); 
			cvinfo() << "New minrange: " << min_range << " at idx = " << min_range_idx << "\n"; 
		}

		cvinfo() << "Slope[" << vstats[i]->get_index() 
			 << "] LF " << low_freq 
			 << " HF = " << high_freq 
			 << " range = " << vstats[i]->get_range() 
			 << "\n"; 

		is_high_freq[i] = (low_freq < high_freq);

		if (is_high_freq[i]) {
			if (vstats[i]->get_mean_energy_position() == CSlopeStatistics::ecp_begin) {
				cvinfo() << "c=" << i << ":override motion because we assume it's RV enhacement\n";
				is_high_freq[i] = false; 
			}
		}
		got_movement |= is_high_freq[i]; 
	}
	
	// got no movement, try with frequency analysis 
	if (!got_movement && min_freq > 0.0f) {
		for (size_t i = 0; i < series.size(); ++i) {
			is_high_freq[i] = vstats[i]->get_mean_frequency() > min_freq; 
			if (is_high_freq[i]) {
				if (vstats[i]->get_mean_energy_position() == CSlopeStatistics::ecp_begin) {
					cvinfo() << "c=" << i << ":override motion because we assume it's RV enhacement\n";
					is_high_freq[i] = false; 
				}
				cvinfo() << "Slope " << i << " << detected as movement based on mean frequency slot " << vstats[i]->get_mean_frequency() << "\n"; 
			}else 
				cvinfo() << "Slope " << i << " << rejected as movement based on mean frequency slot " << vstats[i]->get_mean_frequency() << "\n"; 
		}
	}
	
	

	// if the mean is stripped, the baseline vanishes 
	if (mean_stripped) {
		cvinfo() << "mean stripped hence no baseline\n"; 
		min_range_idx = -1; 
	}

	// now store the componnets according to their classification but ignore the baseline 
	// if it is identified as movement 
	for (size_t i = 0; i < series.size(); ++i) {
		if (is_high_freq[i]) {
			if (vstats[i]->get_index() != min_range_idx) 
				movement_indices.push_back(vstats[i]); 
			else {
				Baseline_idx = min_range_idx; 
				cvinfo() << "Baseline " << min_range_idx <<  " with range  " << min_range << "\n"; 
				min_range_idx = -1; 
			}
				
		} else 
			remaining_indices.push_back(vstats[i]); 
	}
	n_movement_components = movement_indices.size(); 

	// 
	// end of movement component estimation 
	// 

	vector<int> mi(movement_indices.size()); 
	transform(movement_indices.begin(), movement_indices.end(), mi.begin(), 
		  [](PSlopeStatistics i){return i->get_index();}); 
	cvinfo() << "Detect movement candidates " << mi << "\n"; 
	
	// 
	// find movement component with the largest energy
	// 
	if (!movement_indices.empty()) {
		result = CWaveletSlopeClassifier::wsc_normal; 
		max_movment_energy = 0.0;
		int max_movment_idx = -1;
		
		for (auto k = movement_indices.begin(); k != movement_indices.end(); ++k) {
			float energy = (*k)->get_level_coefficient_sums()[movement_idx]; 
			if (max_movment_energy < energy ) {
				max_movment_energy = energy; 
				max_movment_idx = (*k)->get_index();
			}
		}
		
		// if no special movement component could be identified, or the component has an 
		// movement energy below the baseline mark it as no movement 
		if (max_movment_idx < 0 || max_movment_idx == Baseline_idx) {
			result = CWaveletSlopeClassifier::wsc_no_movement; 
		} else {
			// if the movement energy is below one of the remaining components 
			// then mark it as low movement, currently this is treated as "normal movememnt" 
			// but one my reconsider ... 
			for (auto k = remaining_indices.begin(); k != remaining_indices.end(); ++k) {
				if ((*k)->get_level_coefficient_sums()[movement_idx] > max_movment_energy) {
					cvinfo() << "Estimated maximum movement component " << max_movment_idx 
						 << " has less movement frequency energy than other component " << (*k)->get_index() << "\n";
					result = CWaveletSlopeClassifier::wsc_low_movement; 
				}
			}
		}
		Movement_idx = max_movment_idx; 
		cvmsg() << "max movement idx = " << Movement_idx
			<< " with movment energy " << max_movment_energy 
			<< " and full energy " <<  vstats[Movement_idx]->get_wavelet_energy()
			<< "\n"; 
	}else{
		cvmsg() << "No movement component identified\n"; 
		result = CWaveletSlopeClassifier::wsc_no_movement; 
		
	}

	vector<int> ri(remaining_indices.size()); 
	transform(remaining_indices.begin(), remaining_indices.end(), ri.begin(), 
		  [](PSlopeStatistics i){return i->get_index();}); 
	cvinfo() << "Remaining elements: " << ri << "\n"; 
	
	// 
	// identify  LV and RV 
	// 

	if (remaining_indices.size() < 2) {
		cvmsg() << "Classification failed because too few remaining components.\n"; 
		result = CWaveletSlopeClassifier::wsc_fail; 
		return; 
	}

	const int start_movement = (ifree_breathing) ? 0 : (2 * series[0].size()) / 3; 
	
	cvinfo() << "set start movement to "  << start_movement << "\n"; 


	// Properly identifying the RV and LV peak is probably the most difficult task. 
	// this is one option how to do it and it seems quite stable, but it is not perfect. 
	sort(remaining_indices.begin(), remaining_indices.end(), 
	     [&start_movement](PSlopeStatistics lhs, PSlopeStatistics rhs) -> bool {
		     auto lhsgp = lhs->get_gradient_peak(start_movement); 
		     auto rhsgp = rhs->get_gradient_peak(start_movement); 
		     return lhsgp.second / lhsgp.first > rhsgp.second / rhsgp.first;
	     });
	
	
	sort(remaining_indices.begin(), remaining_indices.begin() + 2, 
	     [&start_movement](PSlopeStatistics lhs, PSlopeStatistics rhs) 
	     { return lhs->get_gradient_peak(start_movement).first < rhs->get_gradient_peak(start_movement).first;});

	RV_idx  = remaining_indices[0]->get_index(); 
	LV_idx  = remaining_indices[1]->get_index();
	// 
	// this will only be needed if the LV-ROI segmentation is based on the 
	// RV/LV peak images 
	// 
	RV_peak = remaining_indices[0]->get_perfusion_high_peak().first; 
	LV_peak = remaining_indices[1]->get_perfusion_high_peak().first; 
	

	// that's just assuming, if more then 3 components are 
	// non-movement, then this dosn't have to be myocardial perfusion 
	if (remaining_indices.size() > 2) {
		// first we remove the LV and RV curves 
		
		auto new_end = 
			remove_if(remaining_indices.begin(), remaining_indices.end(), 
				  [this](PSlopeStatistics i) {
					  return i->get_index() == RV_idx || i->get_index() == LV_idx;
				  }); 
		remaining_indices.erase(new_end, remaining_indices.end()); 
		
		// we actually re-calculate the level change every time, 
		// but thes are only 2 or three components enyway 
		sort(remaining_indices.begin(), remaining_indices.end(), 
		     [this](PSlopeStatistics lhs, PSlopeStatistics rhs) 
		     { return lhs->get_level_change(LV_peak) > rhs->get_level_change(LV_peak);});

		Perfusion_idx = remaining_indices[0]->get_index(); 
	}

}

NS_MIA_END
