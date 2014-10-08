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

#define FICA_TOL 1e-9; 

#include <gsl++/fastica.hh>
#include <gsl++/matrix_vector_ops.hh>
#include <algorithm>
#include <random>
#include <gsl/gsl_blas.h>

namespace mia {

using namespace gsl; 

using std::sort; 

FastICA::FastICA(const Matrix&  mix):
        m_mix(mix), 
        m_approach(appr_defl), 
        m_nrIC(m_mix.rows()), 
        m_nonlinearity(new FNonlinPow3), 
        m_finetune(false), 
        m_mu(1.0), 
        m_epsilon(0.0001), 
        m_sampleSize(1.0), 
        m_stabilization(false), 
        m_maxNumIterations(100000), 
        m_maxFineTune(100), 
        m_firstEig(1), 
        m_lastEig(m_mix.rows()), 
        m_PCAonly(false), 
	m_with_initial_guess(false)
{
}

static 
void remove_mean(const Matrix& mixedSig, Matrix& mixedSigCentered, DoubleVector& mixed_mean)
{
	for (size_t r = 0; r < mixedSig.rows(); ++r) {
		for (size_t c = 0; c < mixedSig.cols(); ++c)
			mixed_mean[r] += mixedSig(r,c); 
		mixed_mean[r] /= mixedSig.cols(); 
		for (size_t c = 0; c < mixedSig.cols(); ++c)
			mixedSigCentered.set(r,c, mixedSig(r,c) - mixed_mean[r]); 
	}
}

Matrix FastICA::whiten(const Matrix& signal, const Matrix& evec, const DoubleVector& eval)
{
	m_whitening_matrix.reset(evec.cols(), evec.rows(), false); 
	m_dewhitening_matrix.reset(evec.rows(), evec.cols(), false);
	
	for (int i = 0; i < eval.size(); i++) {
		double iwscale = sqrt(eval[i]);
		double wscale = 1.0 / iwscale;
		
		auto wmr = gsl_matrix_row(m_whitening_matrix, i); 
		auto inv = gsl_matrix_row(evec, i);
		auto dwmc = gsl_matrix_column(m_dewhitening_matrix, i); 
		
		gsl_vector_memcpy (&wmr.vector, &inv.vector); 
		gsl_blas_dscal (wscale, &wmr.vector); 
		
		gsl_vector_memcpy (&dwmc.vector, &inv.vector); 
		gsl_blas_dscal (iwscale, &dwmc.vector); 
	}
	
	Matrix result(evec.rows(), mixedSigC.cols(), true); 
	
	multiply_m_m(result, m_whitening_matrix, signal); 
	
	return result; 
}

bool FastICA::separate()
{

	int dim = m_numOfIC;

	Matrix mixedSigC(m_mixedSig.rows(), m_mixedSig.cols(), true);
	DoubleVector mixedMean(m_mixedSig.rows(), clear);

	Matrix guess;
	if (m_initGuess.rows() != dim || m_initGuess.cols() != dim)
		guess = Matrix(Dim, Dim, true);
	else
		guess = m_initGuess;

	Matrix processing(m_mixedSig.rows(), m_numOfIC, clear);

	m_icasig = Matrix(m_numOfIC, m_mixedSig.cols(), clear);

	remove_mean(m_mixedSig, mixedSigCentered, mixed_mean);
	
	PCA pca(m_numOfIC, 0.9);
	auto pca_result = pca.analyze(m_mixedSig); 
	
	if (pca_result.eval.size() < 1) {
		m_icasig = m_mixedSig;
		return false;
	}

	m_numOfIC = pca_result.evec.size(); 
	
	auto whitened_signal = whithen(mixedSigCentered, pca_result.evec, pca_result.eval);

	// if only PCA, stop here and save the dewithening as independend components 
	// where is the mixing matrix? 
	if (m_PCAonly) {
		m_mixing_matrix.reset(mixedSigCentered.rows(), m_dewhitening_matrix.cols(), 1.0);
		m_independent_components = m_dewhitening_matrix; 
		return true; 
	}
	
	switch (m_approach) {
	case appr_defl: return fpica_defl(whitened_signal);
	case appr_symm: return fpica_symm(whitened_signal);
	default: 
		throw invalid_argument("FastICA::separate: unknown apporach given"); 
		
	}
}

bool FastICA::fpica_defl_round(int component, DoubleVector& w)
{
	
	double mu = m_mu;
	
	double old_delta = m_epsilon + 2.0;
	double delta = m_epsilon + 1.0;
	DoubleVector w_old(w);
	DoubleVector w_old2(w);
	int iter = 0;

	boof is_finetuning = false;  
	bool converged = false; 
	int finetune = 0; 
	
	while (iter < m_maxNumIterations + finetune) {
			
		cvdebug() << "Defl: c=" << component
			  << ", iter= " << iter 
			  << ", delta= " << delta
			  << "\n"; 
		
		m_nonlinearity->set_mu(mu); 
		m_nonlinearity->apply(w); 
		
		DoubleVector w_save(w); 
		for (int j = 0; j < component; ++j) {
			const double wdot = m_separating_matrix.dot_row(j, w_save); 
			cblas_daxpy(N, -wdot, wj->data, wj->stride, w->data, w->stride);
		}
		
		double norm = sqrt(dot(w, w));
		if (norm > 0.0) 
			gsl_vector_scale(w, 1.0/norm); 

		DoubleVector w_help = w_old; 
		gsl_vector_sub(w_help, w); 
		
		double delta = sqrt(dot(w_help, w_help));

		cvinfo() << "DEFL["<<iter<<"]: delta = " << delta << "\n"; 

		if (delta < m_epsilon) {
			if (m_finetune && !is_finetuning) {
				cvinfo() << "DEFL: start fine tuning\n"; 
				is_finetuning = true; 
				finetune = m_maxFineTune; 
				mu = 0.01 * m_mu; 
			}else{
				converged = true; 
			}
		} else if (m_stabilization) {
			gsl_vector_sub(w_old2, w);
			double delta2 = sqrt(dot(w_old2, w_old2));
			if ( (stroke == 0.0) && (delta2 < m_epsilon)) {
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
		
		
		iter++; 
	}
	
	return delta < m_epsilon; 
}

bool FastICA::fpica_defl(const Matrix& X)
{ 
	// not yet supported 
	assert(!m_with_initial_guess); 
		
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> random_source( -0.5, 0.5 ); 

	m_separating_matrix.reset(m_numOfIC, X.cols(), false); 
	const int N = X.rows(); 
	DoubleVector w( X.rows(), false); 
	
	const double inv_m = 1.0 / X.cols();
	
	m_nonlinearity->set_signal(&X);
	
	bool global_converged = true; 
	
	for (int i = 0; i < m_numOfIC; ++i) {
		
		// initalize vector (should also go into extra class 
		for (unsigned i = 0; i < w.size(); ++i) 
			w[i] = random_source(gen);
		
		bool converged = fpica_defl_round(i, w, false); 
		if (!converged && m_stabilization) {
			converged = fpica_defl_round(i, w, true);
		}
		
		global_converged &= converged; 
		m_separating_matrix.set_row(i, w);
	}

	m_independent_components.reset(m_numOfIC, X.cols(), false); 
	multiply(m_independent_components, m_separating_matrix, X); 
	
	m_mixing_matrix.reset(X.rows(), m_numOfIC, false); 
	multiply(m_mixing_matrix, m_dewhitening_matrix, m_separating_matrix); 
	
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

static void msqrtm(Matrix& m )
{

	CSymmvEvalEvec see(m);

	wm = ws; 
	for (unsigned r = 0; r < wm.rows(); ++r) {
		auto wmr = gsl_matrix_row(see.evec, c); 
		gsl_vector_multiply(&wmr.vector, 1.0/ sqrt(see.eval[c])); 
	}
	multiply_m_mT(m, see.evec, see.evec); 
}

bool FastICA::fpica_symm(const Matrix& X)
{
	// not yet supported 
	assert(!m_with_initial_guess); 

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> random_source( -0.5, 0.5 ); 

	m_separating_matrix.reset(m_numOfIC, X.cols(), true); 

	Matrix w_old(m_separating_matrix);
	Matrix w_restart(m_separating_matrix);
	Matrix w_new(m_separating_matrix);
	
	// random and orthogonalize 
	for(unsigned r = 0; r <  m_numOfIC; ++r) 
		for(unsigned c = 0; c <  m_numOfIC; ++c){
			m_separating_matrix.set(r, c, random_source(gen)); 
		}
	
	matrix_orthogonalize(m_separating_matrix); 
	m_nonlinearity->set_signal(&X);
	
	bool is_fine_tuning = false; 
	double mu = m_mu;

	Matrix BTB(m_separating_matrix.cols(), m_separating_matrix.cols(), false); 

	bool converged = false; 
	unsigned iter = 0; 
	double stroke = 0.0; 
	bool loong = false; 

	while (!converged  && iter < m_maxNumIterations) {
		
		m_nonlinearity->set_mu(mu); 
		m_nonlinearity->apply(m_separating_matrix);

		// re-orthognalize 
		multiply_mT_m(BTB, m_separating_matrix, m_separating_matrix); 
		msqrtm(BTB);
		multiply_m_m(w_new, m_separating_matrix, BTB); 
		
		multiply_mT_m(BTB, w_new, m_separating_matrix); 
		double minAbsCos = min_abs_diag(BTB); 
		
		if ( 1.0 - minAbsCos < m_epsilon) {
			// run one more time with lower step-width
			if (m_finetune && !is_fine_tuning) {
				mu *= 0.01;
				is_fine_tuning = true; 
			}
			else {
				converged = true; 
			}
		} else if (m_stabilization) {
			multiply_mT_m(BTB, w_new, m_old); 
			double minAbsCos2 = min_abs_diag(BTB); 
			
			// Avoid ping-pong 
			if (!stroke && (1 - minAbsCos2 < m_epsilon)) {
				stroke = mu; 
				mu /= 2.0; 
			} else if (stroke) { // back to normal 
				mu = stroke; 
				stroke == 0; 
			} else if ( !loong && 2 * iter > m_maxNumIterations) {
				// already running some time and 
				// no convergence, try half step width 
				// once
				loong = true; 
				mu *= 0.5; 
			}
		}
		
		w_old = m_separating_matrix;
		m_separating_matrix = w_new; 
		
		++iter; 
	}
	
	m_independent_components.reset(m_numOfIC, X.cols(), false); 
	multiply(m_independent_components, m_separating_matrix, X); 
	
	m_mixing_matrix.reset(X.rows(), m_numOfIC, false); 
	multiply(m_mixing_matrix, m_dewhitening_matrix, m_separating_matrix); 
				   
	return converged; 
	
}

void FastICA::set_approach(EApproach apr)
{
        m_approach = apr; 
}

void FastICA::set_nrof_independent_components (int nrIC)
{
        m_nrIC = nrIC; 
}


void FastICA::set_non_linearity (FNonlinearity *g)
{
        assert(g); 
        m_nonlinearity.reset(g); 
}


void FastICA::set_fine_tune (bool finetune)
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

void 	FastICA::set_stabilization (bool stabilization)
{
        m_stabilization = stabilization; 
}

void 	FastICA::set_max_num_iterations (int maxNumIterations)
{
        m_maxNumIterations = maxNumIterations; 
}

void 	FastICA::set_max_fine_tune (int maxFineTune)
{
        m_maxFineTune = maxFineTune; 
}

void 	FastICA::set_first_eig (int firstEig)
{
        m_firstEig = firstEig; 
}

void 	FastICA::set_last_eig (int lastEig)
{
        m_lastEig = lastEig; 
}

void 	FastICA::set_pca_only (bool PCAonly)
{
        m_PCAonly = PCAonly; 
}

void 	FastICA::set_init_guess (const Matrix&  initGuess)
{
        m_initGuess = initGuess;
}

const Matrix& 	FastICA::get_mixing_matrix () const
{
        return m_mixing_matrix; 
}

const Matrix& 	FastICA::get_separating_matrix () const
{
        return m_separating_matrix; 
}

const Matrix&	FastICA::get_independent_components () const
{
        return m_independent_components; 
}

int FastICA::get_nrof_independent_components () const
{
        return m_numOfIC; 
}

const Matrix&	FastICA::get_principal_eigenvectors () const
{
        return m_principal_eigenvectors; 
}

const Matrix&	FastICA::get_whitening_matrix () const
{
        return m_whitening_matrix; 
}

const Matrix&	FastICA::get_dewhitening_matrix () const
{
        return m_dewhitening_matrix; 
}

const Matrix&	FastICA::get_white_sig () const
{
        return m_white_sig; 
}

void FastICA::FNonlinearity::set_sample(double sample_size, size_t num_samples)
{
	m_sample_size = sample_size; 
	m_num_samples = num_samples;
}

void FastICA::FNonlinearity::set_signal(const Matrix *signal)
{
	m_signal = signal; 
	assert(m_signal); 

	m_workspace = DoubleVector(m_signal->cols());  
	m_workspace2 = DoubleVector(m_signal->rows());
	post_set_signal(); 
}

void FastICA::FNonlinearity::set_scaling(double myy)
{
	assert(myy != 0.0); 
	m_myy = myy; 
}

double FastICA::FNonlinearity::get_sample_size() const
{
	return m_sample_size; 
}

size_t FastICA::FNonlinearity::get_num_samples() const
{
	return m_sum_samples; 
}

double FastICA::FNonlinearity::get_scaling() const
{
	return m_myy; 
}

const Matrix& FastICA::FNonlinearity::get_signal() const
{
	assert(m_signal); 
	return *m_signal; 
}

void FastICA::FNonlinearity::post_set_signal()
{
}

void FNonlinPow3::apply(gsl::Matrix& W, gsl::Matrix& WtX) const
{
	transform(WtX.begin(), WtX.end(), WtX.begin(), [](double x)(return x*x*x;}); 
	
#error m_matrix_workspace not prepared 
	multiply_m_m(m_matrix_workspace,  get_signal(), WtX); 

	transform(WtX.begin(), WtX.end(), WtX.begin(), [](double x)(return x*x*x;}); 
	
	double inv_m = 1.0 / m_signal.rows(); 
	transform(W.begin(), W.end(), m_matrix_workspace.begin(), W.begin(), 
		  [inv_m](double w, double x){return x * inv_m - 3 * w;}); 
}


void FNonlinTanh::apply(gsl::Matrix& W, gsl::Matrix& WtX) const
{
	transform(WtX.begin(), WtX.end(), WtX.begin(), [this](double x){return tanh(m_a1 * x);}); 
	for(unsigned c = 0; c < WtX.cols(); ++t) {
		double sum = 0.0; 
		auto wtx_col = gsl_matrix_get_const_column(WtX, c); 
		const DoubleVector wc(&wtx_col.vector); 
		for (unsigned r = 0; r < WtX.rows(); ++r) {
			sum += 1 - wc[r] * wc[r]; 
		}
		m_workspace[c] = sum; 
	}
#error m_matrix_workspace not prepared 
	multiply_m_m(m_matrix_workspace,  get_signal(), WtX); 

	// test fist !!!
	

}

void FNonlinGauss::post_set_signal()
{
	m_workspace3 = DoubleVector(m_signal->rows());
}

void FNonlinGauss::apply(DoubleVector& w, const DoubleVector& wtX) const
{
	transform(m_wtX.begin(), m_wtX.end(), m_workspace.begin(), 
		  [](double x) {return x * x; }); 
	
	transform(m_workspace.begin(), m_workspace.end(), m_workspace3.begin(),
		  [](double x) { return exp(- x / 2.0);}); 
	
	transform(m_wtX.begin(), m_wtX.end(), m_workspace3.begin(), m_workspace.begin()
		  [](double u, double expu2) {return u * expu2}); 
	
	multiply_m_v(m_workspace2, get_signal(), m_workspace);
	
	transform(m_wtX.begin(), m_wtX.end(), m_workspace3.begin(), m_workspace3.begin(),
		  [](double u, double expu2) { return (1 - u*u) * expu2;});
	
	double scale = 0.0; 
	for_each(m_workspace3.begin(), m_workspace3.end(), [this, &scale](double x) {
			scale += x;
		}); 
	
	cblas_daxpy(m_workspace2.size(), scale, w->data, w->stride,
		    m_workspace2->data, m_workspace2->stride); 
	
	transform(m_workspace2.begin(), m_workspace2.end(), w.begin(), 
		  [inv_m](double x) { return x * inv_m;}); 
}


}; 
