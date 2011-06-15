/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
 *
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#include <map>
#include <algorithm>
#include <stdexcept>
#include <cmath>

#include <mia/core/msgstream.hh>

#include <boost/algorithm/minmax_element.hpp>


#include <mia/core/slopeclassifier.hh>
#include <mia/core/slopestatistics.hh>


NS_MIA_BEGIN

using namespace std;
using namespace boost;


struct CSlopeClassifierImpl {
	int RV_peak;
	int LV_peak;
	int RV_idx;
	int LV_idx;
	int Periodic_idx;
	int Perfusion_idx;
	int Baseline_idx;
	float max_slope_length_diff;
	CSlopeClassifier::SCorrelation  selfcorr;

	typedef vector<float>::const_iterator position;
	typedef pair<position, position> extrems;
	typedef pair<size_t, size_t> extrems_pos;
	CSlopeClassifierImpl(const CSlopeClassifier::Columns& series, bool mean_stripped);
	CSlopeClassifierImpl(); 
private:
	void evaluate_selfcorr(const CSlopeClassifier::Columns& series);
};


CSlopeClassifier::CSlopeClassifier(const CSlopeClassifier::Columns& m, bool mean_stripped)
{
	if (m.size() < 3)
		throw invalid_argument("CSlopeClassifier: require at least 3 curves");
	impl = new CSlopeClassifierImpl(m, mean_stripped);
}

CSlopeClassifier::CSlopeClassifier(const CSlopeClassifier& other):
	impl(new CSlopeClassifierImpl(*other.impl)) 
{
}
CSlopeClassifier::CSlopeClassifier():
	impl(new CSlopeClassifierImpl())
{
}
		     
CSlopeClassifier& CSlopeClassifier::operator =(const CSlopeClassifier& other)
{
	if (this != &other) {
		auto help = new CSlopeClassifierImpl(*other.impl); 
		delete impl; 
		impl = help; 
	}
	return *this;
}

CSlopeClassifier::~CSlopeClassifier()
{
	delete impl;
}

float CSlopeClassifier::get_max_slope_length_diff() const
{
	assert(impl); 
	return impl->max_slope_length_diff;
}

CSlopeClassifier::SCorrelation CSlopeClassifier::max_selfcorrelation()const
{
	return impl->selfcorr;
}

int CSlopeClassifier::get_periodic_idx() const
{
	return impl->Periodic_idx;
}

int CSlopeClassifier::get_perfusion_idx() const
{
	return impl->Perfusion_idx;
}


int CSlopeClassifier::get_RV_idx()const
{
	return impl->RV_idx;
}

int CSlopeClassifier::get_LV_idx() const
{
	return impl->LV_idx;
}

int CSlopeClassifier::get_RV_peak()const
{
	return impl->RV_peak;
}

int CSlopeClassifier::get_LV_peak() const
{
	return impl->LV_peak;
}

int CSlopeClassifier::get_baseline_idx() const
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

struct compare_mean_freq {
	bool operator () (const statmap& a, const statmap& b) const
	{
		return  (a.first->get_mean_frequency() < b.first->get_mean_frequency());
	}
};



float  EXPORT_CORE correlation(const vector<float>& a, const vector<float>& b)
{
	assert(a.size() > 0);
	assert(a.size() == b.size());

	float sxx = 0.0;
	float syy = 0.0;
	float sxy = 0.0;
	float sx =  0.0;
	float sy =  0.0;

	for (size_t i = 0; i < a.size(); ++i) {
		sx += a[i];
		sy += b[i];
		sxx += a[i] * a[i];
		syy += b[i] * b[i];
		sxy += a[i] * b[i];
	}
	const float ssxy = sxy - sx * sy / a.size();
	const float ssxx = sxx - sx * sx / a.size();
	const float ssyy = syy - sy * sy / a.size();
	if (sxx == 0 && syy == 0)
		return 1.0;

	if (sxx == 0 || syy == 0)
		return 0.0;

	return (ssxy * ssxy) /  (ssxx * ssyy);
}

void CSlopeClassifierImpl::evaluate_selfcorr(const CSlopeClassifier::Columns& series)
{
	selfcorr.corr = 0.0; 
	for (size_t i = 0; i < series.size(); ++i)
		for (size_t j = i+1; j < series.size(); ++j) {
			const float corr = correlation(series[i], series[j]);
			if (selfcorr.corr < corr) {
				selfcorr.row1 = i;
				selfcorr.row2 = j;
				selfcorr.corr = corr;
			}
		}
}


CSlopeClassifierImpl::CSlopeClassifierImpl():
	RV_peak(-1), 
	LV_peak(-1), 
	RV_idx(-1), 
	LV_idx(-1), 
	Periodic_idx(-1), 
	Perfusion_idx(-1), 
	Baseline_idx(-1), 
	max_slope_length_diff(0)
{
	selfcorr.row1 = 0;
	selfcorr.row2 = 0;
	selfcorr.corr = 0.0; 
}

CSlopeClassifierImpl::CSlopeClassifierImpl(const CSlopeClassifier::Columns& series, bool mean_stripped):
	Periodic_idx(-1),
	Perfusion_idx(-1),
	Baseline_idx(-1)

{
	size_t n = series.size();
	vector<statmap> stats(n);

	for(size_t i = 0; i < n; ++i) {
		statmap sm;
		sm.first = PSlopeStatistics(new CSlopeStatistics(series[i]));
		sm.second = i;
		stats[i] = sm;
	}

	/* mechanics for classifying the mixing curves:
	   - first sort for curve length - longest is periodic component
	   - use a heuristic based on its mean frequency to decide whether this curve is actually periodic
	   - then sort the remaining curves by covered value range
	      * the curves that cover largest values should correspond to LV and RV enhancement
                (Remark: this is not save!!!)
		* remove last element from the serach range, it's baseline or perfusion
	   - sort for minimal mean frequency, RV-LV slopes should have a low freq
           -
           - sort these two curves by the order in which the high peaks appear to identify
             which is RV (peak comes first) and which LV
	*/
	int sort_skip=0;

	sort(stats.begin(), stats.end(), compare_length());
	max_slope_length_diff = stats[n-1].first->get_curve_length() - stats[n-2].first->get_curve_length();
	float rate = series[0].size() / 7.0f;
	bool has_periodic = stats[n-1].first->get_mean_frequency() > rate ;
	

	cvinfo() << stats[n-1].first->get_mean_frequency() << " vs " << rate << "=" << has_periodic << "\n";

	if (has_periodic && (mean_stripped || n > 3)) {
		// put the periodic element at the end
		cvinfo() << "identify periodic as " << stats[n-1].second << "\n";
		++sort_skip;
	}else
		has_periodic = false; 

	if (n > 4) {
		sort(stats.begin(), stats.end() - sort_skip, compare_mean_freq());
		for (auto k = stats.rbegin() + sort_skip; k != stats.rend(); ++k)
			if (k->first->get_mean_frequency() > rate) {
				cvinfo() << "identify " << k->second << "as too periodic\n"; 
				++sort_skip;
			}
	}
	int remaining = n - sort_skip; 
	cvdebug() << "remaining = " << remaining << "\n"; 

	if (remaining > 2 ) {
		cvdebug() << "range sort\n"; 
		sort(stats.begin(), stats.end() - sort_skip, compare_range());

		int of_interest = 2; 
		double max_range = stats[0].first->get_range(); 
		while (of_interest < remaining && 
		       max_range - stats[of_interest].first->get_range() < 0.5 * max_range)
			++of_interest; 
			
		remaining = of_interest; 
	}

	cvdebug() << "remaining = " << remaining << "\n"; 
	sort(stats.begin(), stats.begin() + remaining, compare_perfusion_peak());

	cvinfo() << "Sorted\n";
	for(size_t i = 0; i < n; ++i) {
		cvinfo() << "Stats["<< stats[i].second << "]"
			  << " range: " << stats[i].first->get_range()
			  << " perfpeakidx: " << stats[i].first->get_perfusion_high_peak().first
			  << " lenght: " << stats[i].first->get_curve_length()
			  << " l*r: " << stats[i].first->get_curve_length() * stats[i].first->get_range()
			  << " l/r: " << stats[i].first->get_curve_length() / stats[i].first->get_range()
			  << " f: " << stats[i].first->get_mean_frequency()
			  << "\n";
	}


	RV_idx = stats[0].second;
	LV_idx = stats[1].second;

	if (mean_stripped) {
		Periodic_idx = stats[n-1].second;
		if (n > 3)
			Perfusion_idx = stats[2].second;
		if (n > 4)
			Baseline_idx = stats[3].second;
	}else {
		if (n == 3)
			Baseline_idx = stats[n-1].second;
		else {
			Periodic_idx = stats[n-1].second;
			Baseline_idx = stats[n-2].second;
			if (n > 4)
				Perfusion_idx = stats[2].second;
		}
	}
	// was not really a periodic component
	if (!has_periodic)
		Periodic_idx = -1;

	RV_peak = stats[0].first->get_perfusion_high_peak().first;
	LV_peak = stats[1].first->get_perfusion_high_peak().first;

	evaluate_selfcorr(series);

}

NS_MIA_END
