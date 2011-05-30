/* -*- mia-c++  -*-
 *
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

C3DFMatrix FEvaluator::get_gradient(C3DFVectorfield::const_range_iterator& irsrc, int nx, int nxy) const
{
	cvdebug() << irsrc.get_boundary_flags() << "\n"; 
	C3DFMatrix result; 
	C3DFVectorfield::const_iterator isrc = irsrc.get_point(); 
	
	if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_x))
		result.x = 0.5 * (isrc[1] - isrc[-1]); 
	
	if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_y))
		result.y = 0.5 * (isrc[nx] - isrc[-nx]); 
	
	if (! (irsrc.get_boundary_flags() & C3DFVectorfield::const_range_iterator::eb_z))
		result.z = 0.5 * (isrc[nxy] - isrc[-nxy]); 

	cvdebug() << irsrc.get_boundary_flags() << "\n"; 
	cvdebug() << result << "\n"; 

	return result; 
}

double FScalar::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	double d = dot(src, ref);
	return - d * d;
}



C3DFVector  FScalar::grad (int nx, int nxy, C3DFVectorfield::const_range_iterator irsrc,
			   const C3DFVector& ref, double& cost) const	
{
	double d = dot(*irsrc,ref);
	cost -= d * d;
	return - d * (ref * get_gradient(irsrc, nx, nxy)); 
}

double FCross::cost(const C3DFVector& src, const C3DFVector& ref) const
{
	C3DFVector d = cross(src, ref);
	return d.norm2(); 
}

C3DFVector  FCross::grad (int nx, int nxy, C3DFVectorfield::const_range_iterator irsrc,
	      const C3DFVector& ref, double& cost) const 
{
	C3DFVector d = cross(*irsrc, ref);
	cost += d.norm2();
	
	C3DFMatrix src_grad = get_gradient(irsrc, nx, nxy); 
	
	return C3DFVector  ( dot(d, cross(src_grad.x, ref)),
			     dot(d, cross(src_grad.y, ref)),
			     dot(d, cross(src_grad.z, ref)));
}

struct DotHelper {
	double dotss; 
	double dotsr; 
	double f; 
	double cos_a; 
	C3DFVector delta; 
	DotHelper(const C3DFVector& src, const C3DFVector& ref); 
}; 

DotHelper::DotHelper(const C3DFVector& src, const C3DFVector& ref):
	dotss(src.norm2()), 
	dotsr(dot(src, ref)), 
	f( dotss * ref.norm2()), 
	cos_a( f > 0.0 ? dotsr / sqrt(f) : 0.0), 
	delta(ref - cos_a * src)
{
}

double FDeltaScalar::cost (const C3DFVector& src, const C3DFVector& ref) const
{
	DotHelper dh(src, ref); 
	return dot(dh.delta, dh.delta); 
}

C3DFVector FDeltaScalar::grad (int nx, int nxy, C3DFVectorfield::const_range_iterator irsrc,
			     const C3DFVector& ref, double& cost) const
{
	DotHelper dh(*irsrc, ref); 
	const double onebydotss = dh.dotss > 0 ? 1.0/dh.dotss: 0.0; 
	const double onybyf =  dh.f > 0.0 ?  1.0 / sqrt(dh.f) : 0.0; 
	
	cost += dot(dh.delta, dh.delta); 

	C3DFMatrix src_grad = get_gradient(irsrc, nx, nxy); 
	
	const double p1 = dh.cos_a * onebydotss; 
	const double p2 =  onybyf; 
	return C3DFVector( dot(dh.delta, (p1 * dot(*irsrc, src_grad.x)  - p2 * dot(ref, src_grad.x)) * *irsrc - dh.cos_a * src_grad.x), 
			   dot(dh.delta, (p1 * dot(*irsrc, src_grad.y)  - p2 * dot(ref, src_grad.y)) * *irsrc - dh.cos_a * src_grad.y), 
			   dot(dh.delta, (p1 * dot(*irsrc, src_grad.z)  - p2 * dot(ref, src_grad.z)) * *irsrc - dh.cos_a * src_grad.z)); 
	
}

double FDeltaScalar::get_dot(const C3DFVector& src, const C3DFVector& ref)const 
{
	
}


C3DNFGImageCost::C3DNFGImageCost(PEvaluator evaluator):
	m_evaluator(evaluator)
{
	add(property_gradient);
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
	
	auto ia = ng_a.begin_range(C3DBounds::_0, ng_a.get_size()); 
	auto ie = ng_a.end_range(C3DBounds::_0, ng_a.get_size()); 
	auto ib = m_ng_ref.begin_range(C3DBounds::_0, m_ng_ref.get_size()); 
	auto iforce = force.begin_range(C3DBounds::_0, force.get_size()); 

	while (ia != ie) {
		C3DFVector help = m_evaluator->grad (nx, nxy, ia, *ib, sum);
		*iforce = help; 
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
