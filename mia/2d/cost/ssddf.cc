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

	virtual bool do_test() const; 

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

bool C2DSSDCostPlugin::do_test() const
{
	bool success = true; 
	const float src_data[16] = {
		0, 0, 0, 0, 
		0, 3, 1, 0, 
		0, 6, 7, 0, 
		0, 0, 0, 0
	}; 
	const float ref_data[16] = {
		0, 0, 0, 0, 
		0, 2, 3, 0, 
		0, 1, 2, 0, 
		0, 0, 0, 0
	}; 

	C2DFImage *fsrc = new C2DFImage(C2DBounds(4,4), src_data ); 
	C2DFImage *fref = new C2DFImage(C2DBounds(4,4), ref_data ); 
	std::shared_ptr<C2DImage > src(fsrc); 
	std::shared_ptr<C2DImage > ref(fref); 

	CSSDCost cost; 

	success &= (cost.value(*src, *ref) == 55.0); 
	
	C2DFVectorfield force(C2DBounds(4,4)); 
	
	cost.evaluate_force(*src, *ref, 2.0, force); 

	success &=  (force(1,1) == C2DFVector(-2.0, 5.0)); 

	return success; 
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
