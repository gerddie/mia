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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)

///  @cond DOC_PLUGINS 

/**

   @ingroup registation 
*/
template <typename TCost> 
class TSSDCost: public TCost {
public: 	
	typedef typename TCost::Data Data; 
	typedef typename TCost::Force Force; 

	TSSDCost(); 
	TSSDCost(bool normalize, float automask_thresh); 
private: 
	virtual double do_value(const Data& a, const Data& b) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, Force& force) const; 
	bool m_normalize; 
	float m_automask_thresh; 
};


struct FEvalSSD : public mia::TFilter<double> {
	FEvalSSD(bool normalize, float automask_thresh):
		m_normalize(normalize), m_automask_thresh(automask_thresh){}
	
	template <typename T, typename R>
	struct SQD {
		double operator ()(T a, R b) const {
			double d = (double)a - (double)b; 
			return d * d;
		}
	}; 
	
	template <typename  T, typename  R>
	FEvalSSD::result_type operator () (const T& a, const R& b) const {
		double scale = m_normalize ? 0.5 / a.size() : 0.5; 
		if (m_automask_thresh == 0.0f) 
			return scale * inner_product(a.begin(), a.end(), b.begin(), 0.0,  ::std::plus<double>(), 
						     SQD<typename T::value_type , typename R::value_type >()); 
		else {
			auto ia = a.begin(); 
			auto ib = b.begin(); 
			auto ie = a.end(); 
			long n = 0; 
			double sum = 0.0; 
			while (ia != ie) {
				if (*ia > m_automask_thresh) {
					double d = (double)*ia - (double)*ib; 
					sum += d*d; 
					++n; 
				}
				++ia; ++ib; 
			}
			// high penalty if the mask don't overlap at all 
			return n > 0 ?  0.5 * sum / n : std::numeric_limits<float>::max(); 
		}
	}
	bool m_normalize; 
	float m_automask_thresh; 
}; 


template <typename TCost> 
TSSDCost<TCost>::TSSDCost():
	m_normalize(true), 
	m_automask_thresh(0.0)
{
	this->add(::mia::property_gradient);
}

template <typename TCost> 
TSSDCost<TCost>::TSSDCost(bool normalize, float automask_thresh):
        m_normalize(normalize), 
        m_automask_thresh(automask_thresh)
{
	this->add(::mia::property_gradient);
}

template <typename TCost> 
double TSSDCost<TCost>::do_value(const Data& a, const Data& b) const
{
	FEvalSSD essd(m_normalize, m_automask_thresh); 
	return filter(essd, a, b); 
}

template <typename Force>
struct FEvalForce: public mia::TFilter<float> {
	FEvalForce(Force& force, bool normalize, float automask_thresh):
		m_force(force), 
		m_normalize(normalize),
		m_automask_thresh(automask_thresh)
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

		if (m_automask_thresh == 0.0f) {
			float scale = m_normalize ? 1.0 / a.size() : 1.0; 
			
			for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi, ++fi, ++gi) {
				float delta = float(*ai) - float(*bi); 
				*fi = *gi * delta  * scale;
				cost += delta * delta * scale; 
			}
			return 0.5 * cost; 
		}else{
			long n = 0;
			for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi, ++fi, ++gi) {
				if (*ai > m_automask_thresh) {
					float delta = float(*ai) - float(*bi); 
					*fi = *gi * delta;
					cost += delta * delta; 
					++n; 
				}
			}
			if ( n > 0) {
				float  scale = 1.0f / n; 
				transform(m_force.begin(), m_force.end(), m_force.begin(),
					  [scale](const typename Force::value_type&  x){return scale * x;}); 
				return 0.5 * cost  *scale;
			}else{
				return std::numeric_limits<float>::max(); 
			}
		}
	}
private: 
	Force& m_force; 
	bool m_normalize; 
	float m_automask_thresh; 
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename TCost> 
double TSSDCost<TCost>::do_evaluate_force(const Data& a, const Data& b, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce<Force> ef(force, m_normalize, m_automask_thresh); 
	return filter(ef, a, b); 
}


/**
   This is the plug-in declaration - the actual plugin needs to define the 
   cost plugin type and the data type (this could be unified) 
   do_test and do_get_descr need to be implemented 
*/
template <typename CP, typename C> 
class TSSDCostPlugin: public CP {
public: 
	TSSDCostPlugin();
	C *do_create()const;
private:
	bool m_normalize; 
	float m_automask_thresh; 
};


/**
   This plugin will alwasy be "ssd" 
*/
template <typename CP, typename C> 
TSSDCostPlugin<CP,C>::TSSDCostPlugin():
	CP("ssd"), 
	m_normalize(false), 
	m_automask_thresh(0)
{
	TRACE("TSSDCostPlugin<CP,C>::TSSDCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
	this->add_parameter("norm", new mia::CBoolParameter(m_normalize, false, 
			    "Set whether the metric should be normalized by the number of image pixels")
		); 
	this->add_parameter("autothresh", mia::make_ci_param(m_automask_thresh, 0.0f, 1000.0f, false, 
							     "Use automatic masking of the moving image by only takeing "
							     "intensity values into accound that are larger than the given threshold"));  

}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
C *TSSDCostPlugin<CP,C>::do_create() const
{
	return new TSSDCost<C>(m_normalize, m_automask_thresh);
}

/// @endcond 
NS_END
