/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2010
 *
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PUcRPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#define VSTREAM_DOMAIN "NR-REG"

#include <boost/lambda/lambda.hpp>


#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/transformfactory.hh>
#include <gsl++/multimin.hh>
#include <iomanip>

using boost::lambda::_1; 


NS_MIA_BEGIN

using namespace gsl;
using namespace std;

struct C2DNonrigidRegisterImpl {

	C2DNonrigidRegisterImpl(C2DFullCostList& costs, EMinimizers minimizer,
				P2DTransformationFactory transform_creator,
				const C2DInterpolatorFactory& ipf,  size_t mg_levels);

	P2DTransformation run(P2DImage src, P2DImage ref) const;
private:

	void apply(C2DTransformation& transf, const gsl_multimin_fdfminimizer_type *optimizer)const ;


	void apply(C2DTransformation& transf, const gsl_multimin_fminimizer_type *optimizer)const ;


	C2DFullCostList& _M_costs;
	EMinimizers _M_minimizer;
	C2DInterpolatorFactory _M_ipf;
	P2DTransformationFactory _M_transform_creator;
	size_t _M_mg_levels; 
};

class C2DNonrigRegGradientProblem: public gsl::CFDFMinimizer::Problem {
public:
	C2DNonrigRegGradientProblem(const C2DFullCostList& costs, C2DTransformation& transf,
			      const C2DInterpolatorFactory& _M_ipf);

	void reset_counters(); 
private:
	double  do_f(const DoubleVector& x);
	void    do_df(const DoubleVector& x, DoubleVector&  g);
	double  do_fdf(const DoubleVector& x, DoubleVector&  g);

	P2DImage apply(const DoubleVector& x);
	const C2DFullCostList& _M_costs; 
	C2DTransformation& _M_transf;
	const C2DInterpolatorFactory& _M_ipf;
	size_t _M_func_evals; 
	size_t _M_grad_evals; 
	double _M_start_cost; 
};
typedef shared_ptr<C2DNonrigRegGradientProblem> P2DGradientNonrigregProblem;

C2DNonrigidRegister::C2DNonrigidRegister(C2DFullCostList& costs, EMinimizers minimizer,
					 P2DTransformationFactory transform_creation,
					 const C2DInterpolatorFactory& ipf, size_t mg_levels):
	impl(new C2DNonrigidRegisterImpl( costs, minimizer, transform_creation, ipf, mg_levels))
{
}


C2DNonrigidRegister::~C2DNonrigidRegister()
{
	delete impl;
}

P2DTransformation C2DNonrigidRegister::run(P2DImage src, P2DImage ref) const
{
	return impl->run(src, ref);
}

C2DNonrigidRegisterImpl::C2DNonrigidRegisterImpl(C2DFullCostList& costs, EMinimizers minimizer,
						 P2DTransformationFactory transform_creation, 
						 const C2DInterpolatorFactory& ipf,size_t mg_levels):
	_M_costs(costs),
	_M_minimizer(minimizer),
	_M_ipf(ipf),
	_M_transform_creator(transform_creation), 
	_M_mg_levels(mg_levels)
{
}

static bool minimizer_need_gradient(EMinimizers m)
{
	switch (m) {
	case min_undefined: throw invalid_argument("Try to use minimizer 'undefined'");
	default: return true;
	}
}

struct  UMinimzer{
		const gsl_multimin_fminimizer_type *fmin;
		const gsl_multimin_fdfminimizer_type *fdfmin;
};

UMinimzer gradminimizers[min_undefined] = {
	{ NULL, gsl_multimin_fdfminimizer_conjugate_fr },
	{ NULL, gsl_multimin_fdfminimizer_conjugate_pr },
	{ NULL, gsl_multimin_fdfminimizer_vector_bfgs },
	{ NULL, gsl_multimin_fdfminimizer_vector_bfgs2 },
	{ NULL, gsl_multimin_fdfminimizer_steepest_descent }
};


void C2DNonrigidRegisterImpl::apply(C2DTransformation& transf, 
				    const gsl_multimin_fdfminimizer_type *optimizer)const
{
	if (!_M_costs.has(property_gradient))
		throw invalid_argument("requested optimizer needs gradient, but cost functions doesn't prvide one");

	P2DGradientNonrigregProblem gp(new C2DNonrigRegGradientProblem( _M_costs, transf, _M_ipf));
	CFDFMinimizer minimizer(gp, optimizer );

	auto x = transf.get_parameters();
	cvinfo() << "Start Registration of " << x.size() <<  " parameters\n"; 
	minimizer.run(x);
	transf.set_parameters(x);
	cvmsg() << "\n"; 
}

/*
  This filter could be replaced by a histogram equalizing filter 
*/

class FScaleFilterCreator: public TFilter<C2DFilterPluginHandler::ProductPtr> {
public: 
	template <typename T, typename S>
	result_type operator ()(const T2DImage<T>& a, const T2DImage<S>& b) const {
		double sum = 0.0; 
		double sum2 = 0.0; 
		int n = 2 * a.size(); 

		for(auto ia = a.begin(), ib = b.begin(); ia != a.end(); ++ia, ++ib) {
			sum += *ia + *ib; 
			sum2 += *ia * *ia + *ib * *ib;
		}
		
		double mean = sum / n; 
		double sigma = sqrt((sum2 - sum * sum / n) / (n - 1));

		// both images are of the same single color 
		if (sigma == 0.0) 
			return result_type(); 

		// I want a conversion filter, that makes the images together zero mean 
		// and diversion 1
		stringstream filter_descr; 
		filter_descr << "convert:repn=float,map=linear,b=" << -mean << ",a=" << 1.0/sigma; 
			
		return C2DFilterPluginHandler::instance().produce(filter_descr.str()); 
		
	}; 
	
}; 


P2DTransformation C2DNonrigidRegisterImpl::run(P2DImage src, P2DImage ref) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	if (!minimizer_need_gradient(_M_minimizer))
		throw invalid_argument("Non-gradient based optimization not supported\n"); 

	P2DTransformation transform;

	// convert the images to float ans scale to range [-1,1]
	FScaleFilterCreator fc; 
	auto tofloat_converter = ::mia::filter(fc, *src, *ref); 

	src = tofloat_converter->filter(*src); 
	ref = tofloat_converter->filter(*ref); 

	C2DBounds global_size = src->get_size();

	int shift = _M_mg_levels;

	do {
		if (shift)
			shift--;

		C2DBounds BlockSize(1 << shift, 1 << shift);
		cvinfo() << "Blocksize = " << BlockSize.x << "x"<< BlockSize.y << "\n";

		stringstream downscale_descr;
		downscale_descr << "downscale:bx=" << BlockSize.x << ",by=" << BlockSize.y;
		C2DFilterPlugin::ProductPtr downscaler =
			C2DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());

		P2DImage src_scaled = shift ? downscaler->filter(*src) : src;
		P2DImage ref_scaled = shift ? downscaler->filter(*ref) : ref;

		if (transform)
			transform = transform->upscale(src_scaled->get_size());
		else
			transform = _M_transform_creator->create(src_scaled->get_size());

		cvinfo() << "register at " << src_scaled->get_size() << "\n";

		save_image2d("src.@", src_scaled);
		save_image2d("ref.@", ref_scaled);
		_M_costs.reinit(); 
		_M_costs.set_size(src_scaled->get_size()); 
		
		apply(*transform, gradminimizers[_M_minimizer].fdfmin);

		// run the registration at refined splines 
		if (transform->refine())
			apply(*transform, gradminimizers[_M_minimizer].fdfmin);

		//auto params = transform->get_parameters(); 
	} while (shift); 
	return transform;
}

C2DNonrigRegGradientProblem::C2DNonrigRegGradientProblem(const C2DFullCostList& costs, 
							 C2DTransformation& transf, 
							 const C2DInterpolatorFactory& ipf):
	gsl::CFDFMinimizer::Problem(transf.degrees_of_freedom()),
	_M_costs(costs),
	_M_transf(transf),
	_M_ipf(ipf), 
	_M_func_evals(0),
	_M_grad_evals(0), 
	_M_start_cost(0.0)
{

}

void C2DNonrigRegGradientProblem::reset_counters()
{
	_M_func_evals = _M_grad_evals = 0; 
}

double  C2DNonrigRegGradientProblem::do_f(const DoubleVector& x)
{
       

	_M_transf.set_parameters(x);
	double result = _M_costs.cost_value(_M_transf);
	if (!_M_func_evals && !_M_grad_evals) 
		_M_start_cost = result; 
	
	_M_func_evals++; 
	cvmsg() << "Cost[fg="<<setw(4)<<_M_grad_evals 
		<< ",fe="<<setw(4)<<_M_func_evals<<"]=" 
		<< setw(20) << setprecision(12) << result 
		<< "ratio:" << setw(20) << setprecision(12) 
		<< result / _M_start_cost <<   "\r"; 
	cvinfo() << "\n";
	return result; 
}

void    C2DNonrigRegGradientProblem::do_df(const DoubleVector& x, DoubleVector&  g)
{
	do_fdf(x,g); 
}

double  C2DNonrigRegGradientProblem::do_fdf(const DoubleVector& x, DoubleVector&  g)
{

	_M_transf.set_parameters(x);
	fill(g.begin(), g.end(), 0.0); 
	double result = _M_costs.evaluate(_M_transf, g);

	if (!_M_func_evals && !_M_grad_evals) 
		_M_start_cost = result; 

	_M_grad_evals++; 

	cvmsg() << "Cost[fg="<<setw(4)<<_M_grad_evals 
		<< ",fe="<<setw(4)<<_M_func_evals<<"]=" 
		<< setw(20) << setprecision(12) << result 
		<< "ratio:" << setw(20) << setprecision(12) << result / _M_start_cost <<  "\r"; 
	cvinfo() << "\n"; 
	return result; 
}

NS_MIA_END
