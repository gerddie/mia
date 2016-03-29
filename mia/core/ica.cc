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

#include <vector>
#include <cassert>
#include <stdexcept>
#include <memory>
#include <miaconfig.h>
#include <mia/core/ica.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN

using namespace std;

struct CICAAnalysisITPPImpl {

    CICAAnalysisITPPImpl(unsigned int rows, unsigned int length);
    CICAAnalysisITPPImpl(const itpp::mat& ic, const itpp::mat& mix, const std::vector<double>& mean);
	void normalize_ICs();
    void get_mixing_curve(unsigned int c, vector<float>& curve) const;
    void set_mixing_series(unsigned int index, const std::vector<float>& series);
    void check_set(const CICAAnalysisITPP::IndexSet& s) const;
	std::vector<float> normalize_Mix();
	float max_selfcorrelation(int& row1, int &row2) const;

	itpp::mat  m_Signal;
	itpp::mat  m_ICs;
	itpp::mat  m_Mix;
    vector<double> m_mean;

    unsigned int m_ncomponents;
    unsigned int m_nlength;
    unsigned int m_rows;

	int m_max_iterations;
	int m_approach; 
};


CICAAnalysisITPP::CICAAnalysisITPP(unsigned int rows, unsigned int length):
    impl(new CICAAnalysisITPPImpl(rows, length))
{
}

CICAAnalysisITPP::CICAAnalysisITPP(const itpp::mat& ic, const itpp::mat& mix, const std::vector<double>& mean):
    impl(new CICAAnalysisITPPImpl(ic, mix, mean))
{

}

CICAAnalysisITPP::CICAAnalysisITPP():impl(nullptr)
{

}


void CICAAnalysisITPP::initialize(unsigned int series_length, unsigned int slice_size)
{
    auto new_impl = new CICAAnalysisITPPImpl(series_length, slice_size);
    swap(new_impl, impl);

    if (new_impl)
        delete new_impl;
}

CICAAnalysisITPP::~CICAAnalysisITPP()
{
	delete impl;
}

void CICAAnalysisITPP::set_row_internal(unsigned row, const std::vector<double>&  buffer, double mean)
{
	TRACE_FUNCTION;
	assert(impl);
    assert(row < static_cast<unsigned>(impl->m_Signal.rows()));
    assert(buffer.size() == static_cast<unsigned>(impl->m_Signal.cols()));

    itppvector buf(static_cast<signed>(buffer.size()));
    for (auto i= 0; static_cast<unsigned>(i) < buffer.size(); ++i)
        buf[i] = buffer[static_cast<unsigned>(i)];

    impl->m_Signal.set_row(static_cast<signed>(row), buf);
	impl->m_mean[row] = mean;
	cvdebug() << "add row " << row << ", mean=" << mean << "\n";
}

void CICAAnalysisITPP::set_approach(EApproach approach)
{
    switch (approach) {
    case appr_defl: impl->m_approach = FICA_APPROACH_DEFL;
        break;
    case appr_symm: impl->m_approach = FICA_APPROACH_SYMM;
        break;
    default:
        throw invalid_argument("CICAAnalysisITPP:set_approach: Unknown approach given");
    }

}

bool CICAAnalysisITPP::run(unsigned int nica, vector<vector<float> > guess)
{
	TRACE_FUNCTION;
	assert(impl);
	

	itpp::Fast_ICA fastICA(impl->m_Signal);
	
	fastICA.set_nrof_independent_components(nica);
	fastICA.set_non_linearity(  FICA_NONLIN_TANH  );
	
#if ICA_ALWAYS_USE_SYMM
	// approach APPROACH_DEFL may not return in the unpached itpp 
	// therefore, always use APPROACH_SYMM 
	fastICA.set_approach( FICA_APPROACH_SYMM );
#else
	fastICA.set_approach( impl->m_approach );
#endif
	if (impl->m_max_iterations > 0)
		fastICA.set_max_num_iterations(impl->m_max_iterations);

	if (!guess.empty()) {
        itpp::mat mguess(impl->m_Signal.rows(), static_cast<int>(guess.size()));
        for (unsigned int c = 0; c < guess.size(); ++c)
            for (unsigned int r = 0; r < guess.size(); ++r)
				mguess(r,c) = guess[c][r]; 
		fastICA.set_init_guess(mguess); 
	}
#ifdef ICA_ALWAYS_USE_SYMM
	// in the unpatched itpp separate doesn't return a value
	// therefore, we always assume true
	bool result = true; 
	fastICA.separate();
#else
	bool result = fastICA.separate();
#endif

	impl->m_ICs = fastICA.get_independent_components();
	impl->m_Mix = fastICA.get_mixing_matrix();

	impl->m_ncomponents = impl->m_Mix.cols();
	impl->m_nlength     = impl->m_ICs.cols();
	impl->m_rows        = impl->m_Mix.rows();
	return result; 
}

void CICAAnalysisITPP::run_auto(int nica, int min_ica, float corr_thresh)
{
	TRACE_FUNCTION;
	assert(impl);

	std::unique_ptr<itpp::Fast_ICA > fastICA( new itpp::Fast_ICA(impl->m_Signal));

	float corr = 1.0;
	do {

		cvinfo() << __func__ << ": run with " << nica << "\n";
		fastICA->set_nrof_independent_components(nica);
		fastICA->set_non_linearity(  FICA_NONLIN_TANH  );
		fastICA->set_approach( FICA_APPROACH_DEFL );
		fastICA->separate();

		impl->m_ICs = fastICA->get_independent_components();
		impl->m_Mix = fastICA->get_mixing_matrix();

		impl->m_ncomponents = impl->m_Mix.cols();
		impl->m_nlength     = impl->m_ICs.cols();
		impl->m_rows        = impl->m_Mix.rows();

		normalize_Mix();
		normalize_ICs();

		int row1 = -1;
		int row2 = -1;

		corr = 	impl->max_selfcorrelation(row1, row2);
		cvinfo() << __func__ << ": R² = " << corr << ", n=" << nica << " @ " << impl->m_Mix.cols() << "\n";

		if (fabs(corr) < corr_thresh)
			break;

		// copy old ICs to new guess if they don'z correspond to the most correlated mixes
		itpp::mat guess;
        unsigned int guess_rows = 0;
		for (int i = 0; i < nica; ++i)
			if (i != row1 && i != row2)
				guess.ins_row(guess_rows++, impl->m_ICs.get_row(i));
			else
				cvinfo() << __func__ << ": skip row " << i << "\n";


		// now combine the most correlated mixes
		itppvector buffer(impl->m_ICs.cols());

		if (corr < 0)
			for (int i = 0; i < impl->m_ICs.cols(); ++i)
				buffer[i] = (impl->m_ICs(row1, i) - impl->m_ICs(row2, i)) / 2.0;
		else
			for (int i = 0; i < impl->m_ICs.cols(); ++i)
				buffer[i] = (impl->m_ICs(row1, i) + impl->m_ICs(row2, i)) / 2.0;
		--nica;
		guess.ins_row(guess_rows, buffer);
		cvinfo() << "guess.rows()=" << guess.rows() << "\n";
		assert(guess.rows() == nica);

		fastICA.reset( new itpp::Fast_ICA(impl->m_Signal));
		fastICA->set_init_guess(guess);

	} while (nica > min_ica);

}

unsigned int CICAAnalysisITPP::get_ncomponents() const
{
	return impl->m_ncomponents;
}

vector<float> CICAAnalysisITPP::get_feature_row(unsigned int row) const
{
	TRACE_FUNCTION;
	if (row < impl->m_ncomponents) {
		vector<float> result(impl->m_nlength);
        for (unsigned int i = 0; i < impl->m_nlength; ++i)
            result[i] = static_cast<float>(impl->m_ICs(row, i));
		return result;
	}

    throw create_exception<invalid_argument>("CICAAnalysisITPP::get_feature_row: requested row ", row, " out of range");

}

std::vector<float> CICAAnalysisITPP::get_mix_series(unsigned int colm)const
{
	TRACE_FUNCTION;
	if (colm < impl->m_ncomponents) {
		vector<float> result(impl->m_rows);
		impl->get_mixing_curve(colm, result);
		return result;
	}
    throw create_exception<invalid_argument>("CICAAnalysisITPP::get_mix_series: requested series ", colm, " out of range");
}

void CICAAnalysisITPP::set_mixing_series(unsigned int index, const std::vector<float>& filtered_series)
{
	impl->set_mixing_series(index, filtered_series);
}


float correlation(const CICAAnalysisITPP::itppvector& a, const CICAAnalysisITPP::itppvector& b)
{
	assert(a.size() > 0);
	assert(a.size() == b.size());

	float sxx = 0.0;
	float syy = 0.0;
	float sxy = 0.0;
	float sx =  0.0;
	float sy =  0.0;

	for (int i = 0; i < a.size(); ++i) {
		sx += a[i];
		sy += b[i];
		sxx += a[i] * a[i];
		syy += b[i] * b[i];
		sxy += a[i] * b[i];
	}
	const float ssxy = sxy - sx * sy / a.size();
	const float ssxx = sxx - sx * sx / a.size();
	const float ssyy = syy - sy * sy / a.size();
    if (sxx < 1e-10 && syy < 1e-10 )
		return 1.0;

    if (sxx < 1e-10 || syy < 1e-10 )
		return 0.0;

	return (ssxy * ssxy) /  (ssxx * ssyy);
}

float CICAAnalysisITPPImpl::max_selfcorrelation(int& row1, int &row2) const
{
	float max_corr = 0.0;
	for (int i = 0; i < m_Mix.cols(); ++i)
		for (int j = i+1; j < m_Mix.cols(); ++j) {
			const float corr = correlation(m_Mix.get_col(i), m_Mix.get_col(j));
			if (fabs(max_corr) < fabs(corr)) {
				row1 = i;
				row2 = j;
				max_corr = corr;
				cvdebug() << "Corr=" << max_corr << " @ " << i << "," << j << "\n";
			}
		}
	return max_corr;

}

void CICAAnalysisITPPImpl::set_mixing_series(unsigned int index, const std::vector<float>& series)
{
	TRACE_FUNCTION;
	assert(m_rows == series.size());
	assert(index < m_nlength);
    for (unsigned int i = 0; i < m_rows; ++i)
		m_Mix(i, index) = series[i];

}


CSlopeColumns CICAAnalysisITPP::get_mixing_curves() const
{
	TRACE_FUNCTION;
	CSlopeColumns result(impl->m_ncomponents);
    for (unsigned int c = 0; c < impl->m_ncomponents; ++c)
		impl->get_mixing_curve(c, result[c]);
	return result;
}

void CICAAnalysisITPPImpl::get_mixing_curve(unsigned int c, vector<float>& curve) const
{
	TRACE_FUNCTION;
	curve.resize(m_rows);
    for (unsigned int i = 0; i < m_rows; ++i)
        curve[i] = static_cast<float>(m_Mix(i, c));
}

std::vector<float> CICAAnalysisITPP::get_mix(unsigned int idx)const
{
	TRACE_FUNCTION;
	if (idx < impl->m_rows) {
        vector<float> result(impl->m_nlength, static_cast<float>(impl->m_mean[idx]));
        for (unsigned int i = 0; i < impl->m_nlength; ++i) {
            for (unsigned int c = 0; c < impl->m_ncomponents; ++c)
				result[i] += impl->m_ICs(c, i) *  impl->m_Mix(idx, c);
		}
		return result;
	}

    throw create_exception<invalid_argument>("CICAAnalysisITPP::get_mix: requested idx ", idx, " out of range: ", impl->m_rows);
}

std::vector<float> CICAAnalysisITPP::get_delta_feature(const IndexSet& plus, const IndexSet& minus)const
{
	TRACE_FUNCTION;

	vector<float> result(impl->m_nlength, 0.0f);
	impl->check_set(plus);
	impl->check_set(minus);

    for (unsigned int i = 0; i < impl->m_nlength; ++i) {
		for (IndexSet::const_iterator c = plus.begin(); c != plus.end(); ++c)
			result[i] += impl->m_ICs(*c, i);

		for (IndexSet::const_iterator c = minus.begin(); c != minus.end(); ++c)
			result[i] -= impl->m_ICs(*c, i);

	}
	return result;
}

std::vector<float> CICAAnalysisITPP::get_partial_mix(unsigned int idx, const IndexSet& cps)const
{
	TRACE_FUNCTION;
	if (idx >=  impl->m_rows) {
        throw create_exception<invalid_argument>("CICAAnalysisITPP::get_mix: requested idx ", idx,
					       " out of range: ", impl->m_rows);

	} else {
		impl->check_set(cps);
        vector<float> result(impl->m_nlength, static_cast<float>(impl->m_mean[idx]));
        for (unsigned int i = 0; i < impl->m_nlength; ++i) {
			for (IndexSet::const_iterator c = cps.begin(); c != cps.end(); ++c)
				result[i] += impl->m_ICs(*c, i) *  impl->m_Mix(idx, *c);
		}
		return result;
	}
}

std::vector<float> CICAAnalysisITPP::get_incomplete_mix(unsigned int idx, const std::set<unsigned int>& skip)const
{
	TRACE_FUNCTION;
	if (idx < impl->m_rows) {
        vector<float> result(impl->m_nlength, static_cast<float>(impl->m_mean[idx]));
        for (unsigned int i = 0; i < impl->m_nlength; ++i) {
            for (unsigned int c = 0; c < impl->m_ncomponents; ++c)
				if (skip.find(c) == skip.end())
					result[i] += impl->m_ICs(c, i) *  impl->m_Mix(idx, c);
		}
		return result;
	}

    throw create_exception<invalid_argument>("CICAAnalysisITPP::get_mix: requested idx ", idx,
				       " out of range: ", impl->m_rows);
}

std::vector<float> CICAAnalysisITPP::normalize_Mix()
{
	return impl->normalize_Mix();
}

CICAAnalysisITPPImpl::CICAAnalysisITPPImpl(unsigned int rows, unsigned int length):
	m_Signal(rows, length),
	m_mean(rows),
	m_ncomponents(0),
	m_nlength(0),
	m_rows(0),
	m_max_iterations(0), 
	m_approach(FICA_APPROACH_DEFL)
{
	cvdebug() << "Analyis of signal with " << rows << " data rows of " << length << " entries\n";
}

CICAAnalysisITPPImpl::CICAAnalysisITPPImpl(const itpp::mat& ic, const itpp::mat& mix, const std::vector<double>& mean):
	// Coverty may complain here, but the order is correct
	// the signal is a matrix of the number of columns of the mixing matrix = time points 
	// by rows of the independent components - each row constitutes th epixel ogf a feature image
	// coverity[swapped_arguments]
	m_Signal(mix.cols(), ic.rows()),
	m_ICs(ic),
	m_Mix(mix),
	m_mean(mean),
	m_ncomponents(mix.cols()),
	m_nlength(ic.cols()),
	m_rows(mix.rows()),
	m_max_iterations(0), 
	m_approach(FICA_APPROACH_DEFL)
{
	TRACE_FUNCTION;
}

void CICAAnalysisITPP::normalize_ICs()
{
	TRACE_FUNCTION;
	impl->normalize_ICs();
}

void CICAAnalysisITPPImpl::check_set(const CICAAnalysisITPP::IndexSet& s) const
{
    for (CICAAnalysisITPP::IndexSet::const_iterator is = s.begin();
	     is != s.end(); ++is) {
		if (*is >= m_ncomponents) {
            throw create_exception<invalid_argument>("CICAAnalysisITPP: request component index ",  *is ,
						       " but only up to index ", m_ncomponents - 1, 
						       " available\n");
		}
	}
}

void CICAAnalysisITPPImpl::normalize_ICs()
{
#if 1
	if (m_nlength < 2) 
		throw invalid_argument("ICAAnalysis: input should have at least two pixels"); 

	// scale all ICs to have a variance of 1.0 
    for (unsigned int c = 0; c < m_ncomponents; ++c) {
		// evaluate range of IC
		double v = m_ICs(c, 0);
	
		double sum = v; 
		double sum2 = v * v;
        for (unsigned int k = 1; k < m_nlength; ++k) {
			const double v = m_ICs(c, k);
			sum += v;
			sum2 += v * v;
		}
		const double ic_shift = sum / m_nlength;
		const double sigma = sqrt((sum2 - m_nlength * ic_shift * ic_shift) / (m_nlength - 1));

		// we want to start all slopes with the negative value
		// makes only truely sense, if the allover mean was stripped
		float invert = 1.0;
		if (m_Mix(0, c) > 0) {
			cvdebug() << "Component " << c << " invert sign\n";
			invert = -1.0;
		}
		
		if (sigma > 0) {
			const double ic_factor = invert * 2.0 / sigma;
			const double mix_factor = 1.0 / ic_factor;
			
            for (unsigned int k = 0; k < m_nlength; ++k)
				m_ICs(c, k) = (m_ICs(c, k) - ic_shift) * ic_factor;
			
            for (unsigned int r = 0; r < m_rows; ++r) {
				m_mean[r] += m_Mix(r, c) * ic_shift;
				m_Mix(r, c) *= mix_factor;
			}
		}
	}
	
#else 
	// scale all ICs to have a range of 2.0 and are shifted to the mean
    for (unsigned int c = 0; c < m_ncomponents; ++c) {
		// evaluate range of IC
		double min_val = m_ICs(c, 0);
		double max_val = m_ICs(c, 0);

		double sum = m_ICs(c, 0);
        for (unsigned int k = 0; k < m_nlength; ++k) {
			const double v = m_ICs(c, k);
			if (min_val > v)
				min_val = v;
			if (max_val < v)
				max_val = v;
			sum += v;
		}
		assert(min_val != max_val);


		// we want to start all slopes with the negative value
		// makes only truely sense, if the 1allover mean was stripped
		float invert = 1.0;
		if (m_Mix(0, c) > 0) {
			cvdebug() << "Component " << c << " invert sign\n";
			invert = -1.0;
		}

		const double ic_factor = invert * 2.0 / (max_val - min_val);
		const double mix_factor = 1.0 / ic_factor;


		const double ic_shift = sum / m_nlength;

        for (unsigned int k = 0; k < m_nlength; ++k)
			m_ICs(c, k) = (m_ICs(c, k) - ic_shift) * ic_factor;

        for (unsigned int r = 0; r < m_rows; ++r) {
			m_mean[r] += m_Mix(r, c) * ic_shift;
			m_Mix(r, c) *= mix_factor;
		}
	}
#endif 
}

std::vector<float> CICAAnalysisITPPImpl::normalize_Mix()
{
	std::vector<float> result(m_nlength,0.0f);

    for (unsigned int c = 0; c < m_ncomponents; ++c) {
		float mean = 0.0f;
        for (unsigned int r = 0; r < m_rows; ++r)
			mean += m_Mix(r, c);
		mean /= m_rows;
        for (unsigned int r = 0; r < m_rows; ++r)
			m_Mix(r, c) -= mean;

        for (unsigned int k = 0; k < m_nlength; ++k)
			result[k] += mean * m_ICs(c, k);
	}
    for (unsigned int c = 0; c < m_ncomponents; ++c) {
		// invert sign so that all mixing curves start with a value < 0 
		if (m_Mix(0, c) > 0) {
            for (unsigned int r = 0; r < m_rows; ++r)
				m_Mix(r, c) *= -1;
            for (unsigned int k = 0; k < m_nlength; ++k)
				m_ICs(c, k) *= -1; 
		}
	}
	
	return result;
}


void CICAAnalysisITPP::set_max_iterations(int n)
{
	impl->m_max_iterations = n;
}

PICAAnalysis CICAAnalysisITPPFactory::create() const
{
    return PICAAnalysis(new CICAAnalysisITPP);
}

NS_MIA_END
