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


#ifndef mia_core_ICAANALYSISBASE_HH
#define mia_core_ICAANALYSISBASE_HH

#include <memory>
#include <set>
#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>

#include <mia/core/defines.hh>
#include <mia/core/slopevector.hh>
#include <mia/core/factory.hh>

namespace mia
{

class EXPORT_CORE CIndepCompAnalysis
{
public:

       /**
          Separation approach to be used.
       */
       enum EApproach {
              appr_defl, /**< Deflation approach - each component is extimated separately */
              appr_symm,  /**< Symmetric approach thet estimates all components at the same time */
              appr_unknown
       };

       typedef std::unique_ptr<CIndepCompAnalysis> Pointer;

       /// defines a set of indices used for mixing
       typedef std::set<unsigned int> IndexSet;

       virtual ~CIndepCompAnalysis();

       /**
          Set on row of input data
          \tparam Iterator input data iterator, must follow the model of a forward iterator
          \param row index of the input slice
          \param begin start iterator of input data
          \param end end iterator of input data
       */
       template <class Iterator>
       BOOST_CONCEPT_REQUIRES(((::boost::ForwardIterator<Iterator>)),
                              (void))
       set_row(unsigned row, Iterator begin, Iterator end);


       virtual void initialize(unsigned int series_length, unsigned int slice_size) = 0;
       /**
          Run the independed component analysis using the given numbers of components
          \param nica number of indentepended components
          \param guess initial guess for the ICA, pass an empty vector of you
          don't want to use this feature
       */
       virtual bool run(unsigned int nica, std::vector<std::vector<float>> guess) = 0;

       /// \returns the feature vector of \a row
       virtual std::vector<float> get_feature_row(unsigned int row)const = 0;

       /// \returns the mixing signal curve of the feature \a row
       virtual std::vector<float> get_mix_series(unsigned int row)const = 0;

       /// \returns the complete mixed signal at series index \a idx
       virtual std::vector<float> get_mix(unsigned int idx)const = 0;

       /** Evaluate an incomplete mixed signal. Here the features are given that are \a not to be used.
           \sa get_partial_mix
           \param idx series index
           \param skip a set of feature indices that will be skipped when evaluating the mix
           \returns the mixed signal
       */
       virtual std::vector<float> get_incomplete_mix(unsigned int idx, const IndexSet& skip)const = 0;

       /** Evaluate an incomplete mixed signal. Here the features are given that are \a used to create the mix.
           \sa get_incomplete_mix
           \param idx series index
           \param use the set of feature indices that will be used to evaluate the mix
           \returns an incolmplete mixed signal.
       */
       virtual std::vector<float> get_partial_mix(unsigned int idx, const IndexSet& use)const = 0;

       /** Evaluate a mix of the feature signals by adding and subtractig individual features.
           \param plus features o be added
           \param minus features to be subtracted
           \returns the feature mix
       */
       virtual std::vector<float> get_delta_feature(const IndexSet& plus, const IndexSet& minus)const = 0;

       /**
          Replace a mixing signal curve
          \param index of the curve to be replaced
          \param series new data for mixing curve
       */
       virtual void set_mixing_series(unsigned int index, const std::vector<float>& series) = 0;

       /// \returns a vector containing all mixing curves
       virtual CSlopeColumns   get_mixing_curves() const = 0;

       /**
          Normalize the ICs in the following manner:
          * Scale and shift the range of the ICs to  [-1, 1]
          * Scale the mixing curved to compensate for the required scaling
          * move the means of the time points to compensate for the shifting.
          */
       virtual void normalize_ICs() = 0;

       /**
          Normalize the mixing curves to have a zero mean. As a result a mean image is created that
          containes the sum of the ICs weighted by the required mean shift.

       */
       virtual std::vector<float> normalize_Mix() = 0;


       /// \returns the number of actual ICs
       virtual unsigned int get_ncomponents() const = 0;

       /**
          sets the number of iterations in the ICA
          \param n
       */
       virtual void set_max_iterations(int n) = 0;

       /**
          Set the ICA approach to either FICA_APPROACH_DEFL(default) or FICA_APPROACH_SYMM.
          \param approach
       */
       virtual void set_approach(EApproach approach) = 0;

       virtual void set_deterministic_seed(int seed)  = 0;
private:
       virtual void set_row_internal(unsigned row, const std::vector<double>&  buffer, double mean) = 0;

};

typedef CIndepCompAnalysis::Pointer PIndepCompAnalysis;


class EXPORT_CORE CIndepCompAnalysisFactory : public CProductBase
{
public:
       static const char *data_descr;
       static const char *type_descr;

       typedef CIndepCompAnalysisFactory plugin_type;
       typedef CIndepCompAnalysisFactory plugin_data;

       CIndepCompAnalysisFactory();
       virtual ~CIndepCompAnalysisFactory();
       void set_deterministic_seed(int seed);
       CIndepCompAnalysis *create() const __attribute__((warn_unused_result));
private:
       virtual CIndepCompAnalysis *do_create() const __attribute__((warn_unused_result)) = 0;
       int m_deterministic_seed;
};


typedef TFactory<CIndepCompAnalysisFactory> CIndepCompAnalysisFactoryPlugin;

typedef std::shared_ptr<CIndepCompAnalysisFactory> PIndepCompAnalysisFactory;

/// plugin handler for spaciel filter kernels
typedef THandlerSingleton<TFactoryPluginHandler<CIndepCompAnalysisFactoryPlugin>>
              CIndepCompAnalysisFactoryPluginHandler;


template<>  const char *const
TPluginHandler<TFactory<CIndepCompAnalysisFactory>>::m_help;

extern template class EXPORT_CORE  TPluginHandler<TFactory<CIndepCompAnalysisFactory>>;

/// @cond NEVER
FACTORY_TRAIT(CIndepCompAnalysisFactoryPluginHandler);
/// @endcond



inline PIndepCompAnalysisFactory produce_ica_factory(const std::string& descr)
{
       return CIndepCompAnalysisFactoryPluginHandler::instance().produce(descr);
}



/// \cond DOXYGEN_DOESNT_UNDERSTAND_BOOST_CONCEPT_REQUIRES
template <class Iterator>
BOOST_CONCEPT_REQUIRES(((::boost::ForwardIterator<Iterator>)),
                       (void))
CIndepCompAnalysis::set_row(unsigned row, Iterator begin, Iterator end)
{
       const unsigned int length = std::distance(begin, end);
       std::vector<double> buffer(length);
       unsigned int idx = 0;
       double mean = 0.0;

       while (begin != end)
              mean += (buffer[idx++] = *begin++);

       mean /= length;

       for (unsigned int i = 0; i < length; ++i)
              buffer[i] -= mean;

       set_row_internal(row, buffer, mean);
}
/// \endcond

}

#endif // CICAANALYSISBASE_HH
