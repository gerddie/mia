/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2009 - 2010
 *
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


#include <vector>
#include <cassert>
#include <stdexcept>
#include <mia/core/ica.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>


NS_MIA_BEGIN

using namespace std;

struct CICAAnalysisImpl {

	CICAAnalysisImpl(size_t rows, size_t length);
	CICAAnalysisImpl(const itpp::mat& ic, const itpp::mat& mix, const std::vector<float>& mean);
	void normalize_ICs();
	void get_mixing_curve(size_t c, vector<float>& curve) const;
	void set_mixing_series(size_t index, const std::vector<float>& series); 
	void check_set(const CICAAnalysis::IndexSet& s) const; 
	std::vector<float> normalize_Mix(); 

	itpp::mat  m_Signal;
	itpp::mat  m_ICs;
	itpp::mat  m_Mix;
	vector<float> m_mean;

	size_t m_ncomponents;
	size_t m_nlength;
	size_t m_rows;
};


CICAAnalysis::CICAAnalysis(size_t rows, size_t length):
	impl(new CICAAnalysisImpl(rows, length))
{
}

CICAAnalysis::CICAAnalysis(const itpp::mat& ic, const itpp::mat& mix, const std::vector<float>& mean):
	impl(new CICAAnalysisImpl(ic, mix, mean))
{

}

CICAAnalysis::~CICAAnalysis()
{
	delete impl;
}

void CICAAnalysis::set_row(int row, const itppvector&  buffer, double mean)
{
	TRACE_FUNCTION; 
	assert(impl);
	assert(row < impl->m_Signal.rows());
	assert(buffer.size() == impl->m_Signal.cols());
	impl->m_Signal.set_row(row, buffer);
	impl->m_mean[row] = mean;
	cvdebug() << "add row " << row << ", mean=" << mean << "\n";
}

void CICAAnalysis::run(size_t nica)
{
	TRACE_FUNCTION; 
	assert(impl); 
	

	itpp::Fast_ICA fastICA(impl->m_Signal);

	fastICA.set_nrof_independent_components(nica);
	fastICA.set_non_linearity(  FICA_NONLIN_TANH  );
	fastICA.set_approach( FICA_APPROACH_SYMM );
	fastICA.separate();

	impl->m_ICs = fastICA.get_independent_components();
	impl->m_Mix = fastICA.get_mixing_matrix();

	impl->m_ncomponents = impl->m_Mix.cols();
	impl->m_nlength     = impl->m_ICs.cols();
	impl->m_rows        = impl->m_Mix.rows();
}



vector<float> CICAAnalysis::get_feature_row(size_t row) const
{
	TRACE_FUNCTION; 
	if (row < impl->m_ncomponents) {
		vector<float> result(impl->m_nlength);
		for (size_t i = 0; i < impl->m_nlength; ++i)
			result[i] = impl->m_ICs(row, i);
		return result;
	}

	THROW(invalid_argument, "CICAAnalysis::get_feature_row: requested row " << row << " out of range");

}

std::vector<float> CICAAnalysis::get_mix_series(size_t colm)const
{
	TRACE_FUNCTION; 
	if (colm < impl->m_ncomponents) {
		vector<float> result(impl->m_rows);
		impl->get_mixing_curve(colm, result);
		return result;
	}
	THROW(invalid_argument, "CICAAnalysis::get_mix_series: requested series " << colm << " out of range");
}

void CICAAnalysis::set_mixing_series(size_t index, const std::vector<float>& filtered_series)
{
	impl->set_mixing_series(index, filtered_series);
}

void CICAAnalysisImpl::set_mixing_series(size_t index, const std::vector<float>& series)
{
	TRACE_FUNCTION; 
	assert(m_rows == series.size()); 
	assert(index < m_nlength); 
	for (size_t i = 0; i < m_rows; ++i)
		m_Mix(i, index) = series[i];

}


CSlopeClassifier::Columns CICAAnalysis::get_mixing_curves() const
{
	TRACE_FUNCTION; 
	CSlopeClassifier::Columns result(impl->m_ncomponents);
	for (size_t c = 0; c < impl->m_ncomponents; ++c)
		impl->get_mixing_curve(c, result[c]);
	return result;
}

void CICAAnalysisImpl::get_mixing_curve(size_t c, vector<float>& curve) const
{
	TRACE_FUNCTION; 
	curve.resize(m_rows); 
	for (size_t i = 0; i < m_rows; ++i)
		curve[i] = m_Mix(i, c);
}

std::vector<float> CICAAnalysis::get_mix(size_t idx)const
{
	TRACE_FUNCTION; 
	if (idx < impl->m_rows) {
		vector<float> result(impl->m_nlength, impl->m_mean[idx]);
		for (size_t i = 0; i < impl->m_nlength; ++i) {
			for (size_t c = 0; c < impl->m_ncomponents; ++c)
				result[i] += impl->m_ICs(c, i) *  impl->m_Mix(idx, c);
		}
		return result;
	}

	THROW(invalid_argument, "CICAAnalysis::get_mix: requested idx " << idx
	      << " out of range: " << impl->m_rows);
}

std::vector<float> CICAAnalysis::get_delta_feature(const IndexSet& plus, const IndexSet& minus)const
{
	TRACE_FUNCTION;
	
	vector<float> result(impl->m_nlength, 0.0f);
	impl->check_set(plus); 
	impl->check_set(minus); 
	
	for (size_t i = 0; i < impl->m_nlength; ++i) {
		for (IndexSet::const_iterator c = plus.begin(); c != plus.end(); ++c)
			result[i] += impl->m_ICs(*c, i);
		
		for (IndexSet::const_iterator c = minus.begin(); c != minus.end(); ++c)
			result[i] -= impl->m_ICs(*c, i);

	}
	return result;
}

std::vector<float> CICAAnalysis::get_partial_mix(size_t idx, const IndexSet& cps)const
{
	TRACE_FUNCTION; 
	if (idx >=  impl->m_rows) {
		THROW(invalid_argument, "CICAAnalysis::get_mix: requested idx " << idx
		      << " out of range: " << impl->m_rows);		
	
	} else {
		impl->check_set(cps);
		vector<float> result(impl->m_nlength, impl->m_mean[idx]);
		for (size_t i = 0; i < impl->m_nlength; ++i) {
			for (IndexSet::const_iterator c = cps.begin(); c != cps.end(); ++c)
				result[i] += impl->m_ICs(*c, i) *  impl->m_Mix(idx, *c);
		}
		return result;
	}
}

std::vector<float> CICAAnalysis::get_incomplete_mix(size_t idx, const std::set<size_t>& skip)const
{
	TRACE_FUNCTION; 
	if (idx < impl->m_rows) {
		vector<float> result(impl->m_nlength, impl->m_mean[idx]);
		for (size_t i = 0; i < impl->m_nlength; ++i) {
			for (size_t c = 0; c < impl->m_ncomponents; ++c)
				if (skip.find(c) == skip.end())
					result[i] += impl->m_ICs(c, i) *  impl->m_Mix(idx, c);
		}
		return result;
	}

	THROW(invalid_argument, "CICAAnalysis::get_mix: requested idx " << idx
	      << " out of range: " << impl->m_rows);
}

std::vector<float> CICAAnalysis::normalize_Mix()
{
	return impl->normalize_Mix(); 
}

CICAAnalysisImpl::CICAAnalysisImpl(size_t rows, size_t length):
	m_Signal(rows, length),
	m_mean(rows),
	m_ncomponents(0),
	m_nlength(0),
	m_rows(0)
{
	cvdebug() << "Analyis of signal with " << rows << " data rows of " << length << " entries\n"; 
}

CICAAnalysisImpl::CICAAnalysisImpl(const itpp::mat& ic, const itpp::mat& mix, const std::vector<float>& mean):
	m_Signal(mix.cols(), ic.rows()),
	m_ICs(ic),
	m_Mix(mix),
	m_mean(mean),
	m_ncomponents(mix.cols()),
	m_nlength(ic.cols()),
	m_rows(mix.rows())
{
}

void CICAAnalysis::normalize_ICs()
{
	TRACE_FUNCTION; 
	impl->normalize_ICs();
}

void CICAAnalysisImpl::check_set(const CICAAnalysis::IndexSet& s) const
{
	for (CICAAnalysis::IndexSet::const_iterator is = s.begin(); 
	     is != s.end(); ++is) {
		if (*is >= m_ncomponents) {
			THROW(invalid_argument, "CICAAnalysis: request component index " 
			      <<  *is << " but only up to index " << m_ncomponents - 1 
			      << " available\n"); 
		}
	}
}

void CICAAnalysisImpl::normalize_ICs()
{
	// scale all ICs to have a range of 2.0 and are shifted to the mean
	for (size_t c = 0; c < m_ncomponents; ++c) {
		// evaluate range of IC
		double min_val = m_ICs(c, 0);
		double max_val = m_ICs(c, 0);

		double sum = m_ICs(c, 0);
		for (size_t k = 0; k < m_nlength; ++k) {
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

		for (size_t k = 0; k < m_nlength; ++k)
			m_ICs(c, k) = (m_ICs(c, k) - ic_shift) * ic_factor;

		for (size_t r = 0; r < m_rows; ++r) {
			m_mean[r] += m_Mix(r, c) * ic_shift; 
			m_Mix(r, c) *= mix_factor;
		}
	}
}

std::vector<float> CICAAnalysisImpl::normalize_Mix()
{
	std::vector<float> result(m_nlength,0.0f); 
	
	for (size_t c = 0; c < m_ncomponents; ++c) {
		float mean = 0.0f; 
		for (size_t r = 0; r < m_rows; ++r)
			mean += m_Mix(r, c); 
		mean /= m_rows; 
		for (size_t r = 0; r < m_rows; ++r)
			m_Mix(r, c) -= mean; 
		
		for (size_t k = 0; k < m_nlength; ++k)
			result[k] += mean * m_ICs(c, k);
	}
	return result; 
}

NS_MIA_END
