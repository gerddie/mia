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

#ifndef mia_core_fastica_hh
#define mia_core_fastica_hh

#include <memory>
#include <mia/core/gsl_matrix.hh>
#include <mia/core/fastica_nonlinearity.hh>
#include <mia/core/ica.hh>

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
	void set_approach(CIndepCompAnalysis::EApproach apr); 
	
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

	/// use this to make sure that for tests always the same
	/// random series is used
	void set_deterministic_seed(int seed); 
	
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

	CIndepCompAnalysis::EApproach m_approach; 
	
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

	int m_deterministic_seed; 
};

class  EXPORT_CORE CICAAnalysisMIA : public CIndepCompAnalysis {
public:
	typedef CIndepCompAnalysis::IndexSet IndexSet; 
	

	/**
	   Initialize an ICA based of predefined data - this is only used for test cases.
	*/
	CICAAnalysisMIA(const gsl::Matrix& ic, const gsl::Matrix& mix, const std::vector<double>& mean );
	

	CICAAnalysisMIA();
	
	
	~CICAAnalysisMIA();
	
	/**
	   Initialize the ICA
	   \param series_length number of data sets that will be provided
	   \param slice_size number of elements each set containes
	*/
	
	void initialize(unsigned int series_length, unsigned int slice_size) override;
	
	
	/**
	   Run the independed component analysis using the given numbers of components
	   \param nica number of indentepended components
	   \param guess initial guess for the ICA, pass an empty vector of you 
	   don't want to use this feature  
	*/
	bool run(unsigned int nica, std::vector<std::vector<float> > guess) override;
	

	/// \returns the feature vector of \a row
	std::vector<float> get_feature_row(unsigned int row) const override;
	
	/// \returns the mixing signal curve of the feature \a row
	std::vector<float> get_mix_series(unsigned int row) const override;
	
	/// \returns the complete mixed signal at series index \a idx
	std::vector<float> get_mix(unsigned int idx) const override;
	
	/** Evaluate an incomplete mixed signal. Here the features are given that are \a not to be used.
	    \sa get_partial_mix
	    \param idx series index
	    \param skip a set of feature indices that will be skipped when evaluating the mix
	    \returns the mixed signal
	*/
	std::vector<float> get_incomplete_mix(unsigned int idx, const IndexSet& skip)const override;

	/** Evaluate an incomplete mixed signal. Here the features are given that are \a used to create the mix.
	    \sa get_incomplete_mix
	    \param idx series index
	    \param use the set of feature indices that will be used to evaluate the mix
	    \returns an incolmplete mixed signal.
	*/
	std::vector<float> get_partial_mix(unsigned int idx, const IndexSet& use)const override;
	
	/** Evaluate a mix of the feature signals by adding and subtractig individual features.
	    \param plus features o be added
	    \param minus features to be subtracted
	    \returns the feature mix
	*/
	std::vector<float> get_delta_feature(const IndexSet& plus, const IndexSet& minus)const override;
	
	/**
	   Replace a mixing signal curve
	   \param index of the curve to be replaced
	   \param series new data for mixing curve
	 */
	void set_mixing_series(unsigned int index, const std::vector<float>& series) override;
	
	/// \returns a vector containing all mixing curves
	CSlopeColumns   get_mixing_curves() const override;
	
	/**
	   Normalize the ICs in the following manner:
	   * Scale and shift the range of the ICs to  [-1, 1]
	   * Scale the mixing curved to compensate for the required scaling
	   * move the means of the time points to compensate for the shifting.
	 */
	void normalize_ICs() override;

	/**
	   Normalize the mixing curves to have a zero mean. As a result a mean image is created that
	   containes the sum of the ICs weighted by the required mean shift.

	 */
	std::vector<float> normalize_Mix() override;


	/// \returns the number of actual ICs
	unsigned int get_ncomponents() const override;
	
	/**
	   sets the number of iterations in the ICA
	   \param n
	 */
	void set_max_iterations(int n) override;
	
	/**
	   Set the ICA approach to either FICA_APPROACH_DEFL(default) or FICA_APPROACH_SYMM. 
	   \param approach
	*/
	void set_approach(EApproach approach) override;

	void set_deterministic_seed(int seed) override;
private:
	void set_row_internal(unsigned row, const std::vector<double>&  buffer, double mean) override;
	
	struct CICAAnalysisMIAImpl *impl;

	int m_deterministic_seed; 
};


class EXPORT_CORE CICAAnalysisMIAFactory: public CIndepCompAnalysisFactory {
public:
	CIndepCompAnalysis *do_create() const override;
};


class EXPORT_CORE CICAAnalysisMIAFactoryPlugin: public  CIndepCompAnalysisFactoryPlugin {
public:
	CICAAnalysisMIAFactoryPlugin();

	CIndepCompAnalysisFactory *do_create() const override; 
	
	const std::string do_get_descr()const override; 
}; 


}

#endif 
