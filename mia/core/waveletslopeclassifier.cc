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
	CWaveletSlopeClassifier::EAnalysisResult result; 

	typedef vector<float>::const_iterator position;
	typedef pair<position, position> extrems;
	typedef pair<size_t, size_t> extrems_pos;
	CWaveletSlopeClassifierImpl(const CWaveletSlopeClassifier::Columns& series, bool mean_stripped);
	CWaveletSlopeClassifierImpl(); 
};


CWaveletSlopeClassifier::CWaveletSlopeClassifier(const CWaveletSlopeClassifier::Columns& m, bool mean_stripped)
{
	if (m.size() < 3)
		throw invalid_argument("CWaveletSlopeClassifier: require at least 3 curves");
	impl = new CWaveletSlopeClassifierImpl(m, mean_stripped);
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
	result(CWaveletSlopeClassifier::wsc_fail)
{
}

CWaveletSlopeClassifierImpl::CWaveletSlopeClassifierImpl(const CWaveletSlopeClassifier::Columns& series, bool mean_stripped):
	RV_peak(-1), 
	LV_peak(-1), 
	RV_idx(-1), 
	LV_idx(-1), 
	Movement_idx(-1),
	Perfusion_idx(-1),
	Baseline_idx(-1), 
	free_breathing(false), 
	max_movment_energy(0.0), 
	result(CWaveletSlopeClassifier::wsc_fail)
{
	vector<PSlopeStatistics> vstats; 
	for (int i = 0; i < series.size(); ++i)
		vstats.push_back(PSlopeStatistics(new CSlopeStatistics(series[i], i))); 
	
	/// classify 
	int movement_idx = vstats[0]->get_level_coefficient_sums().size() - 2;
	
	// first estimate if this is free breathing or breath holding 
	vector<float> movement_pos(4, 0);
	for (size_t i = 0; i < series.size(); ++i)
		movement_pos[vstats[i]->get_level_mean_energy_position()[movement_idx]] += vstats[i]->get_level_coefficient_sums()[movement_idx];
	
	bool ifree_breathing = (movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_begin] &&
				movement_pos[CSlopeStatistics::ecp_center] > movement_pos[CSlopeStatistics::ecp_end]);

	bool at_begin = (!ifree_breathing) && (movement_pos[CSlopeStatistics::ecp_end] < movement_pos[CSlopeStatistics::ecp_begin]); 
		
	cvmsg() << "Detected free breathing data set\n";
	int low_energy_start_idx = free_breathing ? 0 : 1;
	
	// get movement components 
	vector<PSlopeStatistics> movement_indices;
	vector<PSlopeStatistics> remaining_indices;

	int min_energy_idx = -1; 
	float min_energy = numeric_limits<float>::max(); 

	vector<bool> is_high_freq(series.size()); 
	for (size_t i = 0; i < series.size(); ++i) {
		auto e = vstats[i]->get_level_coefficient_sums(); 
		cvdebug() << "energies: "  << e 
			  << " start= "<< low_energy_start_idx
			  << " end= " << movement_idx - 1
			  << "\n"; 
		float low_freq = accumulate(e.begin() + low_energy_start_idx, e.begin() + movement_idx, 0.0); 
		cvdebug() << i << ": low " << low_freq << " vs  high " << e[movement_idx] << "\n"; 
		float high_freq = accumulate(e.begin() + movement_idx, e.end(), 0.0); 

		if (min_energy > vstats[i]->get_wavelet_energy()) {
			min_energy = vstats[i]->get_wavelet_energy(); 
			min_energy_idx = i; 
		}
		
		is_high_freq[i] = (low_freq < high_freq);
		if (!at_begin && is_high_freq[i])
			if (vstats[i]->get_mean_energy_position() == CSlopeStatistics::ecp_begin) {
				cvinfo() << "c=" << i << ":override motion because we assume it's RV enhacement\n";
				is_high_freq[i] = false; 
			}
		
	}

	
	// if the mean is stripped, the baseline vanishes 
	if (!mean_stripped) {
		Baseline_idx =  min_energy_idx; 
		cvinfo() << "Baseline " << min_energy_idx 
			 <<  " with energy " << min_energy 
			 << "\n"; 
	}
	
	for (size_t i = 0; i < series.size(); ++i) {
		if (i == Baseline_idx) 
			continue; 
		if (is_high_freq[i]) 
			movement_indices.push_back(vstats[i]); 
		else 
			remaining_indices.push_back(vstats[i]); 
	}

	vector<int> mi(movement_indices.size()); 
	transform(movement_indices.begin(), movement_indices.end(), mi.begin(), 
		  [](PSlopeStatistics i){return i->get_index();}); 
	cvinfo() << "Detect movement candidates " << mi << "\n"; 
	
	if (!movement_indices.empty()) {
		vector<float> movement_energies; 
		max_movment_energy = 0.0; 
		int max_movment_idx = -1; 
		
		for (auto k = movement_indices.begin(); k != movement_indices.end(); ++k) {
			float energy = (*k)->get_level_coefficient_sums()[movement_idx]; 
			if (max_movment_energy < energy ) {
				max_movment_energy = energy; 
				max_movment_idx = (*k)->get_index();
			}
			if ( (*k)->get_index() == min_energy_idx )
				continue; 
			movement_energies.push_back(energy);
		}
		
		// if no special movement component could be identified, or the component has an 
		// movement energy below the baseline mark it as no movement 
		if (max_movment_idx < 0 || max_movment_idx == min_energy_idx) {
			result = CWaveletSlopeClassifier::wsc_no_movement; 
		} else {
			// if the movement energy is below one of the remaining components 
			// then mark it as low movement 
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
		if (movement_energies.size() > 1) {
			sort(movement_energies.begin(), movement_energies.end(), [](float x, float y){return x > y;}); 
			cvinfo() << "movement_energys = " << movement_energies << "\n";
			max_movment_energy = movement_energies[0] / movement_energies.size(); 
			cvinfo() << " Min Movement energy delta = " << max_movment_energy << "\n"; 

		}else{
			cvinfo() << "got exactly one movement candidate\n"; 
		}
	}else{
		cvmsg() << "No movement component identified\n"; 
		result = CWaveletSlopeClassifier::wsc_no_movement; 
	}


	vector<int> ri(remaining_indices.size()); 
	transform(remaining_indices.begin(), remaining_indices.end(), ri.begin(), 
		  [](PSlopeStatistics i){return i->get_index();}); 
	cvinfo() << "Remaining elements: " << ri << "\n"; 
	
	// classification of the remaining components 
	if (remaining_indices.size() < 2) {
		cvmsg() << "Classification failed because too few remaining components.\n"; 
		result = CWaveletSlopeClassifier::wsc_fail; 
		return; 
	}
	
	// sort according to the weighted time point of positive curve values. 
	// LV and RV curves have positive values on the left, perfusion has positive values 
	// on the right (later) 
	//if (remaining_indices.size() > 2) {
		sort(remaining_indices.begin(), remaining_indices.end(), 
		     [](PSlopeStatistics lhs, PSlopeStatistics rhs) {
			     return lhs->get_positive_time_mean() < rhs->get_positive_time_mean();
		     });
		//}
	
	
	// RV enhancement comes before LV enhancement 
	//sort(remaining_indices.begin(), remaining_indices.begin() + 2, 
//	     [](PSlopeStatistics lhs, PSlopeStatistics rhs) {
//		     return lhs->get_perfusion_high_peak().first < rhs->get_perfusion_high_peak().first;
//	     });
	
	RV_peak = remaining_indices[0]->get_perfusion_high_peak().first; 
	LV_peak = remaining_indices[1]->get_perfusion_high_peak().first; 
	cvinfo() << "RV ptm = " << remaining_indices[0]->get_positive_time_mean() 
		 << " peak = " << RV_peak 
		 << "\n"; 
	cvinfo() << "LV ptm = " << remaining_indices[1]->get_positive_time_mean() 
		 << " peak = " << LV_peak 
		 << "\n"; 
	RV_idx  = remaining_indices[0]->get_index(); 
	LV_idx  = remaining_indices[1]->get_index();

	
	if (remaining_indices.size() > 2)
		Perfusion_idx = remaining_indices[2]->get_index();

	if (!movement_indices.empty())
		result = CWaveletSlopeClassifier::wsc_normal; 
}

NS_MIA_END
