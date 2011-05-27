/* -*- mia-c++  -*-
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#include <mia/3d/cost/ngf.hh>
#include <mia/3d/nfg.hh>


NS_BEGIN(ngf_3dimage_cost)

using namespace std;
using namespace boost;
using namespace mia;


double FEvaluator::operator()(const mia::C3DFVector& src, const mia::C3DFVector& ref) const
{
	return cost(src, ref); 
}

double FScalar::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	double d = dot(src, ref);
	return - d * d * 0.5;
}
C3DFVector  FScalar::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			   const C3DFVector& ref, double& cost) const	
{
	double d = dot(*isrc,ref);
	cost -= d * d * 0.5;
	C3DFVector result ( dot(isrc[1] - isrc[-1], ref),
			    dot(isrc[nx] - isrc[-nx], ref),
			    dot(isrc[nxy] - isrc[-nxy], ref));
	return - d * result;
}

double FCross::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	C3DFVector d = cross(src, ref);
	return 0.5 * d.norm2(); 
}

C3DFVector  FCross::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
	      const C3DFVector& ref, double& cost) const 
{
	C3DFVector d = cross(*isrc, ref);
	cost += 0.5 * d.norm2();
	
	return C3DFVector  ( dot(d, cross(isrc[1] - isrc[-1], ref)),
			     dot(d, cross(isrc[nx] - isrc[-nx], ref)),
			     dot(d, cross(isrc[nxy] - isrc[-nxy], ref)));
}

double FDeltaScalar::cost (const C3DFVector& src, const C3DFVector& ref) const
{
	double dotss = src.norm2(); 
	double dotrr = ref.norm2(); 
	double dotsr = dot(src, ref); 
	double f = dotss *dotrr; 
	if ( f > 0.0) 
		f = dotsr / sqrt(f); 
	auto delta = ref - f * src; 
	return 0.5 * dot(delta, delta); 
}

C3DFVector FDeltaScalar::grad (int nx, int nxy, C3DFVectorfield::const_iterator isrc,
			     const C3DFVector& ref, double& cost) const
{
	const double dotss = isrc->norm2(); 
	const double dotrr = ref.norm2(); 
	const double dotsr = dot(*isrc, ref); 
	const double f = dotss *dotrr; 
	const double onebydotss = dotss > 0 ? 1.0/dotss: 0.0; 
	const double onybyf =  f > 0.0 ?  1.0 / sqrt(f) : 0.0; 
	const double cos_a = dotsr * onybyf; 
	const auto delta = ref - cos_a * *isrc; 
	
	cost += 0.5 * dot(delta, delta); 
	
	const C3DFVector s2dx =   0.5 * (isrc[1] -   isrc[-1]); 
	const C3DFVector s2dy =  0.5 * (isrc[nx] -  isrc[-nx]); 
	const C3DFVector s2dz = 0.5 * (isrc[nxy] - isrc[-nxy]); 
	
	const double p1 = cos_a * onebydotss; 
	const double p2 =  onybyf; 
	return C3DFVector( dot(delta, (p1 * dot(*isrc, s2dx)  - p2 * dot(ref, s2dx)) * *isrc - cos_a *s2dx), 
			   dot(delta, (p1 * dot(*isrc, s2dy)  - p2 * dot(ref, s2dy)) * *isrc - cos_a *s2dy), 
			   dot(delta, (p1 * dot(*isrc, s2dz)  - p2 * dot(ref, s2dz)) * *isrc - cos_a *s2dz)); 
	
}

double FDeltaScalar::get_dot(const C3DFVector& src, const C3DFVector& ref)const 
{
	
}


C3DNFGImageCost::C3DNFGImageCost(PEvaluator evaluator):
	m_evaluator(evaluator)
{
	add(property_gradient);
}

void C3DNFGImageCost::prepare_reference(const C3DImage& ref)
{
	post_set_reference(ref); 
}

void C3DNFGImageCost::post_set_reference(const mia::C3DImage& ref)
{
	m_ng_ref =  get_nfg(ref);
}



double C3DNFGImageCost::do_value(const mia::C3DImage& a, const mia::C3DImage& /*b*/) const
{
	TRACE("CNFG3DImageCost::do_value");
	const C3DFVectorfield ng_a = get_nfg(a);
	const double sum = inner_product(ng_a.begin(), ng_a.end(), m_ng_ref.begin(), 0.0, 
					 [](double x, double y) {return x + y;},
					 [m_evaluator](const C3DFVector ix, const C3DFVector iy) {
						 return m_evaluator->cost(ix, iy); 
					 }); 
	return 0.5 * sum;
}

double C3DNFGImageCost::do_evaluate_force(const mia::C3DImage& a, 
					  const mia::C3DImage& /*b*/, 
					  float /*scale*/,
					  mia::C3DFVectorfield& force) const
{
	const C3DFVectorfield ng_a = get_nfg(a);
	double sum = 0.0; 
	const int nx = ng_a.get_size().x; 
	const int nxy = nx * ng_a.get_size().y; 
	
	auto ia = ng_a.begin_range(C3DBounds::_1, ng_a.get_size() - C3DBounds::_1); 
	auto ie = ng_a.end_range(C3DBounds::_1, ng_a.get_size() - C3DBounds::_1); 
	auto ib = m_ng_ref.begin_range(C3DBounds::_1, m_ng_ref.get_size() - C3DBounds::_1); 
	auto iforce = force.begin_range(C3DBounds::_1, force.get_size() - C3DBounds::_1); 

	while (ia != ie) {
		*iforce = m_evaluator->grad (nx, nxy, ia.get_point(), *ib, sum);
		++ia; 
		++ib; 
		++iforce; 
	}; 

	return 0.5 * sum;
}

C3DNFGImageCostPlugin::C3DNFGImageCostPlugin():
	C3DImageCostPlugin("ngf"),
	m_kernel("ds")
{
	TRACE("C3DNFGImageCostPlugin::C3DNFGImageCostPlugin()");
	add_parameter("eval",
		      new CStringParameter(m_kernel, false, "plugin subtype (sq, ds,dot,cross)"));

}

enum ESubTypes {st_unknown, st_delta_scalar, st_scalar, st_cross};

C3DImageCostPlugin::ProductPtr C3DNFGImageCostPlugin::do_create()const
{
	TRACE("C3DNFGImageCostPlugin::do_create");

	const TDictMap<ESubTypes>::Table lut[] = {
		{"ds", st_delta_scalar},
		{"dot", st_scalar},
		{"cross", st_cross},
		{0, st_unknown}
	};
	const TDictMap<ESubTypes> subtypemap(lut);

	PEvaluator eval;
	switch (subtypemap.get_value(m_kernel.c_str())) {
	case st_delta_scalar: eval.reset(new FDeltaScalar()); break;
	case st_scalar: eval.reset(new FScalar()); break;
	case st_cross: eval.reset(new FCross()); break;
	default:
		throw invalid_argument(string("C3DNFGImageCostPlugin: unknown cost sub-type '")
				       +m_kernel+"'");
	}
	return C3DImageCostPlugin::ProductPtr(new C3DNFGImageCost(eval));
}

bool C3DNFGImageCostPlugin::do_test() const
{
	return true;
}

const string C3DNFGImageCostPlugin::do_get_descr()const
{
	return "3D nfg cost function";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C3DNFGImageCostPlugin();
}


NS_END
