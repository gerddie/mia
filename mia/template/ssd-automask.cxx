/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/property_flags.hh>
#include <tbb/parallel_reduce.h>
#include <tbb/blocked_range.h>

#include <numeric>
#include <limits>

NS_BEGIN(NS)


struct SRA {
	size_t n; 
	double sum;
}; 

struct FEvalSSDAuto : public mia::TFilter<double> {
	FEvalSSDAuto(double src_mask_thresh, double ref_mask_thresh):
		m_src_mask_thresh(src_mask_thresh),
		m_ref_mask_thresh(ref_mask_thresh){
	}
	
	template <typename  T, typename  R>
	FEvalSSDAuto::result_type operator () (const T& a, const R& b) const {
		SRA result_accumulator = {0, 0.0}; 
		
		SRA  result = 
			tbb::parallel_reduce(tbb::blocked_range<size_t>(0, a.size()), result_accumulator, 
				     [this, &a, &b](const tbb::blocked_range<size_t>& range, SRA acc)->SRA {
					     for (auto ir = range.begin(); ir !=range.end(); ++ir){
						     double va = a[ir]; 
						     if (va >= m_src_mask_thresh) { 
							     double vb = b[ir];
							     if (vb >= m_ref_mask_thresh) {
								     ++acc.n; 
								     double d = va - vb; 
								     acc.sum += d * d; 
							     }
						     }
					     }
					     return acc; 
				     }, 
				     [](const SRA& lhs, const SRA& rhs) -> SRA {
					     SRA result;
					     result.n = lhs.n + rhs.n; 
					     result.sum = lhs.sum + rhs.sum; 
					     return result; 
				     }
			); 
		mia::cvdebug() << "sum=" << result.sum << ", n=" <<  result.n << "\n"; 
		return result.n > 0 ? 0.5 * result.sum / result.n : 0.0; 
	}
	double m_src_mask_thresh;
	double m_ref_mask_thresh; 

}; 


template <typename TCost> 
TSSDAutomaskCost<TCost>::TSSDAutomaskCost():
	m_src_mask_thresh(0.0), 
	m_ref_mask_thresh(0.0)
{
	this->add(::mia::property_gradient);
}

template <typename TCost> 
TSSDAutomaskCost<TCost>::TSSDAutomaskCost(double src_mask_thresh, double ref_thresh_mask):
	m_src_mask_thresh(src_mask_thresh), 
	m_ref_mask_thresh(ref_thresh_mask)
{
	this->add(::mia::property_gradient);
}

template <typename TCost> 
double TSSDAutomaskCost<TCost>::do_value(const Data& a, const Data& b) const
{
	FEvalSSDAuto essd(m_src_mask_thresh, m_ref_mask_thresh); 
	return mia::filter(essd, a, b); 
}

template <typename Force>
struct FEvalForceAuto: public mia::TFilter<float> {
	typedef typename Force::value_type force_type; 
	FEvalForceAuto(Force& force, double src_mask_thresh, double ref_mask_thresh):
		m_force(force), 
		m_src_mask_thresh(src_mask_thresh), 
		m_ref_mask_thresh(ref_mask_thresh)
		{
		}
	template <typename T, typename R> 
	float operator ()( const T& a, const R& b) const {
		Force gradient = get_gradient(a); 
		float cost = 0.0; 
		auto ai = a.begin();
		auto bi = b.begin();
		auto fi = m_force.begin(); 
		auto gi = gradient.begin(); 

		size_t n = 0;
		
		for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi, ++fi, ++gi) {
			if ((*ai >= m_src_mask_thresh) && (*bi >= m_ref_mask_thresh)) {
				float delta = float(*ai) - float(*bi); 
				*fi = *gi * delta ;
				cost += delta * delta; 
				++n; 
			}else 
				*fi = force_type(); 
		}
		double scale = 0.0; 
		if (n > 0) 
			scale = 1.0 / n; 
		
		transform(m_force.begin(), m_force.end(), m_force.begin(), 
			  [scale](const force_type& v){return scale * v;});  
		
		return 0.5 * scale * cost;
	}
private: 
	Force& m_force; 
	double m_src_mask_thresh;
	double m_ref_mask_thresh; 
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename TCost> 
double TSSDAutomaskCost<TCost>::do_evaluate_force(const Data& a, const Data& b, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForceAuto<Force> ef(force, m_src_mask_thresh, m_ref_mask_thresh); 
	return mia::filter(ef, a, b); 
}


/**
   This plugin will alwasy be "ssd" 
*/
template <typename CP, typename C> 
TSSDAutomaskCostPlugin<CP,C>::TSSDAutomaskCostPlugin():
	CP("ssd-automask")
{
	TRACE("TSSDAutomaskCostPlugin<CP,C>::TSSDAutomaskCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
	const double limits = std::numeric_limits<double>::max(); 
	this->add_parameter("rthresh", new mia::CDoubleParameter(m_ref_mask_thresh, -limits, limits, false, 
								 "Threshold intensity value for reference image")); 

	this->add_parameter("sthresh", new mia::CDoubleParameter(m_src_mask_thresh, -limits, limits, false, 
								    "Threshold intensity value for source image")); 

}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
C *TSSDAutomaskCostPlugin<CP,C>::do_create() const
{
	return new TSSDAutomaskCost<C>(m_src_mask_thresh, m_ref_mask_thresh);
}

/// @endcond 
NS_END
