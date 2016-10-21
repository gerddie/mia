/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis 
 * Copyright (c) Leipzig, Madrid 1999-2016 Gert Wollny
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
#include <mia/2d/cost.hh>

#include <numeric>
#include <limits>

NS_BEGIN(ssd_2dimage_cost)

NS_MIA_USE; 
using namespace std; 
using namespace boost; 



class CSSDCost: public C2DImageCost {
public: 	

private: 
	virtual double do_value(const C2DImage& a, const C2DImage& b) const; 
	virtual void do_evaluate_force(const C2DImage& a, const C2DImage& b, float scale, C2DFVectorfield& force) const; 
};

struct FEvalSSD : public TFilter<double> {
	
	template <typename T, typename R>
	struct SQD {
		double operator ()(T a, R b) const {
			double d = (double)a - (double)b; 
			return d * d;
		}
	}; 
	
	template <typename  T, typename  R>
	FEvalSSD::result_type operator () (const T2DImage<T>& a, const T2DImage<R>& b) const {
		return inner_product(a.begin(), a.end(), b.begin(), 0.0,  plus<double>(), SQD<T,R>()); 
	}
}; 

double CSSDCost::do_value(const C2DImage& a, const C2DImage& b) const
{
	FEvalSSD essd; 
	return filter(essd, a, b); 
}

struct FEvalForce: public TFilter<int> {
	FEvalForce(C2DFVectorfield& force, float scale):
		m_force(force),
		m_scale(scale)
		{
		}
	template <typename T, typename R> 
	int operator ()( const T2DImage<T>& a, const T2DImage<R>& b) const {
		
		C2DFImage help(a.get_size()); 
		
		typename T2DImage<T>::const_iterator ai = a.begin(); 
		typename T2DImage<T>::const_iterator ae = a.end(); 
		typename T2DImage<R>::const_iterator bi = b.begin(); 
		C2DFImage::iterator hi = help.begin(); 
		while (ai != ae) {
			*hi = ((float)*ai - (float)*bi) * m_scale; 
			++hi; ++ai; ++bi; 
		}
		
		m_force = get_gradient(help); 

		return 0; 
	}
private: 
	mutable C2DFVectorfield& m_force; 
	float m_scale; 

}; 
		

void CSSDCost::do_evaluate_force(const C2DImage& a, const C2DImage& b, float scale, C2DFVectorfield& force) const
{
	assert(a.get_size() == b.get_size()); 
	assert(a.get_size() == force.get_size()); 
	FEvalForce ef(force, scale); 
	filter(ef, a, b); 
}


class C2DSSDCostPlugin: public C2DImageCostPlugin {
public: 
	C2DSSDCostPlugin();
	virtual C2DSSDCostPlugin::ProductPtr do_create()const;
	virtual const string do_get_descr()const; 
private: 

	float m_min; 
	float m_max; 
};


C2DSSDCostPlugin::C2DSSDCostPlugin():
	C2DImageCostPlugin("ssddf")
{
}

C2DSSDCostPlugin::ProductPtr C2DSSDCostPlugin::do_create()const
{
	return C2DSSDCostPlugin::ProductPtr(new CSSDCost()); 
}


const string C2DSSDCostPlugin::do_get_descr()const
{
	return "2D image cost: sum of squared intensity differences, alternative formulation ";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DSSDCostPlugin(); 
}

NS_END
