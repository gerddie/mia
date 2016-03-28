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
#include <mia/core/gsl_matrix.hh>
#include <mia/core/fastica_nonlinearity.hh>

namespace mia {

/**
   This class implements the FastICA blind source separation

   As additional feature the saddle-check [1] can be run to
   improve the obtained results.
   
   [1] Petr Tichavský, Zbynek Koldovský, and Erkki Oja
   "Performance Analysis of the FastICA Algorithm and Cramér–Rao "
   "Bounds for Linear Independent Component Analysis"
   IEEE Tran Signal Processing, 54(4), 2006, 1189-1203  

 */

class EXPORT_CORE FastICA {
public: 

	/**
	   Separation approach to be used.  
	 */
	enum EApproach {
		appr_defl, /**< Deflation approach - each component is extimated separately */ 
		appr_symm  /**< Symmetric approach thet estimates all components at the same time */
	}; 


	/**
	   Construct the FastICA algorithms with the number of expected components. 
	   \param num_ic will seperate in at most this number of components 
	*/
	FastICA(int num_ic);

	/**
	   Separate the given signal 
	   \param mix the mixed signal, each row contains a time step 
	   \returns true if the method converged 
	*/
	bool separate(const gsl::Matrix&  mix); 

	/**
	   Set the separation approach to be used. 
	   \param apr appr_delf|appr_symm 
	*/
	void set_approach(EApproach apr); 
	
	/**
	   Set the limit of the energy sum used to estimate the number of 
	   components that make sense based on the PCA 
	   \param limit energy limit, set >= 1.0 if you want to enforce the number 
	*/
	void set_component_energy_limit(double limit); 
	
	/**
	   Set the number of components to be estimated 
	   \param nrIC (maximum) number of components to be estimated 
	   
	 */
	void set_nr_of_independent_components (int nrIC); 
	
	/**
	   Set the non-linearity to be used for separation 
	 */
	void set_nonlinearity (PFastICADeflNonlinearity in_g); 
	
	/**
	   Set if fine tuning is to be applied 
	 */ 
	void set_finetune (bool in_finetune); 
		
	/**
	   Set parameter \a mu to run the iteration 
	 */ 
 	void set_mu (double mu); 
	
	/**
	   Iteration breaking condition, i.e. maximum change in mixing vector 
	*/ 
 	void set_epsilon (double epsilon); 

	/**
	   Sample size to be used (not yet implemented) 
	 */
 	void set_sample_size (double sampleSize);

	/**
	   Set if stablilization is to be used. 
	*/
 	void set_stabilization (bool in_stabilization);

	/**
	   Set maximum number of iteration to run
	 */
 	void set_max_num_iterations (int maxNumIterations);

	/**
	   Set number of additional fine-tune iterations that might be run 
	*/ 
 	void set_max_fine_tune (int maxFineTune);

	/**
	   Run only PCA (Why is this here, PCA is a aseparate class?) 
	 */
	void set_pca_only (bool in_PCAonly);
 
	/**
	   Set an initial guess (not yet used) 
	 */
	void set_init_guess (const gsl::Matrix&  ma_initGuess);
	
	/**
	   Use the saddle check in the symetric method
	*/
	void set_saddle_check(bool saddle_check); 

	/**
	   Number of iterations after a sucessful saddle check 
	*/
	void set_saddle_check_postiterations(int saddle_postiter); 

	/// \returns number of estimated independedn components 
	int  get_nr_of_independent_components () const;
	
	
	const gsl::Matrix& 	get_mixing_matrix () const;

	const gsl::Matrix& 	get_separating_matrix () const;

	const gsl::Matrix&	get_independent_components () const;



	const gsl::Matrix&	get_whitening_matrix () const;

	const gsl::Matrix&	get_dewhitening_matrix () const;
	
	const gsl::Matrix&	get_principal_eigenvectors () const; 
	
	const gsl::Matrix&	get_white_signal () const;
	
private:
	// evaluate the whitening and de-whitening matrices 
	void evaluate_whiten_matrix(const gsl::Matrix& evec, const gsl::Vector& eval); 
    bool fpica_defl_round(int component, gsl::Vector& w, gsl::Matrix& B);
	bool fpica_defl(const gsl::Matrix& X, gsl::Matrix& B); 
	double fpica_symm_step(gsl::Matrix& B, gsl::Matrix& B_old, double mu, gsl::Matrix& Workspace); 
	bool fpica_symm(const gsl::Matrix& X, gsl::Matrix& B); 
	bool run_saddlecheck(gsl::Matrix &B, const gsl::Matrix& X); 

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

	double m_component_energy_limit; 
	
	bool m_with_initial_guess; 

	bool m_do_saddle_check; 

	int m_saddle_postiter; 
		
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
