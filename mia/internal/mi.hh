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

#include <mia/core/filter.hh>
#include <mia/core/msgstream.hh>
#include <mia/core/parameter.hh>
#include <mia/core/splineparzenmi.hh>
#include <mia/core/property_flags.hh>

#include <numeric>
#include <limits>

NS_BEGIN(NS)

template <typename T> 
class TMIImageCost: public T {
public: 	
	typedef typename T::Data Data; 
	typedef typename T::Force Force; 

	TMIImageCost(size_t fbins, mia::PSplineKernel fkernel, size_t rbins, mia::PSplineKernel rkernel); 
private: 
	virtual double do_value(const Data& a, const Data& b) const; 
	virtual double do_evaluate_force(const Data& a, const Data& b, float scale, Force& force) const; 

	mutable mia::CSplineParzenMI m_parzen_mi; 

};


struct FEvalMI : public mia::TFilter<double> {
	FEvalMI( mia::CSplineParzenMI& parzen_mi):
		m_parzen_mi(parzen_mi)
		{}
	

	template <typename  T, typename  R>
	FEvalMI::result_type operator () (const T& a, const R& b) const {
		m_parzen_mi.fill(a.begin(), a.end(), b.begin(), b.end()); 
		return  m_parzen_mi.value(); 
	}
	bool m_normalize; 
	mia::CSplineParzenMI& m_parzen_mi; 
}; 


template <typename T> 
TMIImageCost<T>::TMIImageCost(size_t rbins, mia::PSplineKernel rkernel, size_t mbins, mia::PSplineKernel mkernel):
	m_parzen_mi(rbins, rkernel, mbins,  mkernel)
	
{
	this->add(::mia::property_gradient);
}

template <typename T> 
double TMIImageCost<T>::do_value(const Data& a, const Data& b) const
{
	FEvalMI essd(m_parzen_mi); 
	return filter(essd, a, b); 
}

template <typename Force>
struct FEvalForce: public mia::TFilter<float> {
	FEvalForce(Force& force, float scale, mia::CSplineParzenMI& parzen_mi):
		m_force(force), 
		m_scale(scale),
		m_parzen_mi(parzen_mi)
		{
		}
	template <typename T, typename R> 
	float operator ()( const T& a, const R& b) const {
		Force gradient = get_gradient(a); 
		m_parzen_mi.fill(a.begin(), a.end(), b.begin(), b.end()); 
		typename T::const_iterator ai = a.begin();
		typename R::const_iterator bi = b.begin();
	
		for (size_t i = 0; i < a.size(); ++i, ++ai, ++bi) {
			float delta = m_parzen_mi.get_gradient(*ai, *bi); 
			m_force[i] = gradient[i] * delta * m_scale;
		}
		return m_parzen_mi.value() * m_scale; 
	}
private: 
	mutable Force& m_force; 
	float m_scale; 
	mia::CSplineParzenMI& m_parzen_mi;
}; 
		

/**
   This is the force evaluation routine of the cost function   
*/
template <typename T> 
double TMIImageCost<T>::do_evaluate_force(const Data& a, const Data& b, float scale, Force& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce<Force> ef(force, scale, m_parzen_mi); 
	return filter(ef, a, b); 
}


/**
   This is the plug-in declaration - the actual plugin needs to define the 
   cost plugin type and the data type (this could be unified) 
   do_test and do_get_descr need to be implemented 
*/
template <typename CP, typename C> 
class TMIImageCostPlugin: public CP {
public: 
	TMIImageCostPlugin();
	C *do_create()const;
private: 
	const std::string do_get_descr() const; 
	unsigned int m_rbins;  
	unsigned int m_mbins;  
	std::string m_mkernel; 
	std::string m_rkernel;
};


/**
   This plugin will alwasy be "ssd" 
*/
template <typename CP, typename C> 
TMIImageCostPlugin<CP,C>::TMIImageCostPlugin():
	CP("mi"), 
	m_rbins(64), 
	m_mbins(64), 
	m_mkernel("bspline:d=3"), 
	m_rkernel("bspline:d=0")
{
	TRACE("TMIImageCostPlugin<CP,C>::TMIImageCostPlugin()"); 
	this->add_property(::mia::property_gradient); 
	this->add_parameter("rbins", new mia::CUIntParameter(m_rbins, 1, 256, false, 
				     "Number of histogram bins used for the reference image")); 

	this->add_parameter("mbins", new mia::CUIntParameter(m_mbins, 1, 256, false, 
				     "Number of histogram bins used for the moving image")); 
	
	this->add_parameter("rkernel", new mia::CStringParameter(m_rkernel, false, 
				     "Spline kernel for reference image parzen hinstogram")); 

	this->add_parameter("mkernel", new mia::CStringParameter(m_mkernel, false, 
				     "Spline kernel for moving image parzen hinstogram")); 
}

/**
   The creator routine is also generic
*/
template <typename CP, typename C> 
C *TMIImageCostPlugin<CP,C>::do_create() const
{
	auto mkernel = mia::produce_spline_kernel(m_mkernel); 
	auto rkernel = mia::produce_spline_kernel(m_rkernel); 
	return new TMIImageCost<C>(m_rbins, rkernel, m_mbins,  mkernel); 
}

template <typename CP, typename C> 
const std::string TMIImageCostPlugin<CP,C>::do_get_descr() const
{
	return "Spline parzen based mutual information";  
	
}

NS_END
