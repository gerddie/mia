/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2015 Gert Wollny
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

#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <limits>
#include <numeric>
#include <mia/core/ica_template.hh>
#include <mia/core/msgstream.hh>

NS_MIA_BEGIN

template <class Data>
TDataSeriesICA<Data>::TDataSeriesICA(const CICAAnalysisFactory& icatool, const std::vector<Data>& initializer,
                                     bool strip_mean):
        m_analysis(icatool.create())
{
        TRACE_FUNCTION;

        if (initializer.empty())
                throw std::invalid_argument("TDataSeriesICA: empty series not supported");

        m_size =initializer[0].get_size();

        m_analysis->initialize(initializer.size(), initializer[0].size());

        m_mean = Data(m_size);

        if (strip_mean) {
                for(size_t i = 0; i < initializer.size(); ++i) {
                        std::transform(initializer[i].begin(), initializer[i].end(),
                                       m_mean.begin(), m_mean.begin(),
                                       [](float x, float y){return x+y;});
                }
                float scale = 1.0f / initializer.size();
                std::transform(m_mean.begin(), m_mean.end(), m_mean.begin(),
                               [&scale](float x){return x * scale;});

                std::vector<double> help(initializer[0].size());
                for(size_t i = 0; i < initializer.size(); ++i) {
                        std::transform(initializer[i].begin(), initializer[i].end(),
                                       m_mean.begin(), help.begin(),
                                       [](float x, float y){return  x - y;});
                        m_analysis->set_row(i, help.begin(), help.end());
                }
        }else
                for(size_t i = 0; i < initializer.size(); ++i)
                        m_analysis->set_row(i, initializer[i].begin(), initializer[i].end());

}


template <class Data> 
TDataSeriesICA<Data>::~TDataSeriesICA()
{
}

template <class Data> 
bool TDataSeriesICA<Data>::run(size_t ncomponents, bool strip_mean, bool ica_normalize, 
			       std::vector<std::vector<float> >  guess )
{
	TRACE_FUNCTION; 
        bool result = m_analysis->run(ncomponents, guess);
	if (result) {
		if (strip_mean)
			this->normalize_Mix();
		if (ica_normalize) 
			this->normalize();
	}
	return result; 
}

template <class Data> 
const Data& TDataSeriesICA<Data>::get_mean_image() const
{
	return m_mean; 
}

template <class Data> 
Data TDataSeriesICA<Data>::get_mix(size_t idx) const
{
	TRACE_FUNCTION; 
        std::vector<float> mix = m_analysis->get_mix(idx);
	Data result(m_size);
	assert( result.size() == mix.size());
	std::transform(mix.begin(), mix.end(), m_mean.begin(), result.begin(), 
		       [](float x, float y){return x+y;});
	return result;
}

template <class Data> 
Data TDataSeriesICA<Data>::get_incomplete_mix(size_t idx, const IndexSet& skip) const
{
	TRACE_FUNCTION; 
        std::vector<float> mix = m_analysis->get_incomplete_mix(idx, skip);
	Data result(m_size);
	assert( result.size() == mix.size());
	std::transform(mix.begin(), mix.end(), m_mean.begin(), result.begin(), 
		       [](float x, float y){return x+y;});
	return result;
}

template <class Data> 
Data TDataSeriesICA<Data>::get_partial_mix(size_t idx, const IndexSet& comps) const
{
	TRACE_FUNCTION; 
        std::vector<float> mix = m_analysis->get_partial_mix(idx, comps);
	Data result(m_size);
	assert( result.size() == mix.size());
	std::transform(mix.begin(), mix.end(), m_mean.begin(), result.begin(), 
		       [](float x, float y){return x+y;});
	return result;
}

template <class Data> 
CSlopeColumns TDataSeriesICA<Data>::get_mixing_curves() const
{
	TRACE_FUNCTION; 
        return m_analysis->get_mixing_curves();
}

template <class Data> 
typename TDataSeriesICA<Data>::PData TDataSeriesICA<Data>::get_feature_image(size_t idx) const 
{
	Data *result = new Data(m_size); 
	PData presult(result); 
        const std::vector<float> feature = m_analysis->get_feature_row(idx);
	std::copy(feature.begin(), feature.end(), result->begin()); 
	return presult; 
}

template <class Data> 
typename TDataSeriesICA<Data>::PData TDataSeriesICA<Data>::get_delta_feature(const IndexSet& plus, const IndexSet& minus)const
{
	Data *result = new Data(m_size); 
	PData presult(result); 
        const std::vector<float> feature = m_analysis->get_delta_feature(plus, minus);
	std::copy(feature.begin(), feature.end(), result->begin()); 
	return presult; 
}

template <class Data> 
void TDataSeriesICA<Data>::set_mixing_series(size_t index, const std::vector<float>& filtered_series)
{
        m_analysis->set_mixing_series(index, filtered_series);
}

template <class Data> 
void TDataSeriesICA<Data>::normalize()
{
        m_analysis->normalize_ICs();
}

template <class Data> 
void  TDataSeriesICA<Data>::normalize_Mix()
{
        std::vector<float> mean = m_analysis->normalize_Mix();
	transform(m_mean.begin(), m_mean.end(), mean.begin(), m_mean.begin(), 
		  [](float x, float y){return x+y;}); 
}

template <class Data> 
size_t TDataSeriesICA<Data>::run_auto(int nica, int min_ica, float corr_thresh)
{
        m_analysis->run_auto(nica, min_ica, corr_thresh);
        return m_analysis->get_ncomponents();
}


template <class Data> 
void TDataSeriesICA<Data>::set_max_iterations(int n)
{
        m_analysis->set_max_iterations(n);
}

template <class Data> 
void TDataSeriesICA<Data>::set_approach(CICAAnalysis::EApproach approach)
{
        m_analysis->set_approach(approach);
}

NS_MIA_END
