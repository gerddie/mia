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

#ifndef mia_core_fastica_hh
#define mia_core_fastica_hh

#include <memory>
#include <gsl++/matrix.hh>
#include <mia/core/fastica_nonlinearity.hh>

namespace mia {

/**
   
 */

class FastICA {
public: 
	enum EApproach {appr_defl, appr_symm}; 

	FastICA(const gsl::Matrix&  mix);

	bool separate(); 

	void set_approach(EApproach apr); 
	
	void set_nr_of_independent_components (int in_nrIC); 
	
	void set_nonlinearity (PFastICADeflNonlinearity in_g); 
	
	void set_finetune (bool in_finetune); 
		
 	void set_mu (double mu); 
	
 	void set_epsilon (double epsilon); 

 	void set_sample_size (double sampleSize);

 	void set_stabilization (bool in_stabilization);

 	void set_max_num_iterations (int in_maxNumIterations);

 	void set_max_fine_tune (int in_maxFineTune);

	void set_pca_only (bool in_PCAonly);
 
	void set_init_guess (const gsl::Matrix&  ma_initGuess);

	void set_saddle_check(bool saddle_check); 

	const gsl::Matrix& 	get_mixing_matrix () const;

	const gsl::Matrix& 	get_separating_matrix () const;

	const gsl::Matrix&	get_independent_components () const;

	int  get_nr_of_independent_components () const;

	const gsl::Matrix&	get_whitening_matrix () const;

	const gsl::Matrix&	get_dewhitening_matrix () const;
	
	const gsl::Matrix&	get_principal_eigenvectors () const; 
	
	const gsl::Matrix&	get_white_signal () const;
	
private:
	// evaluate the whitening and de-whitening matrices 
	void evaluate_whiten_matrix(const gsl::Matrix& evec, const gsl::Vector& eval); 
        bool fpica_defl_round(int component, gsl::Vector& w, gsl::Matrix& B); 
	bool fpica_defl(const gsl::Matrix& X); 
	double fpica_symm_step(gsl::Matrix& B, gsl::Matrix& B_old, double mu, gsl::Matrix& Workspace); 
	bool fpica_symm(const gsl::Matrix& X); 
	bool run_saddlecheck(gsl::Matrix &B, const gsl::Matrix& X); 

	const gsl::Matrix&  m_mix;
	
	EApproach m_approach; 
	
	int m_numOfIC; 
	
	PFastICADeflNonlinearity m_nonlinearity; 
	
	bool m_finetune; 
	
	double m_mu; 
	
 	double m_epsilon; 
	
 	double m_sampleSize;
	
 	bool m_stabilization;
	
 	int m_maxNumIterations;
	
 	int m_maxFineTune;
	
 	int m_firstEig;
	
	int m_lastEig;
	
	bool m_PCAonly;
	
	bool m_with_initial_guess; 

	bool m_do_saddle_check; 
	
	gsl::Matrix m_initGuess;
	
	gsl::Matrix m_mixing_matrix;
	
	gsl::Matrix m_separating_matrix;
	
	gsl::Matrix	m_independent_components;
	
	gsl::Matrix	m_whitening_matrix;
	
	gsl::Matrix	m_dewhitening_matrix;

        gsl::Matrix	m_principal_eigenvectors; 

        gsl::Matrix	m_white_sig; 

	
};

}

#endif 
