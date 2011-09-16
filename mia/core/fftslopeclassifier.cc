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

#include <mia/core/fftslopeclassifier.hh>
#include <mia/core/slopestatistics.hh>


NS_MIA_BEGIN

using namespace std;
using namespace boost;


struct CFFTSlopeClassifierImpl {
	int RV_peak;
	int LV_peak;
	int RV_idx;
	int LV_idx;
	vector<int> Periodic_idx;
	int Perfusion_idx;
	int Baseline_idx;
	float max_freq_energy;
	float max_slope_length_diff;

	typedef vector<float>::const_iterator position;
	typedef pair<position, position> extrems;
	typedef pair<size_t, size_t> extrems_pos;
	CFFTSlopeClassifierImpl(const CFFTSlopeClassifier::Columns& series, float breah_rate, bool mean_stripped);
	CFFTSlopeClassifierImpl(); 
};


CFFTSlopeClassifier::CFFTSlopeClassifier(const CFFTSlopeClassifier::Columns& m, float min_freq, bool mean_stripped)
{
	if (m.size() < 3)
		throw invalid_argument("CFFTSlopeClassifier: require at least 3 curves");
	impl = new CFFTSlopeClassifierImpl(m, min_freq, mean_stripped);
}

CFFTSlopeClassifier::CFFTSlopeClassifier(const CFFTSlopeClassifier& other):
	impl(new CFFTSlopeClassifierImpl(*other.impl)) 
{
}
CFFTSlopeClassifier::CFFTSlopeClassifier():
	impl(new CFFTSlopeClassifierImpl())
{
}
		     
CFFTSlopeClassifier& CFFTSlopeClassifier::operator =(const CFFTSlopeClassifier& other)
{
	if (this != &other) {
		auto help = new CFFTSlopeClassifierImpl(*other.impl); 
		delete impl; 
		impl = help; 
	}
	return *this;
}

CFFTSlopeClassifier::~CFFTSlopeClassifier()
{
	delete impl;
}

float CFFTSlopeClassifier::get_max_slope_length_diff() const
{
	assert(impl); 
	return impl->max_slope_length_diff;
}

vector<int> CFFTSlopeClassifier::get_periodic_indices() const
{
	return impl->Periodic_idx;
}

int CFFTSlopeClassifier::get_perfusion_idx() const
{
	return impl->Perfusion_idx;
}


int CFFTSlopeClassifier::get_RV_idx()const
{
	return impl->RV_idx;
}

int CFFTSlopeClassifier::get_LV_idx() const
{
	return impl->LV_idx;
}

int CFFTSlopeClassifier::get_RV_peak()const
{
	return impl->RV_peak;
}

int CFFTSlopeClassifier::get_LV_peak() const
{
	return impl->LV_peak;
}

int CFFTSlopeClassifier::get_baseline_idx() const
{
	return impl->Baseline_idx;
}


typedef pair<PSlopeStatistics, int> statmap;

struct compare_length {
	bool operator () (const statmap& a, const statmap& b) const
	{
		return  (a.first->get_curve_length() < b.first->get_curve_length());
	}
};

struct compare_range {
	bool operator () (const statmap& a, const statmap& b) const
	{
		return  (a.first->get_range() > b.first->get_range());
	}
};

struct compare_perfusion_peak {
	bool operator () (const statmap& a, const statmap& b) const
	{
		return  (a.first->get_perfusion_high_peak() < b.first->get_perfusion_high_peak());
	}
};

struct compare_freq_enery {
	bool operator () (const statmap& a, const statmap& b) const
	{
		return  (a.first->get_mean_frequency() * a.first->get_energy() < 
			 b.first->get_mean_frequency() * b.first->get_energy());
	}
};

CFFTSlopeClassifierImpl::CFFTSlopeClassifierImpl():
	RV_peak(-1), 
	LV_peak(-1), 
	RV_idx(-1), 
	LV_idx(-1), 
	Perfusion_idx(-1), 
	Baseline_idx(-1), 
	max_freq_energy(0)
{
}


CFFTSlopeClassifierImpl::CFFTSlopeClassifierImpl(const CFFTSlopeClassifier::Columns& series, float breath_rate, bool mean_stripped):
	Perfusion_idx(-1),
	Baseline_idx(-1), 
	max_freq_energy(0.0)

{
	size_t n = series.size();
	vector<statmap> stats(n);
	vector<statmap> nonperiodic;
	float min_freq = series[0].size() / breath_rate;
	
	for(size_t i = 0; i < n; ++i) {
		statmap sm;
		sm.first = PSlopeStatistics(new CSlopeStatistics(series[i], i));
		float freq_energy = sm.first->get_energy(); 
		if (max_freq_energy < freq_energy) 
			max_freq_energy = freq_energy; 
		sm.second = i;
		stats[i] = sm;
	}
	
	double thresh_energy = max_freq_energy / 4.0; 
	

	for(size_t i = 0; i < n; ++i) {
		const statmap& sm = stats[i]; 
		
		cvdebug() << "[" << i << "] freq = " << sm.first->get_mean_frequency() 
			  << ", energy = " << sm.first->get_energy(); 
		if (sm.first->get_mean_frequency() > min_freq && sm.first->get_energy() > thresh_energy) {
			cverb << " <periodic>\n";
			Periodic_idx.push_back(i);
		}else {
			cverb << " <non-periodic>\n"; 
			nonperiodic.push_back(sm); 
		}
	}

	
	
	/* mechanics for classifying the mixing curves:
	   - sort the slopes that are below the periodic threshhold for high range and eliminate the 
	   other curves, they are perfusion or baseline 
	   - sort these two curves by the order in which the high peaks appear to identify
             which is RV (peak comes first) and which LV
	*/
	int remaining = nonperiodic.size(); 
	if (remaining > 2) {	
		sort(nonperiodic.begin(), nonperiodic.end(), compare_range()); 
		
		int of_interest = 2; 
		double max_range = stats[0].first->get_range(); 
		while (of_interest < remaining && 
		       stats[of_interest].first->get_range() < 0.5 * max_range)
			++of_interest; 
		remaining = of_interest; 
	}

	cvdebug() << "remaining = " << remaining << "\n"; 
	sort(nonperiodic.begin(), nonperiodic.begin() + remaining, compare_perfusion_peak());
	
	RV_idx = nonperiodic[0].second;
	LV_idx = nonperiodic[1].second;

	RV_peak = nonperiodic[0].first->get_perfusion_high_peak().first;
	LV_peak = nonperiodic[1].first->get_perfusion_high_peak().first;

	// one could trail this by estimating base line and perfusion index 
}

NS_MIA_END
