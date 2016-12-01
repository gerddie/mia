/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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

#ifndef addons_itpp_ica_hh
#define addons_itpp_ica_hh

#include <set>
#include <vector>
#include <itpp/signal/fastica.h>

#include <mia/core/defines.hh>
#include <mia/core/slopevector.hh>
#include <mia/core/ica.hh>


namespace itpp_fastica {

/**
   \ingroup perf
   \brief class for ICA analysis and use of such data.
   This class implements basic operations for of ICA. It makes use of the ITPP implementation of FastICA.
*/

class  EXPORT_CORE CICAAnalysisITPP : public mia::CIndepCompAnalysis {
public:
	typedef mia::CIndepCompAnalysis::IndexSet IndexSet; 
	

	/**
	   Initialize an ICA based of predefined data - this is only used for test cases.
	*/
	CICAAnalysisITPP(const itpp::mat& ic, const itpp::mat& mix, const std::vector<double>& mean );
	

	CICAAnalysisITPP();
	
	
	~CICAAnalysisITPP();
	
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
	std::vector<float> get_feature_row(unsigned int row)const override;
	
	/// \returns the mixing signal curve of the feature \a row
	std::vector<float> get_mix_series(unsigned int row)const override;
	
	/// \returns the complete mixed signal at series index \a idx
	std::vector<float> get_mix(unsigned int idx)const override;
	
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
	mia::CSlopeColumns   get_mixing_curves() const override;
	
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
	void set_row_internal(unsigned row, const std::vector<double>&  buffer, double mean);
	
	struct CICAAnalysisITPPImpl *impl;
	
};


class EXPORT_CORE CICAAnalysisITPPFactory: public mia::CIndepCompAnalysisFactory {
public:
	mia::CIndepCompAnalysis *do_create() const override;
};

class EXPORT_CORE CICAAnalysisITPPFactoryPlugin: public  mia::CIndepCompAnalysisFactoryPlugin {
public:
	CICAAnalysisITPPFactoryPlugin();

	mia::CIndepCompAnalysisFactory *do_create() const override; 
	
	const std::string do_get_descr()const override; 
}; 


}

#endif
