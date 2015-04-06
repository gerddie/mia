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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>


NS_BEGIN(NS)

template <typename Mask>
struct FEvalSSD : public mia::TFilter<double> {
	FEvalSSD(const Mask& m):m_mask(m)
		{}
	
	template <typename T, typename R>
	struct SQD {
		double operator ()(T a, R b) const {
			double d = (double)a - (double)b; 
			return d * d;
		}
	}; 
	
	template <typename  T, typename  R>
	FEvalSSD::result_type operator () (const T& a, const R& b) const {
		auto ia = a.begin(); 
		auto ib = b.begin(); 
		auto ie = a.end(); 
		auto im = m_mask.begin(); 
		const long n = a.size(); 
		double sum = 0.0; 
		while (ia != ie) {
			if (*im) {
				double d = (double)*ia - (double)*ib; 
				sum += d*d; 
			}
			++ia; ++ib; ++im; 
		}
		// If masks don't overlap the cost is zero, this is bad!!
		return n > 0 ?  0.5 * sum / n : 0.0;
	}
private: 
	const Mask& m_mask; 
}; 


template <typename TCost> 
TSSDMaskedImageCost<TCost>::TSSDMaskedImageCost()
{
	this->add(::mia::property_gradient);
}

template <typename TCost> 
double TSSDMaskedImageCost<TCost>::do_value(const Data& a, const Data& b, const Mask& m) const
{
	FEvalSSD<Mask> essd(m); 
	return filter(essd, a, b); 
}

template <typename Force, typename Mask>
struct FEvalForce: public mia::TFilter<float> {
	FEvalForce(Force& force, const Mask& m):
		m_force(force), 
		m_mask(m)
		{
		}
	template <typename T, typename R> 
	float operator ()( const T& a, const R& b) const {
		Force gradient = get_gradient(a); 
		float cost = 0.0; 
		auto ai = a.begin();
		auto bi = b.begin();
		auto fi = m_force.begin(); 
		auto im = m_mask.begin(); 
		auto gi = gradient.begin(); 

		long n = a.size();
		for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi, ++fi, ++gi, ++im) {
			if (*im) {
				float delta = float(*ai) - float(*bi); 
				*fi = *gi * delta;
				cost += delta * delta; 
			}
		}
		if ( n > 0) {
			float  scale = 1.0f / n; 
			transform(m_force.begin(), m_force.end(), m_force.begin(),
				  [scale](const typename Force::value_type&  x){return scale * x;}); 
			return 0.5 * cost  *scale;
		}else{
			return 0.0; 
		}
	}
private: 
	Force& m_force; 
	const Mask& m_mask;
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename TCost> 
double TSSDMaskedImageCost<TCost>::do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce<Force, Mask> ef(force, m); 
	return filter(ef, a, b); 
}



/**
   This plugin will alwasy be "ssd" 
*/
template <typename CP, typename C> 
TSSDMaskedImageCostPlugin<CP,C>::TSSDMaskedImageCostPlugin():
	CP("ssd")
{
	TRACE("TSSDCostPlugin<CP,C>::TSSDCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
C *TSSDMaskedImageCostPlugin<CP,C>::do_create() const
{
	return new TSSDMaskedImageCost<C>();
}

template <typename CP, typename C> 
const std::string TSSDMaskedImageCostPlugin<CP,C>::do_get_descr() const
{
	return "Sum of squared differences with masking.";  
	
}


NS_END
