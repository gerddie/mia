/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <miaconfig.h> 

#include <mia/core/gsl_matrix_vector_ops.hh>
#include <mia/core/gsl_pca.hh>
#include <mia/core/fastica/fastica.hh>
#include <mia/core/fastica_nonlinearity.hh>
#include <mia/core/errormacro.hh>
#include <mia/core/msgstream.hh>

#include <algorithm>
#include <random>
#include <cmath>
#include <stdexcept>
#include <gsl/gsl_blas.h>
#include <vector>
#include <cassert>
#include <stdexcept>
#include <memory>


NS_MIA_BEGIN 

using gsl::Vector; 
using gsl::Matrix; 
using gsl::CSymmvEvalEvec; 

using std::swap; 
using std::sort; 
using std::transform; 
using std::vector;
using std::invalid_argument; 

FastICA::FastICA(int num_ic):
        m_approach(CIndepCompAnalysis::appr_defl), 
        m_numOfIC(num_ic), 
        m_finetune(true), 
        m_mu(0.9), 
        m_epsilon(1e-10), 
        m_sampleSize(1.0), 
        m_stabilization(true), 
        m_maxNumIterations(1000), 
        m_maxFineTune(200),
	m_firstEig(-1), 
	m_lastEig(-1),
        m_PCAonly(false), 
	m_component_energy_limit(0.9), 
	m_with_initial_guess(false), 
	m_do_saddle_check(true), 
	m_saddle_postiter(10), 
	m_separating_matrix(1,1,false),
	m_deterministic_seed(-1)
{
        m_nonlinearity = produce_fastica_nonlinearity("pow3");
}

struct CCenteredSignal  {
	CCenteredSignal(const Matrix& mixedSig); 
	
	Matrix signal; 
	Vector mean; 
}; 


CCenteredSignal::CCenteredSignal(const Matrix& mixedSig):
	signal(mixedSig.rows(), mixedSig.cols(), false), 
	mean(mixedSig.rows(), true)
{
	for (size_t r = 0; r < mixedSig.rows(); ++r) {
		for (size_t c = 0; c < mixedSig.cols(); ++c)
			mean[r] += mixedSig(r,c); 
		mean[r] /= mixedSig.cols(); 
		for (size_t c = 0; c < mixedSig.cols(); ++c)
			signal.set(r,c, mixedSig(r,c) - mean[r]); 
	}
}

void FastICA::set_deterministic_seed(int seed)
{
	TRACE_FUNCTION; 
	m_deterministic_seed = seed; 
}

void FastICA::evaluate_whiten_matrix(const Matrix& evec, const Vector& eval)
{
	m_whitening_matrix.reset(evec.cols(), evec.rows(), false); 
	m_dewhitening_matrix.reset(evec.rows(), evec.cols(), false);
	
	for (unsigned  i = 0; i < eval.size(); i++) {
		double iwscale = sqrt(eval[i]);
		double wscale = 1.0 / iwscale;
		
		auto wmr = m_whitening_matrix.get_row(i); 
		auto inv = evec.get_column(i);
		auto dwmc = m_dewhitening_matrix.get_column(i); 
		
		transform(inv.begin(), inv.end(), wmr.begin(), 
			  [wscale](double x) {return wscale * x;}); 
		
		transform(inv.begin(), inv.end(), dwmc.begin(), 
			  [iwscale](double x) {return iwscale * x;}); 
	}
	
	cvdebug() << "Whitening: = " << m_whitening_matrix << "\n"; 
	cvdebug() << "DeWhitening: = " << m_dewhitening_matrix << "\n"; 

}

bool FastICA::separate(const gsl::Matrix&  mix)
{
	CCenteredSignal centered(mix);
	cvdebug() << "separate signal of size " << centered.signal.rows() << "x" << centered.signal.cols() << "\n"; 

	// currently not used 
	Matrix guess;
	if (m_initGuess)
		guess = m_initGuess;


	// run PCA to prepare the whitening 
	// also select the limit of useful compinents based on maximal 
	// energy 
	gsl::PCA pca(m_numOfIC, m_component_energy_limit);
	auto pca_result = pca.analyze(centered.signal); 
	
	if (pca_result.eval.size() < 1) {
		m_independent_components = mix;
		return false;
	}

	// update the number of components 
	m_numOfIC = pca_result.eval.size(); 

	cvdebug() << "Considering " << m_numOfIC << " independend components\n"; 
	cvdebug() << "PCA: eval= " << pca_result.eval << "\n"; 
	cvdebug() << "PCA: evec= " << pca_result.evec << "\n"; 

	evaluate_whiten_matrix(pca_result.evec, pca_result.eval);

	// if only PCA, stop here and save the dewithening as independend components 
	// where is the mixing matrix? 
	if (m_PCAonly) {
		m_mixing_matrix.reset(mix.rows(), m_dewhitening_matrix.cols(), 1.0);
		m_independent_components = m_dewhitening_matrix; 
		return true; 
	}

	// run the actual ICA 
	
	auto whitened_signal = m_whitening_matrix * centered.signal; 

	bool result = false; 
	Matrix B( whitened_signal.rows(), m_numOfIC,  true); 	
	
	switch (m_approach) {
	case CIndepCompAnalysis::appr_defl: result = fpica_defl(whitened_signal, B);break; 
	case CIndepCompAnalysis::appr_symm: result = fpica_symm(whitened_signal, B); break; 
	default: 
		throw std::invalid_argument("FastICA::separate: unknown apporach given"); 
		
	}

	// evaluate the results if the ICA 
	m_mixing_matrix = m_dewhitening_matrix * B; 
		
	cvdebug() << "Mixing matrix= " << m_mixing_matrix << "\n"; 

	multiply_mT_m(m_separating_matrix, B, m_whitening_matrix); 
	
	m_independent_components = m_separating_matrix * mix; 

	cvdebug() << "ICs= " << m_independent_components << "\n"; 

	return result; 
}

bool FastICA::fpica_defl_round(int component, Vector& w, Matrix& B)
{
	Vector w_old(w);
	Vector w_old2(w);

	double mu = m_mu;
	double delta = m_epsilon + 1.0;

	bool is_finetuning = false;  
	bool converged = false; 
	bool loong = false; 

	double stroke = 0.0; 

	int iter = 0;
	int maxiter = m_maxNumIterations; 
	
	while (!converged && iter < maxiter) {
			
		cvdebug() << "Defl: c=" << component
			  << ", iter= " << iter 
			  << ", delta= " << delta
			  << "\n"; 
		
		m_nonlinearity->set_mu(mu); 
		m_nonlinearity->apply(w); 
		
		Vector w_save(w); 
		for (int j = 0; j < component; ++j) {
			const double wdot = B.dot_column(j, w_save); 
                        auto wj = B.get_column(j);
			cblas_daxpy(wj->size, -wdot, wj->data, wj->stride, w->data, w->stride);
		}
		
		double norm = sqrt(dot(w, w));
		if (norm > 0.0) 
			gsl_vector_scale(w, 1.0/norm); 

		Vector w_help = w_old; 
		gsl_vector_sub(w_help, w); 
		
		cvdebug() << "w-help=" << w_help << "\n"; 
		delta = sqrt(dot(w_help, w_help));

		cvmsg() << "DEFL["<<iter<<"]: delta = " << delta << "\n"; 
		
		if (delta < m_epsilon) {
			if (m_finetune && !is_finetuning) {
				cvinfo() << "DEFL: start fine tuning\n"; 
				is_finetuning = true; 
				maxiter += m_maxFineTune; 
				mu = 0.01 * m_mu; 
			}else{
				converged = true; 
			}
		} else if (m_stabilization) {
			// this checks whether the values just pig-pong

			Vector w_old2_copy = w_old2; 
			gsl_vector_sub(w_old2, w);
			gsl_vector_add(w_old2_copy, w); 
			double delta2 = sqrt(dot(w_old2, w_old2));
			double delta3 = sqrt(dot(w_old2_copy, w_old2_copy));
			cvinfo() << "DEFL: stabelize, mu=" << mu
				 << ", stroke=" << stroke
				 << ", delta2= " << delta2
				 << ", delta3= " << delta3 
				 <<"\n";
			if ( (stroke == 0.0) &&
			     ((delta2 < m_epsilon) || (delta3 < m_epsilon))) {
				stroke = mu; 
				mu *= 0.5; 
			}else if (stroke != 0.0) {
				mu = stroke; 
				stroke = 0.0; 
			}else if (! loong && 2*iter >  m_maxNumIterations ) {
				loong = true; 
				mu *= 0.5; 
			}
		}
		w_old2 = w_old; 
		w_old = w; 
		
		cvdebug() << "w_old=" << w_old << " norm=" << norm << "\n"; 
		
		iter++; 
	}
	
	return delta < m_epsilon; 
}

bool FastICA::fpica_defl(const Matrix& X, Matrix& B)
{ 
	// not yet supported 
	assert(!m_with_initial_guess); 
		
	std::random_device rd;

	// this deterministic seed ensures that the tests can be 
	// run in a controlled environment
	std::mt19937 gen((m_deterministic_seed < 0) ? rd() : m_deterministic_seed);
	std::uniform_real_distribution<> random_source( -0.5, 0.5 ); 

	Vector w( X.rows(), false); 

	m_nonlinearity->set_signal(&X);
	
	bool global_converged = true; 
	
	for (int i = 0; i < m_numOfIC; ++i) {
		
		// initalize vector (should also go into extra class 
		for (unsigned i = 0; i < w.size(); ++i) 
			w[i] = random_source(gen);
		
		bool converged = fpica_defl_round(i, w, B); 
		cvmsg() << "Round(" << i << ")" << (converged ? "converged" : "did not converge") << "\n"; 

		global_converged &= converged; 
		B.set_column(i, w);
	}

	return global_converged; 
}


static double min_abs_diag(const Matrix& m)
{
	unsigned N = m.rows() > m.cols() ? m.rows() : m.cols(); 
	double min_val = fabs(m(0,0)); 
	for (unsigned i = 1; i < N; ++i) {
		double v = fabs(m(i,i)); 
		if ( min_val > v) 
			min_val = v; 
	}
	return min_val; 
}

double FastICA::fpica_symm_step(Matrix& B, Matrix& B_old,double mu, Matrix& workspace)
{
	m_nonlinearity->set_mu(mu); 
	m_nonlinearity->apply(B);
	matrix_inv_sqrt(B);
	multiply_mT_m(workspace, B, B_old); 
	return min_abs_diag(workspace); 
}

bool FastICA::fpica_symm(const Matrix& X, Matrix& B)
{
	// not yet supported 
	assert(!m_with_initial_guess); 

	std::random_device rd;

	// this deterministic seed ensures that the tests can be 
	// run in a controlled environment 
	std::mt19937 gen((m_deterministic_seed < 0) ? rd() : m_deterministic_seed);
	std::uniform_real_distribution<> random_source( -0.5, 0.5 ); 

	Matrix B_old2(B);
	Matrix B_restart(B);
	
	// random and orthogonalize 
	for(auto ib = B.begin(); ib != B.end(); ++ib) 
		*ib = random_source(gen); 

	matrix_orthogonalize(B); 
	Matrix B_old(B);

	m_nonlinearity->set_signal(&X);
	
	bool is_fine_tuning = false; 
	double mu = m_mu;

	Matrix BTB(B.cols(), B.rows(), false); 

	bool converged = false; 
	int iter = 0; 
	double stroke = 0.0; 
	bool loong = false; 

	int maxiter = m_maxNumIterations; 

	bool do_saddle_check = m_do_saddle_check; 
	bool finished = !do_saddle_check;

	int saddle_iter = 0; 

	do {
		while (!converged  && iter < maxiter) {

			double minAbsCos = fpica_symm_step(B, B_old, mu, BTB); 
			
			cvdebug() << "B= "  << B << "\n"; 

			cvmsg() << "FastICA: "<<  iter << ":" << 1.0 - minAbsCos << "\n"; 
			
			if ( 1.0 - minAbsCos < m_epsilon) {
				// run one more time with lower step-width
				if (m_finetune && !is_fine_tuning) {
					mu *= 0.01;
					is_fine_tuning = true; 
					maxiter += m_maxFineTune;
				} else {
					converged = true; 
				}
			} else if (m_stabilization) {
				multiply_mT_m(BTB, B, B_old2); 
				double minAbsCos2 = min_abs_diag(BTB); 
				
				// Avoid ping-pong 
				if (!stroke && (1 - minAbsCos2 < m_epsilon)) {
					stroke = mu; 
					mu /= 2.0; 
				} else if (stroke) { // back to normal 
					mu = stroke; 
					stroke = 0; 
				} else if ( !loong && 2 * iter > m_maxNumIterations) {
					// already running some time and 
					// no convergence, try half step width 
					// once
					loong = true; 
					mu *= 0.5; 
				}
			}
			
			B_old2 = B_old; 
			B_old = B;
			
			++iter; 
		}
		if (do_saddle_check && saddle_iter < m_numOfIC) {
			++saddle_iter; 
			do_saddle_check = run_saddlecheck(B, X);
			if (!do_saddle_check)
				break; 
			converged = false; 
			iter = 0; 
		} else 
			finished = true;
		
	} while (!finished); 
	
	return converged; 	
}


/*
  This is the saddle check as described in 
 
  Petr Tichavský, Zbynek Koldovský, and Erkki Oja
  "Performance Analysis of the FastICA Algorithm and Cramér–Rao "
  "Bounds for Linear Independent Component Analysis"
  IEEE Tran Signal Processing, 54(4), 2006, 1189-1203  
  
  Returns true if at least one check resulted in an update of the
  components.
  
*/ 
bool FastICA::run_saddlecheck(Matrix &B, const Matrix& X)
{
	cvinfo() << "run_saddlecheck \n"; 
	bool result = false; 
	vector<bool> rotated(B.cols(), false); 
	Matrix U(B.cols(), X.cols(), false); 
	multiply_mT_m(U, B, X); 
	auto table = m_nonlinearity->get_saddle_test_table(U);
	
	for (unsigned i = 0; i < U.rows(); ++i) {
		for (unsigned j = i+1; j < U.rows() && !rotated[i]; ++j) {
			if (rotated[j]) 
				continue; 
			auto ui = U.get_row(i);
			auto uj = U.get_row(j);
			const double isqrt2 = 1.0 / sqrt(2.0); 
			auto ui_new = (ui + uj) * isqrt2; 
			auto uj_new = (ui - uj) * isqrt2; 
			
			auto ui_sir = m_nonlinearity->get_saddle_test_value(ui_new);
			auto uj_sir = m_nonlinearity->get_saddle_test_value(uj_new);
			
			if (fmax(ui_sir, uj_sir) > fmax(table[i], table[j])) {
				result = rotated[i] = rotated[j] = true;
				auto bi_new = (B.get_row(i) + B.get_row(j)) * isqrt2; 
				auto bj_new = (B.get_row(i) - B.get_row(j)) * isqrt2; 
				B.set_row(i, bi_new); 
				B.set_row(j, bj_new);
				cvmsg() << "Rotating components " << i << " and " << j << "\n"; 
			}
		}
	}
	return result; 
}; 

void FastICA::set_approach(CIndepCompAnalysis::EApproach apr)
{
        m_approach = apr; 
}

void FastICA::set_nr_of_independent_components (int nrIC)
{
        m_numOfIC = nrIC; 
}


void FastICA::set_nonlinearity (PFastICADeflNonlinearity g)
{
        assert(g); 
        m_nonlinearity = g; 
}


void FastICA::set_finetune (bool finetune)
{
        m_finetune = finetune; 
}

void FastICA::set_mu (double mu)
{
        m_mu = mu; 
}

void FastICA::set_epsilon (double epsilon)
{
        m_epsilon = epsilon; 
}

void FastICA::set_sample_size (double sampleSize)
{
        m_sampleSize = sampleSize; 
}

void FastICA::set_component_energy_limit(double limit)
{
	m_component_energy_limit = limit; 
}

void FastICA::set_stabilization (bool stabilization)
{
        m_stabilization = stabilization; 
}

void FastICA::set_max_num_iterations (int maxNumIterations)
{
        m_maxNumIterations = maxNumIterations; 
}

void FastICA::set_max_fine_tune (int maxFineTune)
{
        m_maxFineTune = maxFineTune; 
}

void FastICA::set_pca_only (bool PCAonly)
{
        m_PCAonly = PCAonly; 
}

void FastICA::set_init_guess (const Matrix&  initGuess)
{
        m_initGuess = initGuess;
}

void FastICA::set_saddle_check(bool saddle_check)
{
	m_do_saddle_check = saddle_check; 
}

void FastICA::set_saddle_check_postiterations(int saddle_postiter)
{
	m_saddle_postiter = saddle_postiter; 
}
	
int FastICA::get_nr_of_independent_components () const
{
        return m_numOfIC; 
}

const Matrix& FastICA::get_mixing_matrix () const
{
        return m_mixing_matrix; 
}

const Matrix& FastICA::get_separating_matrix () const
{
        return m_separating_matrix; 
}

const Matrix& FastICA::get_independent_components () const
{
        return m_independent_components; 
}


const Matrix& FastICA::get_principal_eigenvectors () const
{
        return m_principal_eigenvectors; 
}

const Matrix& FastICA::get_whitening_matrix () const
{
        return m_whitening_matrix; 
}

const Matrix& FastICA::get_dewhitening_matrix () const
{
        return m_dewhitening_matrix; 
}

const Matrix& FastICA::get_white_signal () const
{
        return m_white_sig; 
}

struct CICAAnalysisMIAImpl {

	CICAAnalysisMIAImpl(unsigned int rows, unsigned int length);
	CICAAnalysisMIAImpl(const gsl::Matrix& ic, const gsl::Matrix& mix, const std::vector<double>& mean);
	void normalize_ICs();
	void get_mixing_curve(unsigned int c, vector<float>& curve) const;
	void set_mixing_series(unsigned int index, const std::vector<float>& series);
	void check_set(const CICAAnalysisMIA::IndexSet& s) const;
	std::vector<float> normalize_Mix();

	gsl::Matrix  m_Signal;
	gsl::Matrix  m_ICs;
	gsl::Matrix  m_Mix;
	vector<double> m_mean;

	unsigned int m_ncomponents;
	unsigned int m_nlength;
	unsigned int m_rows;

	int m_max_iterations;
        CIndepCompAnalysis::EApproach m_approach; 
};


CICAAnalysisMIA::CICAAnalysisMIA(const gsl::Matrix& ic, const gsl::Matrix& mix, const std::vector<double>& mean):
	impl(new CICAAnalysisMIAImpl(ic, mix, mean)),
	m_deterministic_seed(-1)
{

}

CICAAnalysisMIA::CICAAnalysisMIA():impl(nullptr),
	m_deterministic_seed(-1)
{

}


void CICAAnalysisMIA::initialize(unsigned int series_length, unsigned int slice_size)
{
	auto new_impl = new CICAAnalysisMIAImpl(series_length, slice_size);
	swap(new_impl, impl);

	if (new_impl)
		delete new_impl;
}

CICAAnalysisMIA::~CICAAnalysisMIA()
{
	delete impl;
}

void CICAAnalysisMIA::set_row_internal(unsigned row, const std::vector<double>&  buffer, double mean)
{
	TRACE_FUNCTION;
	assert(impl);
	assert(row < static_cast<unsigned>(impl->m_Signal.rows()));
	assert(buffer.size() == static_cast<unsigned>(impl->m_Signal.cols()));

	gsl::Vector buf(buffer.size(), &buffer[0]);

	impl->m_Signal.set_row(static_cast<signed>(row), buf);
	impl->m_mean[row] = mean;
	cvdebug() << "add row " << row << ", mean=" << mean << "\n";
}

void CICAAnalysisMIA::set_approach(CIndepCompAnalysis::EApproach approach)
{
	switch (approach) {
	case appr_defl: impl->m_approach = CIndepCompAnalysis::appr_defl;
		break;
	case appr_symm: impl->m_approach = CIndepCompAnalysis::appr_symm;
		break;
	default:
		throw std::invalid_argument("CICAAnalysisMIA:set_approach: Unknown approach given");
	}

}


void CICAAnalysisMIA::set_deterministic_seed(int seed)
{
	TRACE_FUNCTION;
	m_deterministic_seed = seed;
}; 

bool CICAAnalysisMIA::run(unsigned int nica, vector<vector<float> > guess)
{
	TRACE_FUNCTION;
	assert(impl);
	

	FastICA fastICA(nica);
	
	if (m_deterministic_seed >= 0 ) {
		fastICA.set_deterministic_seed(m_deterministic_seed);
	}

        auto nonlin = produce_fastica_nonlinearity("tanh"); 
	fastICA.set_nonlinearity (nonlin); 
		
#if ICA_ALWAYS_USE_SYMM
	// approach APPROACH_DEFL may not return in the unpached itpp 
	// therefore, always use APPROACH_SYMM 
	fastICA.set_approach( appr_symm );
#else
	fastICA.set_approach( impl->m_approach );
#endif
	if (impl->m_max_iterations > 0)
		fastICA.set_max_num_iterations(impl->m_max_iterations);

	if (!guess.empty()) {
                gsl::Matrix mguess(impl->m_Signal.rows(), guess.size(), false);
		for (unsigned int c = 0; c < guess.size(); ++c)
			for (unsigned int r = 0; r < guess.size(); ++r)
				mguess.set(r,c, guess[c][r]); 
		fastICA.set_init_guess(mguess); 
	}
#ifdef ICA_ALWAYS_USE_SYMM
	// in the unpatched itpp separate doesn't return a value
	// therefore, we always assume true
	bool result = true; 
	fastICA.separate(impl->m_Signal);
#else
	bool result = fastICA.separate(impl->m_Signal);
#endif

	impl->m_ICs = fastICA.get_independent_components();
	impl->m_Mix = fastICA.get_mixing_matrix();

	impl->m_ncomponents = impl->m_Mix.cols();
	impl->m_nlength     = impl->m_ICs.cols();
	impl->m_rows        = impl->m_Mix.rows();
	return result; 
}

unsigned int CICAAnalysisMIA::get_ncomponents() const
{
	return impl->m_ncomponents;
}

vector<float> CICAAnalysisMIA::get_feature_row(unsigned int row) const
{
	TRACE_FUNCTION;
	if (row < impl->m_ncomponents) {
		vector<float> result(impl->m_nlength);
		for (unsigned int i = 0; i < impl->m_nlength; ++i)
			result[i] = static_cast<float>(impl->m_ICs(row, i));
		return result;
	}

	throw create_exception<invalid_argument>("CICAAnalysisMIA::get_feature_row: requested row ", row, " out of range");

}

std::vector<float> CICAAnalysisMIA::get_mix_series(unsigned int colm)const
{
	TRACE_FUNCTION;
	if (colm < impl->m_ncomponents) {
		vector<float> result(impl->m_rows);
		impl->get_mixing_curve(colm, result);
		return result;
	}
	throw create_exception<invalid_argument>("CICAAnalysisMIA::get_mix_series: requested series ", colm, " out of range");
}

void CICAAnalysisMIA::set_mixing_series(unsigned int index, const std::vector<float>& filtered_series)
{
	impl->set_mixing_series(index, filtered_series);
}

void CICAAnalysisMIAImpl::set_mixing_series(unsigned int index, const std::vector<float>& series)
{
	TRACE_FUNCTION;
	assert(m_rows == series.size());
	assert(index < m_nlength);
	for (unsigned int i = 0; i < m_rows; ++i)
		m_Mix.set(i, index, series[i]);

}


CSlopeColumns CICAAnalysisMIA::get_mixing_curves() const
{
	TRACE_FUNCTION;
	CSlopeColumns result(impl->m_ncomponents);
	for (unsigned int c = 0; c < impl->m_ncomponents; ++c)
		impl->get_mixing_curve(c, result[c]);
	return result;
}

void CICAAnalysisMIAImpl::get_mixing_curve(unsigned int c, vector<float>& curve) const
{
	TRACE_FUNCTION;
	curve.resize(m_rows);
	for (unsigned int i = 0; i < m_rows; ++i)
		curve[i] = static_cast<float>(m_Mix(i, c));
}

std::vector<float> CICAAnalysisMIA::get_mix(unsigned int idx)const
{
	TRACE_FUNCTION;
	if (idx < impl->m_rows) {
		vector<float> result(impl->m_nlength, static_cast<float>(impl->m_mean[idx]));
		for (unsigned int i = 0; i < impl->m_nlength; ++i) {
			for (unsigned int c = 0; c < impl->m_ncomponents; ++c) {
				result[i] += impl->m_ICs(c, i) *  impl->m_Mix(idx, c);
                        }
		}
		return result;
	}

	throw create_exception<invalid_argument>("CICAAnalysisMIA::get_mix: requested idx ", idx, " out of range: ", impl->m_rows);
}

std::vector<float> CICAAnalysisMIA::get_delta_feature(const IndexSet& plus, const IndexSet& minus)const
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

std::vector<float> CICAAnalysisMIA::get_partial_mix(unsigned int idx, const IndexSet& cps)const
{
	TRACE_FUNCTION;
	if (idx >=  impl->m_rows) {
		throw create_exception<invalid_argument>("CICAAnalysisMIA::get_mix: requested idx ", idx,
							 " out of range: ", impl->m_rows);

	} else {
		impl->check_set(cps);
		vector<float> result(impl->m_nlength, static_cast<float>(impl->m_mean[idx]));
		for (unsigned int i = 0; i < impl->m_nlength; ++i) {
			for (auto c = cps.begin(); c != cps.end(); ++c) {
                                cvdebug() << " idx = " << idx <<  ", i = " << i  << ", c=" << *c<< std::endl; 
				result[i] += impl->m_ICs(*c, i) *  impl->m_Mix(idx, *c);
                        }
		}
		return result;
	}
}

std::vector<float> CICAAnalysisMIA::get_incomplete_mix(unsigned int idx, const std::set<unsigned int>& skip)const
{
	TRACE_FUNCTION;
	if (idx < impl->m_rows) {
                IndexSet use_set; 
                for (unsigned int c = 0; c < impl->m_ncomponents; ++c)
                        if (skip.find(c) == skip.end())
                                use_set.insert(c); 
                
		return get_partial_mix(idx, use_set); 
	}

	throw create_exception<invalid_argument>("CICAAnalysisMIA::get_mix: requested idx ", idx,
						 " out of range: ", impl->m_rows);
}

std::vector<float> CICAAnalysisMIA::normalize_Mix()
{
	return impl->normalize_Mix();
}

CICAAnalysisMIAImpl::CICAAnalysisMIAImpl(unsigned int rows, unsigned int length):
	m_Signal(rows, length, false),
	m_mean(rows),
	m_ncomponents(0),
	m_nlength(0),
	m_rows(0),
	m_max_iterations(0), 
	m_approach(CIndepCompAnalysis::appr_defl)
{
	cvdebug() << "Analyis of signal with " << rows << " data rows of " << length << " entries\n";
}

CICAAnalysisMIAImpl::CICAAnalysisMIAImpl(const gsl::Matrix& ic, const gsl::Matrix& mix, const std::vector<double>& mean):
	// Coverty may complain here, but the order is correct
	// the signal is a matrix of the number of columns of the mixing matrix = time points 
	// by rows of the independent components - each row constitutes th epixel ogf a feature image
	// coverity[swapped_arguments]
	m_Signal(mix.cols(), ic.rows(), false),
	m_ICs(ic),
	m_Mix(mix),
	m_mean(mean),
	m_ncomponents(ic.rows()),
	m_nlength(ic.cols()),
	m_rows(mix.rows()),
	m_max_iterations(0), 
	m_approach(CIndepCompAnalysis::appr_defl)
{
	TRACE_FUNCTION;
}

void CICAAnalysisMIA::normalize_ICs()
{
	TRACE_FUNCTION;
	impl->normalize_ICs();
}

void CICAAnalysisMIAImpl::check_set(const CICAAnalysisMIA::IndexSet& s) const
{
	for (CICAAnalysisMIA::IndexSet::const_iterator is = s.begin();
	     is != s.end(); ++is) {
		if (*is >= m_ncomponents) {
			throw create_exception<invalid_argument>("CICAAnalysisMIA: request component index ",  *is ,
								 " but only up to index ", m_ncomponents - 1, 
								 " available\n");
		}
	}
}

void CICAAnalysisMIAImpl::normalize_ICs()
{
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
				m_ICs.set(c, k, (m_ICs(c, k) - ic_shift) * ic_factor);
			
			for (unsigned int r = 0; r < m_rows; ++r) {
				m_mean[r] += m_Mix(r, c) * ic_shift;
				m_Mix.set(r, c, m_Mix(r, c) * mix_factor);
			}
		}
	}
	

}

std::vector<float> CICAAnalysisMIAImpl::normalize_Mix()
{
	std::vector<float> result(m_nlength,0.0f);

	for (unsigned int c = 0; c < m_ncomponents; ++c) {
		float mean = 0.0f;
		for (unsigned int r = 0; r < m_rows; ++r)
			mean += m_Mix(r, c);
		mean /= m_rows;
		for (unsigned int r = 0; r < m_rows; ++r)
			m_Mix.set(r, c, m_Mix(r,c) - mean);

		for (unsigned int k = 0; k < m_nlength; ++k)
			result[k] += mean * m_ICs(c, k);
	}
	for (unsigned int c = 0; c < m_ncomponents; ++c) {
		// invert sign so that all mixing curves start with a value < 0 
		if (m_Mix(0, c) > 0) {
			for (unsigned int r = 0; r < m_rows; ++r)
				m_Mix.set(r, c, -m_Mix(r, c));
			for (unsigned int k = 0; k < m_nlength; ++k)
				m_ICs.set(c, k, -m_ICs(c, k)); 
		}
	}
	
	return result;
}


void CICAAnalysisMIA::set_max_iterations(int n)
{
	impl->m_max_iterations = n;
}

CIndepCompAnalysis *CICAAnalysisMIAFactory::do_create() const
{
	return new CICAAnalysisMIA;
}

CICAAnalysisMIAFactoryPlugin::CICAAnalysisMIAFactoryPlugin():
	CIndepCompAnalysisFactoryPlugin("internal")
{
}

CIndepCompAnalysisFactory *CICAAnalysisMIAFactoryPlugin::do_create() const
{
	return new CICAAnalysisMIAFactory; 
}

const std::string CICAAnalysisMIAFactoryPlugin::do_get_descr()const
{
	return "This is the MIA implementation of the FastICA algorithm."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CICAAnalysisMIAFactoryPlugin;
}


NS_END  
