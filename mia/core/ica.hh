/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
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


#ifndef mia_core_ica_hh
#define mia_core_ica_hh

#include <set>
#include <vector>
#include <mia/core/defines.hh>
#include <mia/core/slopeclassifier.hh>
#include <itpp/signal/fastica.h>
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>


NS_MIA_BEGIN

/**
   \Brief class for ICA analysis and use of such data.

   This class implements basic operations for of ICA. It makes use of the ITPP implementation of FastICA.

*/

class  EXPORT_CORE CICAAnalysis {
public:

	typedef itpp::Vec<itpp::mat::value_type> itppvector;
	/**
	   Initialize an ICA based of predefined data - this is unly used for test cases.
	 */
	CICAAnalysis(const itpp::mat& ic, const itpp::mat& mix, const std::vector<float>& mean );

	/**
	   Main constructor of the ICA, i.e. you want to use this.
	   \param series_length number of data sets that will be provided
	   \param slice_size number of elements each set containes
	 */
	CICAAnalysis(size_t series_length, size_t slice_size);


	~CICAAnalysis();


	/// defines a set of indices used for mixing
	typedef std::set<size_t> IndexSet;

	/**
	   Set on row of input data
	   \tparam Iterator input data iterator, must follow the model of a forward iterator
	   \param row index of the input slice
	   \param begin start iterator of input data
	   \param end
	 */
	template <class Iterator>
	BOOST_CONCEPT_REQUIRES(((::boost::ForwardIterator<Iterator>)),
			       (void))
		set_row(size_t row, Iterator begin, Iterator end);


	/**
	   Run the independed component analysis using the given numbers of components
	   \param nica number of indentepended components
	 */
	void run(size_t nica);

        /**
	   Run the independed component analysis with an estimation of the optimal number
	   of components.
	   \param max_ica maximum number of independend components
	   \param min_ica minimum number of independend components
	   \param corr_thresh minimum absolute correation of the mixing signals to joins two components
	*/
	void run_auto(int max_ica, int min_ica, float corr_thresh=0.9);


	/// \returns the feature vector of \a row
	std::vector<float> get_feature_row(size_t row)const;

	/// \returns the mixing signal curve of the feature \a row
	std::vector<float> get_mix_series(size_t row)const;

	/// \returns the complete mixed signal at series index \a idx
	std::vector<float> get_mix(size_t idx)const;

	/** Evaluate an incomplete mixed signal. Here the features are given that are \a not to be used.
	    \sa get_partial_mix
	    \param idx series index
	    \param skip a set of feature indices that will be skipped when evaluating the mix
	    \returns the mixed signal
	*/
	std::vector<float> get_incomplete_mix(size_t idx, const IndexSet& skip)const;

	/** Evaluate an incomplete mixed signal. Here the features are given that are \a used to create the mix.
	    \sa get_incomplete_mix
	    \param idx series index
	    \param use the set of feature indices that will be used to evaluate the mix
	    \returns an incolmplete mixed signal.
	*/
	std::vector<float> get_partial_mix(size_t idx, const IndexSet& use)const;

	/** Evaluate a mix of the feature signals by adding and subtractig individual features.
	    \param plus features o be added
	    \param minus features to be subtracted
	    \returns the feature mix
	 */
	std::vector<float> get_delta_feature(const IndexSet& plus, const IndexSet& minus)const;

	/**
	   Replace a mixing signal curve
	   \param index of the curve to be replaced
	   \param new data for mixing curve
	 */
	void set_mixing_series(size_t index, const std::vector<float>& series);

	/// \returns a vector containing all mixing curves
	CSlopeClassifier::Columns get_mixing_curves() const;

	/**
	   Normalize the ICs in the following manner:
	   * Scale and shift the range of the ICs to  [-1, 1]
	   * Scale the mixing curved to compensate for the required scaling
	   * move the means of the time points to compensate for the shifting.
	 */
	void normalize_ICs();

	/**
	   Normalize the mixing curves to have a zero mean. As a result a mean image is created that
	   containes the sum of the ICs weighted by the required mean shift.

	 */
	std::vector<float> normalize_Mix();


	/// \returns the number of actual ICs
	size_t get_ncomponents() const;

	/**
	   sets the number of iterations in the ICA
	   \param n
	 */
	void set_max_iterations(int n);
private:
	void set_row(int row, const itppvector&  buffer, double mean);

	struct CICAAnalysisImpl *impl;

};


template <class Iterator>
BOOST_CONCEPT_REQUIRES(((::boost::ForwardIterator<Iterator>)),
		       (void))
CICAAnalysis::set_row(size_t row, Iterator begin, Iterator end)
{
	const size_t length = std::distance(begin, end);
	itppvector buffer(length);
	size_t idx = 0;
	double mean = 0.0;

	while (begin != end)
		mean += (buffer[idx++] = *begin++);
	mean /= length;
	for(size_t i = 0; i < length; ++i)
		buffer[i] -= mean;
	set_row(row, buffer, mean);
}

NS_MIA_END

#endif
