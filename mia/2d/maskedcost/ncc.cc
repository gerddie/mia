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

#include <mia/2d/maskedcost/ncc.hh>

#include <mia/core/threadedmsg.hh>
#include <mia/core/nccsum.hh> 
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>


NS_BEGIN(NS)

using namespace mia; 


CNCC2DImageCost::CNCC2DImageCost()
{
}

template <typename T, typename S> 
struct FEvaluateNCCSum {
	FEvaluateNCCSum(const C2DBitImage& mask, const T& mov, const S& ref); 
	NCCSums operator ()(const tbb::blocked_range<size_t>& range, const NCCSums& sumacc) const; 
private: 
	C2DBitImage m_mask; 
	T m_mov; 
	S m_ref; 
};


template <typename T, typename S> 
FEvaluateNCCSum<T,S>::FEvaluateNCCSum(const C2DBitImage& mask, const T& mov, const S& ref):
	m_mask(mask),  m_mov(mov), m_ref(ref) 
{
	
}

template <typename T, typename S> 
NCCSums FEvaluateNCCSum<T,S>::operator ()(const tbb::blocked_range<size_t>& range, const NCCSums& sumacc) const
{
	CThreadMsgStream msks; 
	
	NCCSums sum;
	for (auto z = range.begin(); z != range.end(); ++z) {
		auto imask  = m_mask.begin_at(0,z);
		auto ia = m_mov.begin_at(0, z); 
		auto ib = m_ref.begin_at(0, z); 
		auto eb = m_ref.begin_at(0, z + 1); 
		
		while (ib != eb) {
			if (*imask) {
				sum.add(*ia, *ib); 
			}
			++ia; ++ib; ++imask; 
		}
	}
	return sum + sumacc; 
};


class FEvalCost : public TFilter<float> {
	const C2DBitImage& m_mask; 
public:
	FEvalCost(const C2DBitImage& mask):
		m_mask(mask)
		{}
	
	template <typename T, typename R> 
	float operator () ( const T& mov, const R& ref) const {

		FEvaluateNCCSum<T,R> ev(m_mask, mov, ref); 
		NCCSums sum; 
		sum = parallel_reduce(tbb::blocked_range<size_t>(0, mov.get_size().y, 1), sum, ev, 
				      [](const NCCSums& x, const NCCSums& y){
					      return x + y;
				      });
		return sum.value(); 
	}
}; 


double CNCC2DImageCost::do_value(const Data& a, const Data& b, const Mask& m) const
{
	FEvalCost ecost(m); 
	return mia::filter(ecost, a, b); 
}


class FEvalCostForce : public TFilter<float> {
	const C2DBitImage& m_mask; 
	C2DFVectorfield& m_force; 
public: 
	FEvalCostForce(const C2DBitImage& mask, C2DFVectorfield& force):
		m_mask(mask), 
		m_force(force)
		{}
	
	template <typename T, typename R> 
	float operator () ( const T& mov, const R& ref) const {
		CThreadMsgStream msks;
		
		NCCSums sum; 
		FEvaluateNCCSum<T,R> ev(m_mask, mov, ref); 
		sum = parallel_reduce(tbb::blocked_range<size_t>(0, mov.get_size().y, 1), sum, ev, 
					 [](const NCCSums& x, const NCCSums& y){
					      return x + y;
				      });
		
		auto geval = sum.get_grad_helper(); 

		auto grad = get_gradient(mov); 
		auto grad_eval = [this, &mov, &ref, &grad, &geval](const tbb::blocked_range<size_t>& range) {
			for (auto z = range.begin(); z != range.end(); ++z) {
				auto ig = grad.begin_at(0,z); 
				auto iforce = m_force.begin_at(0,z); 
				auto im = m_mask.begin_at(0,z); 
				auto ia = mov.begin_at(0,z); 
				auto ib = ref.begin_at(0,z); 
				auto eb = ref.begin_at(0,z+1); 
				
				while (ib != eb) {
					if (*im) {
						*iforce = geval.second.get_gradient_scale(*ia, *ib) * *ig; 
					}
					++ig; 
					++iforce; 
					++ia; ++ib; ++im; 
				}
			}; 
		}; 
		
		parallel_for(tbb::blocked_range<size_t>(0, mov.get_size().y, 1), grad_eval); 

		return geval.first; 
	}
	
};

double CNCC2DImageCost::do_evaluate_force(const Data& a, const Data& b, const Mask& m, Force& force) const
{
	FEvalCostForce ecostforce(m, force); 
	return mia::filter(ecostforce, a, b); 
}


CNCC2DImageCostPlugin::CNCC2DImageCostPlugin():
C2DMaskedImageCostPlugin("ncc")
{
}

C2DMaskedImageCost *CNCC2DImageCostPlugin::do_create() const
{
	return new CNCC2DImageCost();
}

const std::string CNCC2DImageCostPlugin::do_get_descr() const
{
	return "normalized cross correlation with masking support."; 
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new CNCC2DImageCostPlugin();
}

NS_END
