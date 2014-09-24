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

#include <gsl++/fastica.hh>

namespace gsl {

FastICA::FastICA(const Matrix&  mix):
        m_mix(mix), 
        m_approach(appr_defl), 
        m_nrIC(m_mix.rows()), 
        m_nonlinearity(new FNonlinPow3), 
        m_finetune(false), 
        m_a1(1.0), 
        m_a2(1.0), 
        m_mu(1.0), 
        m_epsilon(0.0001), 
        m_sampleSize(1.0), 
        m_stabilization(false), 
        m_maxNumIterations(100000), 
        m_maxFineTune(100), 
        m_firstEig(1), 
        m_lastEig(m_mix.rows()), 
        m_PCAonly(false), 
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


static 
int pcamat(const Matrix& vectors, int numOfIC, int firstEig, int lastEig, Matrix& E, DoubleVector& D)
{
	Matrix Et;
	DoubleVector  Dt;
	Matrix Ec;
	DoubleVector  Dc;
	double lowerLimitValue = 0.0,
		higherLimitValue = 0.0;
	
	int oldDimension = vectors.rows();
	
	Matrix covarianceMatrix = covariance_transposed(vectors);
	
	eig_sym(covarianceMatrix, Dt, Et);
	
	int maxLastEig = 0;
	
	// Compute rank
	for (int i = 0; i < Dt.length(); i++) if (Dt(i) > FICA_TOL) maxLastEig++;
	if (maxLastEig < 1) return 0;
	
	// Force numOfIC components
	if (maxLastEig > numOfIC) maxLastEig = numOfIC;
	
	vec eigenvalues = zeros(size(Dt));
	vec eigenvalues2 = zeros(size(Dt));
	
	eigenvalues2 = Dt;
	
	sort(eigenvalues2);
	
	vec lowerColumns = zeros(size(Dt));
	
	for (int i = 0; i < size(Dt); i++) eigenvalues(i) = eigenvalues2(size(Dt) - i - 1);
	
	if (lastEig > maxLastEig) lastEig = maxLastEig;
	
	if (lastEig < oldDimension) lowerLimitValue = (eigenvalues(lastEig - 1) + eigenvalues(lastEig)) / 2;
	else lowerLimitValue = eigenvalues(oldDimension - 1) - 1;
	
	for (int i = 0; i < size(Dt); i++) if (Dt(i) > lowerLimitValue) lowerColumns(i) = 1;
	
	if (firstEig > 1) higherLimitValue = (eigenvalues(firstEig - 2) + eigenvalues(firstEig - 1)) / 2;
	else higherLimitValue = eigenvalues(0) + 1;
	
	vec higherColumns = zeros(size(Dt));
	for (int i = 0; i < size(Dt); i++) if (Dt(i) < higherLimitValue) higherColumns(i) = 1;
	
	vec selectedColumns = zeros(size(Dt));
	for (int i = 0; i < size(Dt); i++) selectedColumns(i) = (lowerColumns(i) == 1 && higherColumns(i) == 1) ? 1 : 0;
	
	selcol(Et, selectedColumns, Es);
	
	int numTaken = 0;
	
	for (int i = 0; i < size(selectedColumns); i++) if (selectedColumns(i) == 1) numTaken++;
	
	Ds = zeros(numTaken);
	
	numTaken = 0;
	
	for (int i = 0; i < size(Dt); i++)
		if (selectedColumns(i) == 1) {
			Ds(numTaken) = Dt(i);
			numTaken++;
		}
	
	return lastEig;
}

bool FastICA::separate()
{

	int dim = numOfIC;

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
	
	Matrix E; 
	DoubleVector D; 
	
	
	if (pcamat(mixedSigCentered, m_numOfIC, m_firstEig, m_lastEig, E, D) < 1) {
		// no principal components could be found (e.g. all-zero data): return the unchanged input
		m_icasig = m_mixedSig;
		return false;
	}
	
	whitenv(mixedSigC, E, diag(D), whitesig, whiteningMatrix, dewhiteningMatrix);

  Dim = whitesig.rows();
  if (numOfIC > Dim) numOfIC = Dim;

  ivec NcFirst = to_ivec(zeros(numOfIC));
  vec NcVp = D;
  for (int i = 0; i < NcFirst.size(); i++) {

    NcFirst(i) = max_index(NcVp);
    NcVp(NcFirst(i)) = 0.0;
    VecPr.set_col(i, dewhiteningMatrix.get_col(i));

  }

  bool result = true;
  if (PCAonly == false) {

    result = fpica(whitesig, whiteningMatrix, dewhiteningMatrix, approach, numOfIC, g, finetune, a1, a2, mu, stabilization, epsilon, maxNumIterations, maxFineTune, initState, guess, sampleSize, A, W);

    icasig = W * mixedSig;

  }

  else { // PCA only : returns E as IcaSig
    icasig = VecPr;
  }
  return result;

        
	
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

void FastICA::set_a1 (double a1)
{
        m_a1 = a1; 
}

void FastICA::set_a2 (double a2)
{
        m_a2 = a2; 
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



}; 
