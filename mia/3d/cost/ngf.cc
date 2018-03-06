/* -*- mia-c++  -*-
 *
 * This file is part of MIA - a toolbox for medical image analysis
 * Copyright (c) Leipzig, Madrid 1999-2017 Gert Wollny
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

#include <mia/3d/cost/ngf.hh>
#include <mia/3d/nfg.hh>

#include <numeric>

NS_BEGIN(ngf_3dimage_cost)

using namespace std;
using namespace boost;
using namespace mia;


double FEvaluator::operator()(const mia::C3DFVector& src, const mia::C3DFVector& ref) const
{
       return cost(src, ref);
}

C3DFMatrix FEvaluator::get_gradient(field_range_iterator& irsrc, int nx, int nxy) const
{
       cvdebug() << irsrc.get_boundary_flags() << "\n";
       C3DFMatrix result;
       C3DFVectorfield::const_iterator isrc = irsrc.get_point();

       if (! (irsrc.get_boundary_flags() & field_range_iterator::eb_x))
              result.x = 0.5 * (isrc[1] - isrc[-1]);

       if (! (irsrc.get_boundary_flags() & field_range_iterator::eb_y))
              result.y = 0.5 * (isrc[nx] - isrc[-nx]);

       if (! (irsrc.get_boundary_flags() & field_range_iterator::eb_z))
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



C3DFVector  FScalar::grad (int nx, int nxy, field_range_iterator& irsrc,
                           const C3DFVector& ref, double& cost) const
{
       double d = dot(*irsrc, ref);
       cost -= d * d;
       return - d * (get_gradient(irsrc, nx, nxy) * ref);
}

double FCross::cost(const C3DFVector& src, const C3DFVector& ref) const
{
       C3DFVector d = cross(src, ref);
       return d.norm2();
}

C3DFVector  FCross::grad (int nx, int nxy, field_range_iterator& irsrc,
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

C3DFVector FDeltaScalar::grad (int nx, int nxy, field_range_iterator& irsrc,
                               const C3DFVector& ref, double& cost) const
{
       DotHelper dh(*irsrc, ref);
       const double onebydotss = dh.dotss > 0 ? 1.0 / dh.dotss : 0.0;
       const double onybyf =  dh.f > 0.0 ?  1.0 / sqrt(dh.f) : 0.0;
       cost += dot(dh.delta, dh.delta);
       C3DFMatrix src_grad = get_gradient(irsrc, nx, nxy);
       const double p1 = dh.cos_a * onebydotss;
       const double p2 =  onybyf;
       return C3DFVector( dot(dh.delta, (p1 * dot(*irsrc, src_grad.x)  - p2 * dot(ref, src_grad.x)) * *irsrc - dh.cos_a * src_grad.x),
                          dot(dh.delta, (p1 * dot(*irsrc, src_grad.y)  - p2 * dot(ref, src_grad.y)) * *irsrc - dh.cos_a * src_grad.y),
                          dot(dh.delta, (p1 * dot(*irsrc, src_grad.z)  - p2 * dot(ref, src_grad.z)) * *irsrc - dh.cos_a * src_grad.z));
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
       [](double x, double y) {
              return x + y;
       },
       [this](const C3DFVector ix, const C3DFVector iy) {
              return m_evaluator->cost(ix, iy);
       });
       return 0.5 * sum;
}

double C3DNFGImageCost::do_evaluate_force(const mia::C3DImage& a,
              const mia::C3DImage& /*b*/,
              mia::C3DFVectorfield& force) const
{
       const C3DFVectorfield ng_a = get_nfg(a);
       double sum = 0.0;
       const int nx = ng_a.get_size().x;
       const int nxy = nx * ng_a.get_size().y;
       auto  ia = ng_a.begin_range(C3DBounds::_0, ng_a.get_size()).with_boundary_flag();
       auto  ie = ng_a.end_range(C3DBounds::_0, ng_a.get_size()).with_boundary_flag();
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

const TDictMap<C3DNFGImageCostPlugin::ESubTypes>::Table lut[] = {
       {"ds", C3DNFGImageCostPlugin::st_delta_scalar, "square of scaled difference"},
       {"dot", C3DNFGImageCostPlugin::st_scalar, "scalar product kernel"},
       {"cross", C3DNFGImageCostPlugin::st_cross, "cross product kernel"},
       {0, C3DNFGImageCostPlugin::st_unknown, ""}
};
const TDictMap<C3DNFGImageCostPlugin::ESubTypes> subtypemap(lut);

C3DNFGImageCostPlugin::C3DNFGImageCostPlugin():
       C3DImageCostPlugin("ngf"),
       m_kernel(st_delta_scalar)
{
       TRACE("C3DNFGImageCostPlugin::C3DNFGImageCostPlugin()");
       add_parameter("eval",
                     new CDictParameter<ESubTypes>(m_kernel, subtypemap, "plugin subtype (sq, ds,dot,cross)"));
}


C3DImageCost *C3DNFGImageCostPlugin::do_create()const
{
       TRACE("C3DNFGImageCostPlugin::do_create");
       PEvaluator eval;

       switch (m_kernel) {
       case st_delta_scalar:
              eval.reset(new FDeltaScalar());
              break;

       case st_scalar:
              eval.reset(new FScalar());
              break;

       case st_cross:
              eval.reset(new FCross());
              break;

       default:
              throw invalid_argument(string("C3DNFGImageCostPlugin: unknown cost sub-type '")
                                     + subtypemap.get_name(m_kernel) + "'");
       }

       return new C3DNFGImageCost(eval);
}

const string C3DNFGImageCostPlugin::do_get_descr()const
{
       return "This function evaluates the image similarity based on normalized gradient "
              "fields. Given normalized gradient fields $\n_S$ of the src image and $\n_R$ "
              "of the ref image various evaluators are implemented.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new C3DNFGImageCostPlugin();
}

NS_END
