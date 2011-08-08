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


/*
  things to optimize:
  cache the ngf of the reference image, since it never changes
  merge get value and get force should probably be only one call
*/

#include <numeric>
#include <mia/core/msgstream.hh>
#include <mia/3d/nfg.hh>
#include <mia/3d/3dimageio.hh>
#include <mia/3d/cost/fatngf.hh>

using namespace std;
using namespace boost;
using namespace mia;

//#define USE_CROSS

NS_BEGIN(nfg_3dimage_fatcost)

class FScalar: public FEvaluator {
public:
	virtual double cost (const C3DFVector& src, const C3DFVector& ref) const{
		double d = dot(src, ref);
		return - d * d;
	}
	virtual C3DFVector  grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			     const C3DFVector& ref, double& cost) const	{
		double d = dot(*isrc,ref);
		cost += d * d;
		C3DFVector result ( dot(isrc[1] - isrc[-1], ref),
				    dot(isrc[nx] - isrc[-nx], ref),
				    dot(isrc[nxy] - isrc[-nxy], ref));
		return - d * result;
	}
};

class FCross: public FEvaluator {
public:
	virtual double cost (const C3DFVector& src, const C3DFVector& ref) const{
		C3DFVector d = cross(src, ref);
		return d.norm2(); 
	}
	virtual C3DFVector  grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			     const C3DFVector& ref, double& cost) const {
		C3DFVector d = cross(*isrc, ref);
		cost += d.norm2();

		return C3DFVector  ( dot(d, cross(isrc[1] - isrc[-1], ref)),
				     dot(d, cross(isrc[nx] - isrc[-nx], ref)),
				     dot(d, cross(isrc[nxy] - isrc[-nxy], ref)));
	}
};

double FDeltaScalar::cost (const C3DFVector& src, const C3DFVector& ref) const
{
	double d = get_dot(src, ref);
	return d * d;
}

C3DFVector FDeltaScalar::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			       const C3DFVector& ref, double& cost) const 
{
	double d = get_dot(*isrc,ref);
	cost += d * d;
	C3DFVector result ( dot(isrc[1] - isrc[-1], ref),
			    dot(isrc[nx] - isrc[-nx], ref),
			    dot(isrc[nxy] - isrc[-nxy], ref));
	return d * result;
}

double FDeltaScalar::get_dot(const C3DFVector& src, const C3DFVector& ref)const {
	return dot (src, ref) > 0 ? dot(src - ref, ref): dot(src + ref, ref);
}

struct FGetMinMax : public TFilter<float> {
	template <typename T>
	float operator ()( const T3DImage<T>& image) const {
		pair<typename T3DImage<T>::const_iterator, typename T3DImage<T>::const_iterator>
			mm = minmax_element(image.begin(), image.end());
		return *mm.second - *mm.first;
	}
};


CFatNFG3DImageCost::CFatNFG3DImageCost(P3DImage src, P3DImage ref, float weight, PEvaluator evaluator):
	C3DImageFatCost(src,  ref,  weight),
	m_jump_levels_valid(false),
	m_evaluator(evaluator),
	m_intensity_scale(1.0)
{
	FGetMinMax fgmm;
	double src_range = mia::filter(fgmm, *src); 
	double ref_range = mia::filter(fgmm, *ref); 
	m_intensity_scale = src_range * ref_range;
	cvdebug() << "src_range= " << src_range << "\n"; 
	cvdebug() << "ref_range= " << ref_range << "\n"; 
	cvdebug() << "m_intensity_scale = " << m_intensity_scale << "\n";
}

P3DImageFatCost CFatNFG3DImageCost::cloned(P3DImage src, P3DImage ref) const
{
	return P3DImageFatCost(new CFatNFG3DImageCost(src, ref,  get_weight(), m_evaluator));
}

void CFatNFG3DImageCost::prepare() const
{

	m_ng_ref =  get_nfg(get_ref());
	m_jump_levels_valid = true;
}

double CFatNFG3DImageCost::do_value() const
{
	TRACE("CFatNFG3DImageCost::do_value");
	if (!m_jump_levels_valid) {
		prepare();
	}

	C3DFVectorfield ng_a = get_nfg(get_floating());

	double sum = 0.0;
	for (C3DFVectorfield::const_iterator ia = ng_a.begin(), ib = m_ng_ref.begin();
	     ia != ng_a.end(); ++ia, ++ib) {
		sum += m_evaluator->cost(*ia, *ib);
	}
	return 0.5  * m_intensity_scale * get_weight() * sum / ng_a.size();
}

double CFatNFG3DImageCost::do_evaluate_force(C3DFVectorfield& force) const
{
	TRACE("CFatNFG3DImageCost::do_evaluate_force");
	if (!m_jump_levels_valid) {
		prepare();
	}

	const C3DFVectorfield ng_src = get_nfg(get_floating());
	const float weight =  m_intensity_scale * get_weight();

	const size_t nx = m_ng_ref.get_size().x;
	const size_t ny = m_ng_ref.get_size().y;
	const size_t nz = m_ng_ref.get_size().z;
	const size_t nxy = nx * ny;


	double cost = 0.0;
#ifdef __OPENMP
#pragma omp parallel for shared(cost, force) schedule(static) default(none)
#endif
	for (size_t z = 1; z < nz - 1; ++z) {
		C3DFVectorfield::const_iterator isrc = ng_src.begin_at(1, 1, z);
		C3DFVectorfield::const_iterator iref = m_ng_ref.begin_at(1, 1, z);
		C3DFVectorfield::iterator iforce = force.begin_at(1, 1, z);
		double lcost = 0.0;
		for (size_t y = 1; y < ny - 1; ++y) {
			for (size_t x = 1; x < nx - 1; ++x, ++iforce, ++isrc, ++iref) {
				*iforce =  weight * 0.5 * m_evaluator->grad(nx, nxy, isrc, *iref, lcost);
			}
			iforce += 2;
			isrc   += 2;
			iref   += 2;
		}
#ifdef __OPENMP
#pragma omp critical
#endif
		cost += lcost;
	}

	return 0.5 * m_intensity_scale * cost / ng_src.size();
}

class C3DNFGFatImageCostPlugin: public C3DFatImageCostPlugin {
public:
	C3DNFGFatImageCostPlugin();
private:
	virtual C3DImageFatCost *do_create(P3DImage src, P3DImage ref, 
					   P3DInterpolatorFactory ipf, float weight)const;
	bool do_test() const;
	const string do_get_descr()const;
	string m_type;
};

C3DNFGFatImageCostPlugin::C3DNFGFatImageCostPlugin():
	C3DFatImageCostPlugin("ngf"),
	m_type("delta")
{
	TRACE("C3DNFGFatImageCostPlugin::C3DNFGFatImageCostPlugin()");
	add_parameter("eval", new CStringParameter(m_type, true,
						   "plugin subtype (delta, scalar,cross)"));

}

enum ESubTypes {st_unknown, st_delta, st_scalar, st_cross};

C3DImageFatCost *C3DNFGFatImageCostPlugin::do_create(P3DImage src, P3DImage ref, 
						     P3DInterpolatorFactory ipf, float weight)const
{
	TRACE("C3DNFGFatImageCostPlugin::do_create");



	const TDictMap<ESubTypes>::Table lut[] = {
		{"delta", st_delta},
		{"scalar", st_scalar},
		{"cross", st_cross},
		{0, st_unknown}
	};
	const TDictMap<ESubTypes> subtypemap(lut);

	PEvaluator eval;
	switch (subtypemap.get_value(m_type.c_str())) {
	case st_delta: eval.reset(new FDeltaScalar()); break;
	case st_scalar: eval.reset(new FScalar()); break;
	case st_cross: eval.reset(new FCross()); break;
	default:
		throw invalid_argument(string("C3DNFGFatImageCostPlugin: unknown cost sub-type '")+m_type+"'");
	}
	return new CFatNFG3DImageCost(src, ref, weight, eval);
}

bool C3DNFGFatImageCostPlugin::do_test() const
{
	return true; 

}

const string C3DNFGFatImageCostPlugin::do_get_descr()const
{
	return "3D nfg cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DNFGFatImageCostPlugin();
}

NS_END
