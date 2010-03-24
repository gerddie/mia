/* -*- mia-c++  -*-
 * Copyright (c) 2007 Gert Wollny 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

/*
  things to optimize:
  cache the ngf of the reference image, since it never changes
  merge get value and get force should probably be only one call
*/

#define VSTREAM_DOMAIN "GF-COST" 
#include <numeric>
#include <boost/algorithm/minmax_element.hpp>

#include <mia/core/msgstream.hh>
#include <mia/2d/cost.hh>
#include <mia/2d/fatcost.hh>
#include <mia/2d/nfg.hh>
#include <mia/2d/2dimageio.hh>

using namespace std; 
using namespace boost; 
using namespace mia; 

//#define USE_CROSS

NS_BEGIN(gf_2dimage_fatcost)

class FEvaluator {
public: 
        virtual ~FEvaluator(){}; 
	virtual double operator () (const C2DFVector& src, const C2DFVector& ref) const = 0; 
	virtual double grad (const C2DFVector& src, const C2DFVector& ref) const = 0; 
	virtual double negate() { return 1.0; }
};

class FScalar: public FEvaluator {
public: 
	virtual double operator () (const C2DFVector& src, const C2DFVector& ref) const{
		return dot(src, ref); 
	}
	virtual double grad (const C2DFVector& src, const C2DFVector& ref) const {
		return -dot(src, ref); 
	}
	virtual double negate() { return -1.0; }
};

class FCross: public FEvaluator {
public: 
	virtual double operator () (const C2DFVector& src, const C2DFVector& ref) const{
		return cross(src, ref); 
	}
	virtual double grad (const C2DFVector& src, const C2DFVector& ref) const {
		return cross(src, ref); 
	}
};

class FDeltaScalar: public FEvaluator {
public: 
	virtual double operator () (const C2DFVector& src, const C2DFVector& ref) const{
		return dot (src, ref) > 0 ? dot(src - ref, ref): dot(src + ref, ref); 
	}
	virtual double grad (const C2DFVector& src, const C2DFVector& ref) const {
		return -dot(src, ref); 
	}
};

typedef SHARED_PTR(FEvaluator) PEvaluator; 

class CFatGF2DImageCost : public C2DImageFatCost {
public: 
	CFatGF2DImageCost(P2DImage src, P2DImage ref, float weight, PEvaluator evaluator);
private: 	
	virtual P2DImageFatCost cloned(P2DImage src, P2DImage ref, float weight) const; 
	virtual double do_value() const; 
	virtual double do_evaluate_force(C2DFVectorfield& force) const;
	void prepare() const;  
	mutable C2DFVectorfield _M_ng_ref; 
	mutable bool _M_ref_prepared; 
	PEvaluator _M_evaluator; 
	float _M_intensity_scale; 
}; 


struct FGetMinMax : public TFilter<float> {
	template <typename T> 
	float operator ()( const T2DImage<T>& image) const {
		pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator> 
			mm = ::boost::minmax_element(image.begin(), image.end());
		return *mm.second - *mm.first; 
	}
}; 


CFatGF2DImageCost::CFatGF2DImageCost(P2DImage src, P2DImage ref, float weight, PEvaluator evaluator):
	C2DImageFatCost(src,  ref,  weight), 
	_M_ref_prepared(false), 
	_M_evaluator(evaluator), 
	_M_intensity_scale(1.0)
{
	FGetMinMax fgmm;
	_M_intensity_scale = mia::filter(fgmm, *src) * mia::filter(fgmm, *ref); 
	cvdebug() << "_M_intensity_scale = " << _M_intensity_scale << "\n"; 
}

P2DImageFatCost CFatGF2DImageCost::cloned(P2DImage src, P2DImage ref, float weight) const
{
	return P2DImageFatCost(new CFatGF2DImageCost(src, ref,  weight, _M_evaluator)); 
}

void CFatGF2DImageCost::prepare() const
{
	_M_ng_ref =  get_gradient(get_ref()); 
	_M_ref_prepared = true; 
}

double CFatGF2DImageCost::do_value() const
{
	TRACE("CFatGF2DImageCost::do_value"); 
	if (!_M_ref_prepared)
		prepare(); 

	
	C2DFVectorfield ng_a = get_gradient(get_floating()); 

	double sum = 0.0; 
	for (C2DFVectorfield::const_iterator ia = ng_a.begin(), ib = _M_ng_ref.begin(); 
	     ia != ng_a.end(); ++ia, ++ib) {
		double help = (*_M_evaluator)(*ia, *ib); 
		sum += help * help; 
	}
	return 0.5 * _M_evaluator->negate() * _M_intensity_scale * get_weight() * sum / ng_a.size(); 
}

double CFatGF2DImageCost::do_evaluate_force(C2DFVectorfield& force) const
{
	TRACE("CFatGF2DImageCost::do_evaluate_force"); 
	if (!_M_ref_prepared) {
		prepare(); 
	}

	C2DFVectorfield ng_src = get_gradient(get_floating()); 
	float weight =  _M_intensity_scale * get_weight();

	const size_t nx = _M_ng_ref.get_size().x;
	const size_t ny = _M_ng_ref.get_size().y;
	
	C2DFVectorfield::const_iterator isrc = ng_src.begin() + nx; 
	C2DFVectorfield::const_iterator iref = _M_ng_ref.begin() + nx; 
	C2DFVectorfield::iterator iforce = force.begin() + nx; 
	
	double cost = 0.0;
	for (size_t y = 1; y < ny - 1; ++y) {
		++iforce; 
		++isrc; 
		++iref;
		for (size_t x = 1; x < nx - 1; ++x, ++iforce, ++isrc, ++iref) {
			double help =  (*_M_evaluator)(*isrc, *iref);
			cost += help * help; 
			
			iforce->x += weight * 0.5 * help * _M_evaluator->grad(isrc[1] - isrc[-1], *iref); 
			iforce->y += weight * 0.5 * help * _M_evaluator->grad(isrc[nx] - isrc[-nx], *iref); 
		}
		++iforce; 
		++isrc;
		++iref; 
	}
	
	return 0.5 * _M_evaluator->negate() * _M_intensity_scale * cost / ng_src.size();
}

class C2DGFFatImageCostPlugin: public C2DFatImageCostPlugin {
public: 
	C2DGFFatImageCostPlugin(); 
private: 
	virtual C2DFatImageCostPlugin::ProductPtr do_create(P2DImage src, 
							    P2DImage ref, float weight)const;
	bool do_test() const; 
	const string do_get_descr()const; 
	string _M_type; 
}; 

C2DGFFatImageCostPlugin::C2DGFFatImageCostPlugin():
	C2DFatImageCostPlugin("gf"), 
	_M_type("delta")
{
	TRACE("C2DGFFatImageCostPlugin::C2DGFFatImageCostPlugin()"); 
	add_parameter("eval", new CStringParameter(_M_type, true, 
						   "plugin subtype (delta, scalar,cross)")); 
		
}

enum ESubTypes {st_unknown, st_delta, st_scalar, st_cross}; 

C2DFatImageCostPlugin::ProductPtr C2DGFFatImageCostPlugin::do_create(P2DImage src, P2DImage ref, float weight)const
{
	TRACE("C2DGFFatImageCostPlugin::do_create"); 

	
	
	const TDictMap<ESubTypes>::Table lut[] = {
		{"delta", st_delta}, 
		{"scalar", st_scalar}, 
		{"cross", st_cross}, 
		{0, st_unknown}
	}; 
	const TDictMap<ESubTypes> subtypemap(lut); 

	PEvaluator eval; 
	switch (subtypemap.get_value(_M_type.c_str())) {
	case st_delta: eval.reset(new FDeltaScalar()); break; 
	case st_scalar: eval.reset(new FScalar()); break; 
	case st_cross: eval.reset(new FCross()); break; 
	default: 
		throw invalid_argument(string("C2DGFFatImageCostPlugin: unknown cost sub-type '")+_M_type+"'");
	}
	return C2DFatImageCostPlugin::ProductPtr(new CFatGF2DImageCost(src, ref, weight, eval)); 
}
	
bool C2DGFFatImageCostPlugin::do_test() const
{

	bool success = true; 

	const C2DBounds size(7,7); 
	float init_ref[49] = { -1, 0, 1, 2, 1, 0, -1, 
			       0, 1, 2, 3, 2, 1,  0, 
			       1, 2, 3, 4, 3, 2,  1,  
			       2, 3, 4, 5, 4, 3,  2,  
			       1, 2, 3, 4, 3, 2,  1,  
			       0, 1, 2, 3, 2, 1,  0, 
			       -1, 0, 1, 2, 1, 0, -1 
	};
	
	float init_src[49] = { 	 1, 1, 1, 1, 1, 1, 1,
				 1, 1, 1, 1, 3, 1, 1,
				 1, 1, 1, 3, 5, 3, 1,
				 1, 1, 1, 1, 3, 1, 1,
				 1, 1, 1, 1, 1, 1, 1, 
				 1, 1, 1, 1, 1, 1, 1,
				 1, 1, 1, 1, 1, 1, 1 
 	}; 

	P2DImage src(new C2DFImage(size, init_src)); 
	P2DImage ref(new C2DFImage(size, init_ref)); 
	
	C2DFVectorfield force_self(size);

	CFatGF2DImageCost cost_self(ref, ref, 1.0, PEvaluator(new FDeltaScalar())); 
	double cost_value_self = cost_self.evaluate_force(force_self); 
	if (fabs(cost_value_self) > 0.01) {
		cvfail() << "cost with myself is " << cost_value_self << " expect 0.0\n"; 
		success = false; 
	}

	

	return success; 

}

const string C2DGFFatImageCostPlugin::do_get_descr()const
{
	return "2D gradient field cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DGFFatImageCostPlugin(); 
}

NS_END
