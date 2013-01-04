/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2013 Gert Wollny
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
   
   \subsection{Normalized Gradient Fields}
   \label{cost2d:ngf}
   
   \begin{description}
   
   \item [Plugin:] ngf
   \item [Description:] This function evaluates the image similarity based on normalized gradient 
                        fields. Given normalized gradient fields $\n_S$ of the study image and $\n_R$
			of the reference image various evaluators are implemented: 
   \begin{itemize}
   \item cross Cross product based formulation: 
     \begin{equation}
       F_{\text{ngf}, \times}(\n_S, \n_R) := \frac{1}{2}\int_{\Omega} \left( \n_S(x) \times \n_R(x) \right)^2 \text{d}x
     \end{equation}
   \item dot Dot product based formulation: 
     \begin{equation}
        F_{\text{ngf}, \cdot}(\n_S, \n_R) := \frac{1}{2}\int_{\Omega} \left( \n_S(x) \cdot \n_R(x) \right)^2 \text{d}x
      \end{equation}
   \item sq 
     \begin{equation}
   	F_{\text{ngf}, \Delta^2} := \frac{1}{2}  \int_\Omega  
	\left(<\n_R(x),\n_S(x)>^2   - \| \n_R(x)\|^2 \right)^2  \text{d}x,
     \end{equation}
   \item ds 
     \begin{equation}
   	F_{\text{ngf}, \cdot\Delta} := \frac{1}{2}  \int_\Omega  
	\left( \| \n_R(x)\|^2 -
        \frac{<\n_R(x),\n_S(x)>^2}{\|\n_R(x)\|\|\n_S(x)\|} \right)^2  \text{d}x,
      \end{equation}
   \end{itemize}

   \item [Study:] An abitrary gray scale or binary images 
   \item [Reference:] An abitrary gray scale or binary images 
   
   \end{description}
   
   \plugtabstart
   eval &  strimng & Evaluator (cross|dot|ds|sq) & ds  \\
   \plugtabend

   For further information see \cite{haber05, wollny08a, wollny10b}. 

 */


#include <mia/2d/cost/ngf.hh>
#include <mia/core/property_flags.hh>

using namespace std;
using namespace boost;
using namespace mia;

NS_BEGIN(nfg_2dimage_cost)


const  char *CCostEvaluatorCross::do_get_name() const
{
	return "nfg-x";
}

float CCostEvaluatorCross::get_cost(size_t x,param_pass& p) const
{
	float help = cross(p.ref[x], p.src[x]);
	return help * help;

}

float CCostEvaluatorCross::get_cost_grad(size_t x,param_pass& p, C2DFVectorfield::iterator g)const
{
	float help = cross(p.ref[x], p.src[x]);
	g[x].x = help * cross(p.ref[x], (p.src[x+1] - p.src[x-1]));
	g[x].y = help * cross(p.ref[x], (p.srcp[x]  - p.srcm[x]));
	return help * help;
}


const  char *CCostEvaluatorScalar::do_get_name() const{
	return "nfg-s";
}

float CCostEvaluatorScalar::get_cost(size_t x,param_pass& p) const {
	float  help = dot(p.ref[x], p.src[x]);
	return - help * help;
}

float CCostEvaluatorScalar::get_cost_grad(size_t x,param_pass& p, C2DFVectorfield::iterator g) const
{
	float  help = -dot(p.ref[x], p.src[x]);
	g[x].x = help * dot( p.ref[x], (p.src[x+1] - p.src[x-1]));
	g[x].y = help * dot( p.ref[x], (p.srcp[x]  - p.srcm[x]));
	return - help * help;
}

const  char *CCostEvaluatorSQDelta::do_get_name() const
{
	return "nfgsq";
}

float CCostEvaluatorSQDelta::get_cost(size_t x,param_pass& p) const
{
	float help = get_help_value(p.ref[x], dot(p.ref[x], p.src[x]));
	return 0.5 * scale * help * help;
}

float CCostEvaluatorSQDelta::get_cost_grad(size_t x,param_pass& p, C2DFVectorfield::iterator g)const
{
	float dot_rs = dot(p.ref[x], p.src[x]);
	float help =  get_help_value(p.ref[x], dot_rs);
	float factor = 2.0 * dot_rs *scale * help;

	g[x].x = factor * dot( p.ref[x], (p.src[x+1] - p.src[x-1]));
	g[x].y = factor * dot( p.ref[x], (p.srcp[x]  - p.srcm[x]));

	return 0.5 * scale * help * help;
}

float CCostEvaluatorSQDelta::get_help_value(const C2DFVector& ref, float dot_rs) const
{
	float rnorm  = ref.norm2();
	return dot_rs * dot_rs - rnorm * rnorm;
}

const float CCostEvaluatorSQDelta::scale = 1.0;


const  char *CCostEvaluatorDeltaScalar::do_get_name() const
{
	return "ngf-d";
}

float CCostEvaluatorDeltaScalar::get_cost(size_t x,param_pass& p) const
{
	const float dot_rs = dot(p.ref[x], p.src[x]);
	const float ref_norm2 = p.ref[x].norm2();
	const float f = ref_norm2 * p.src[x].norm2();
	const float weight = f > 0.0 ? dot_rs / sqrt(f) : 0.0;
	const float help = ref_norm2 - weight * dot_rs;
	return 0.5 * scale * help * help;
}

float CCostEvaluatorDeltaScalar::get_cost_grad(size_t x,param_pass& p, mia::C2DFVectorfield::iterator g)const
{

	const C2DFVector& ref = p.ref[x];
	const C2DFVector& src = p.src[x];

	const float ref_norm2 = ref.norm2();
	const float src_norm2 = src.norm2(); ;
	const float dot_rs = dot(ref, src);

	const float f= ref_norm2 * src_norm2;
	const float cxy = f != 0.0f ? dot_rs/ ( sqrt(f)) : 0.0f;
	const float dotrsbyrr = src_norm2 != 0.0f ? dot_rs / src_norm2 : 0.0f;
	const float helper0 = ref_norm2 - dot_rs * cxy;

	// this 0.5 comes in because beloe in the gradient evaluation we don't do it
	const float helper = 0.5 * helper0 * cxy;

	const C2DFVector dsdx = p.src[x+1] - p.src[x-1];
	const C2DFVector dsdy = p.srcp[x] - p.srcm[x];

	g[x].x = -scale * helper * ( 2 * dot(ref, dsdx ) - dotrsbyrr * dot(src, dsdx));
	g[x].y = -scale * helper * ( 2 * dot(ref, dsdy ) - dotrsbyrr * dot(src, dsdy));

	return 0.5 * scale * helper0 * helper0;
}
const float CCostEvaluatorDeltaScalar::scale = 1.0;

struct FGetMinMax : public TFilter<float> {
	template <typename T>
	float operator ()( const T2DImage<T>& image) const {
		pair<typename T2DImage<T>::const_iterator, typename T2DImage<T>::const_iterator>
			mm = ::boost::minmax_element(image.begin(), image.end());
		return *mm.second - *mm.first;
	}
};


C2DNFGImageCost::C2DNFGImageCost(PEvaluator evaluator):
	m_evaluator(evaluator)
{
	add(property_gradient);
}

void C2DNFGImageCost::prepare_reference(const C2DImage& ref)
{
	post_set_reference(ref); 
}

void C2DNFGImageCost::post_set_reference(const mia::C2DImage& ref)
{
	m_ng_ref =  get_nfg(ref);
}

double C2DNFGImageCost::do_value(const mia::C2DImage& a, const mia::C2DImage& /*b*/) const
{
	TRACE("CNFG2DImageCost::do_value");

	CCostEvaluator::param_pass pp;
	double sum = 0.0;

	C2DFVectorfield ng_a = get_nfg(a);
	for (size_t y = 1; y < ng_a.get_size().y - 1; ++y) {
		pp.ref = m_ng_ref.begin_at(0,y);
		pp.src = ng_a.begin_at(0,y);
		for (size_t x = 1; x < ng_a.get_size().x - 1; ++x) {
			sum +=  m_evaluator->get_cost(x, pp);
		}
	}
	return 0.5 * sum / ng_a.size();
}

double C2DNFGImageCost::do_evaluate_force(const mia::C2DImage& a, 
					  const mia::C2DImage& /*b*/, 
					  float /*scale*/,
					  mia::C2DFVectorfield& force) const
{
	TRACE("CNFG2DImageCost::do_evaluate_force");
	double sum = 0.0;
	C2DFVectorfield ng_a = get_nfg(a);

	const size_t nx = m_ng_ref.get_size().x;
	const size_t ny = m_ng_ref.get_size().y;

	assert(m_ng_ref.get_size() == ng_a.get_size()); 
	assert(m_ng_ref.get_size() == a.get_size()); 

	CCostEvaluator::param_pass pp;

	pp.src = ng_a.begin() + nx;
	pp.srcp = ng_a.begin() + 2 * nx;
	pp.srcm = ng_a.begin();
	pp.ref = m_ng_ref.begin() + nx;

	C2DFVectorfield::iterator iforce = force.begin() + nx;

	for (size_t y = 1; y < ny - 1;
	     ++y, pp.src += nx, pp.srcm += nx, pp.srcp += nx,
		     iforce += nx, pp.ref += nx) {
		for (size_t x = 1; x < nx - 1; ++x) {
			sum +=  m_evaluator->get_cost_grad(x, pp, iforce);
		}
	}
	return 0.5 * sum / ng_a.size();
}

C2DNFGImageCostPlugin::C2DNFGImageCostPlugin():
	C2DImageCostPlugin("ngf"),
	m_kernel("ds")
{
	TRACE("C2DNFGImageCostPlugin::C2DNFGImageCostPlugin()");
	add_parameter("eval",
		      new CStringParameter(m_kernel, false, "plugin subtype (sq, ds,dot,cross)"));

}

enum ESubTypes {st_unknown, st_delta, st_delta_scalar, st_scalar, st_cross};

C2DImageCost *C2DNFGImageCostPlugin::do_create()const
{
	TRACE("C2DNFGImageCostPlugin::do_create");

	const TDictMap<ESubTypes>::Table lut[] = {
		{"sq", st_delta, "square of difference"},
		{"ds", st_delta_scalar, "square of scaled difference"},
		{"dot", st_scalar, "scalar product kernel"},
		{"cross", st_cross, "cross product kernel"},
		{0, st_unknown, ""}
	};
	const TDictMap<ESubTypes> subtypemap(lut);

	PEvaluator eval;
	switch (subtypemap.get_value(m_kernel.c_str())) {
	case st_delta: eval.reset(new CCostEvaluatorSQDelta()); break;
	case st_delta_scalar: eval.reset(new CCostEvaluatorDeltaScalar()); break;
	case st_scalar: eval.reset(new CCostEvaluatorScalar()); break;
	case st_cross: eval.reset(new CCostEvaluatorCross()); break;
	default:
		throw invalid_argument(string("C2DNFGImageCostPlugin: unknown cost sub-type '")
				       +m_kernel+"'");
	}
	return new C2DNFGImageCost(eval);
}

const string C2DNFGImageCostPlugin::do_get_descr()const
{
	return "This function evaluates the image similarity based on normalized gradient fields. "
		"Various evaluation kernels are availabe.";
}


extern "C" EXPORT CPluginBase *get_plugin_interface()
{
	return new C2DNFGImageCostPlugin();
}

NS_END
