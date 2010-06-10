/* -*- mia-c++  -*-
 *
 * Copyright (c) Madrid 2009 - 2010
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

#ifndef mia_core_ica_template_hh
#define mia_core_ica_template_hh

#include <mia/core/ica.hh>
#include <vector>

NS_MIA_BEGIN

/**
   \brief Templated representation of a ICA series analyis 
   This class provides a generic implementation for the ICA analysis 
   of data series. 
   \tparam Data some kind of data set ofwhich series are analyzed 
   The class \a Data must somehow provide:
   - a typedef \a Pointer  for a pointer like representation of itself 
     or on of its base classes (preferable based on boost::shared_ptr)
   - a typedef \a dimsize_type for its the size seperated into dimensions 
   - a method \a get_size() that returns \a dimsize_type 
   - a method \a size() that returns the number of elements hold by itself
   - STL comaptible const and non-const iterators returned by 
       functions named \a begin() and \a end() 
*/

template <class Data> 
class TDataSeriesICA {
public:
	/** a set of indices used for addressing a subset of the independend componsts 
	 */
	typedef CICAAnalysis::IndexSet IndexSet;

	/** a (shared) pointer to itself */
	typedef typename Data::Pointer PData; 

	/** \addgroup Contructors */
	/*\@{  
	  \brief ICA initialization 
	  The contructor for an ICA
	  \param initializer data set containing all the time steps of input data 
	  \param strip_mean strip the mean from the series before processing 
	 */
	TDataSeriesICA(const std::vector<Data>& initializer, bool strip_mean);
	/* \@}*/

	/** \addgroup ICA application */
	/*\@{  
	  Runs the ICA 
	  \param ncomponents retained components 
	*/
	void run(size_t ncomponents);


	size_t run_auto(int nica, int min_ica, float corr_thresh); 
	
	/** Normalizes the ICs to the range of [-1,1] and correct the mixing matrix accordingly. 
	    This operation does not change the output of a mix. 
	 */
	void normalize();
	
	/// Normalizes the Mixing Matrix columns to have zero mean. 
	void normalize_Mix();
	/* \@}*/


	/** \addgroup Getters */
	/*\@{  
	  \returns the mixed signal at index \a idx
	 */
	Data get_mix(size_t idx) const;

	/// \returns the mean of the input data 
	const Data&  get_mean_image() const;

	/**
	   Evaluate a partial mix of the ICs 
	   \param idx time indes 
	   \param skip set of components to skip when mixing
	   \returns the mixed data
	 */
	Data get_incomplete_mix(size_t idx, const IndexSet& skip) const;

	/**
	   Evaluate a partial mix of the ICs by using the given indes set 
	   \param idx time indes 
	   \param skip set of components to use when mixing
	   \returns the mixed data
	 */
	Data get_partial_mix(size_t idx, const IndexSet& comps) const;
	
	/// \returns the mixing curves as vector of vectors
	CSlopeClassifier::Columns get_mixing_curves() const;

	/// \returns the feature data relating to component \a idx
	PData get_feature_image(size_t idx) const; 

	/**
	   Evaluate a mix as sum and difference of ICs
	   \param 
	   \@}
	 */
	
	PData get_delta_feature(const IndexSet& plus, const IndexSet& minus)const; 
	
	/**
	   replace a mixing series by a new set
	   \param index component number of the mixing curve to be replaced
	   \param series new mixing data 
	 */
	void set_mixing_series(size_t index, const std::vector<float>& series);
	

	~TDataSeriesICA();
private:
	CICAAnalysis m_analysis;
	typedef typename Data::dimsize_type dimsize_type; 
	dimsize_type m_size;
	Data m_mean;

};

NS_MIA_END

#endif
