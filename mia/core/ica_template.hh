/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2011 Gert Wollny
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef mia_core_ica_template_hh
#define mia_core_ica_template_hh

#include <mia/core/ica.hh>
#include <vector>

#ifndef EXPORT_TDataSeriesICA
#ifdef WIN32
#  define EXPORT_TDataSeriesICA __declspec(dllimport)
#else
# ifdef __GNUC__
#   define EXPORT_TDataSeriesICA __attribute__((visibility("default")))
# else
#   define EXPORT_TDataSeriesICA
# endif
#endif
#endif 

NS_MIA_BEGIN

/**
   \ingroup classes

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
class  EXPORT_TDataSeriesICA TDataSeriesICA {
public:
	/** a set of indices used for addressing a subset of the independend componsts 
	 */
	typedef CICAAnalysis::IndexSet IndexSet;

	/** a (shared) pointer to itself */
	typedef typename Data::Pointer PData; 

	/**
	   \brief ICA initialization 
	   
	   The contructor for an ICA
	   \param initializer data set containing all the time steps of input data 
	   \param strip_mean strip the mean from the series before processing 
	 */
	TDataSeriesICA(const std::vector<Data>& initializer, bool strip_mean);
	
	/**  Runs the ICA 
	     \param ncomponents retained components 
	     \param strip_mean strip the mean from the input 
	     \param ica_normalize normalize the ICA after processing 
	     \param guess provide an initial guess (leave empty, if not wanted)  
	*/
	bool run(size_t ncomponents, bool strip_mean, bool ica_normalize, 
		 std::vector<std::vector<float> >  guess = std::vector<std::vector<float> >());


        /**
	   Run the independed component analysis with an estimation of the optimal number
	   of components based on mixing curve correlation (experimental) 
	   \param max_ica maximum number of independend components
	   \param min_ica minimum number of independend components
	   \param corr_thresh minimum absolute correation of the mixing signals to joins two components
	   \returns number of obtained independend components
	*/
	size_t run_auto(int max_ica, int min_ica, float corr_thresh); 
	
	/** Normalizes the ICs to the range of [-1,1] and correct the mixing matrix accordingly. 
	    This operation does not change the output of a mix. 
	 */
	void normalize();
	
	/// Normalizes the Mixing Matrix columns to have zero mean. 
	void normalize_Mix();
	

	/** 
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
	   \param comps set of components to use when mixing
	   \returns the mixed data
	 */
	Data get_partial_mix(size_t idx, const IndexSet& comps) const;
	
	/// \returns the mixing curves as vector of vectors
	CSlopeColumns get_mixing_curves() const;

	/// \returns the feature data relating to component \a idx
	PData get_feature_image(size_t idx) const; 

	/**
	   Evaluate a mix as sum and difference of ICs
	   \param plus add these ICs 
	   \param minus subtract these ICs 
	   \returns the combined signal 
	 */
	PData get_delta_feature(const IndexSet& plus, const IndexSet& minus)const; 
	
	/**
	   replace a mixing series by a new set
	   \param index component number of the mixing curve to be replaced
	   \param series new mixing data 
	 */
	void set_mixing_series(size_t index, const std::vector<float>& series);


	/**
	   sets the number of iterations in the ICA 
	   \param n 
	 */
	void set_max_iterations(int n); 

	/**
	   Set the ICA approach to either FICA_APPROACH_DEFL(default) or FICA_APPROACH_SYMM. 
	   \param approach
	 */
	void set_approach(int approach); 
	

	~TDataSeriesICA();
private:
	CICAAnalysis m_analysis;
	typedef typename Data::dimsize_type dimsize_type; 
	dimsize_type m_size;
	Data m_mean;

};

NS_MIA_END

#endif
