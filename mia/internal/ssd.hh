/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2010
 * Max-Planck-Institute for Evolutionary Anthropoloy
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

/*! \brief basic type of a plugin handler 

A generic SSD implementation for plugins

\author Gert Wollny <gw.fossdev@gmail.com>

*/


#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)
/**
   This is the generic const function that gets derived from the 
   cost function type of the data 
*/

template <typename TCost> 
class CSSDCost: public TCost {
public: 	
	typedef typename TCost::Data Data; 
	typedef typename TCost::Force Force; 

	CSSDCost(); 
private: 
	virtual double do_value(const Data& a, const Data& b) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, float scale, Force& force) const; 
};


/**
   this is a filter to measure the actual cost
*/
struct FEvalSSD : public mia::TFilter<double> {
	
	template <typename T, typename R>
	struct SQD {
		double operator ()(T a, R b) const {
			double d = (double)a - (double)b; 
			return d * d;
		}
	}; 
	
	template <typename  T, typename  R>
	FEvalSSD::result_type operator () (const T& a, const R& b) const {
		return 0.5 * inner_product(a.begin(), a.end(), b.begin(), 0.0,  ::std::plus<double>(), 
					   SQD<typename T::value_type , typename R::value_type >()); 
	}
}; 


/**
   This is the implementation of the cost function
*/
template <typename TCost> 
CSSDCost<TCost>::CSSDCost()
{
	this->add(::mia::property_gradient); 	
}

template <typename TCost> 
double CSSDCost<TCost>::do_value(const Data& a, const Data& b) const
{
	FEvalSSD essd; 
	return filter(essd, a, b); 
}

/**
   This is a force evaluation filter 
*/

template <typename Force>
struct FEvalForce: public mia::TFilter<float> {
	FEvalForce(Force& force, float scale):
		_M_force(force), 
		_M_scale(scale)
		{
		}
	template <typename T, typename R> 
	float operator ()( const T& a, const R& b) const {
		Force gradient = get_gradient(a); 
		float cost = 0.0; 
		typename T::const_iterator ai = a.begin();
		typename R::const_iterator bi = b.begin();
		
		for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi) {
			float delta = float(*ai) - float(*bi); 
			_M_force[i] += gradient[i] * delta * _M_scale;
			cost += delta * delta; 
		}
		return 0.5 * cost * _M_scale; 
	}
private: 
	mutable Force& _M_force; 
	float _M_scale; 
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename TCost> 
double CSSDCost<TCost>::do_evaluate_force(const Data& a, const Data& b, float scale, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce<Force> ef(force, scale); 
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
	typedef typename CP::ProductPtr ProductPtr; 
	TSSDCostPlugin();
	virtual typename TSSDCostPlugin<CP,C>::ProductPtr do_create()const;
};


/**
   This plugin will alwasy be "ssd" 
*/
template <typename CP, typename C> 
TSSDCostPlugin<CP,C>::TSSDCostPlugin():
	CP("ssd")
{
	TRACE("TSSDCostPlugin<CP,C>::TSSDCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
typename TSSDCostPlugin<CP,C>::ProductPtr TSSDCostPlugin<CP,C>::do_create() const
{
	return typename TSSDCostPlugin<CP,C>::ProductPtr(new CSSDCost<C>);
}

NS_END
