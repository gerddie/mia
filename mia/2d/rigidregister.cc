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

#define VSTREAM_DOMAIN "rigidreg"

#include <mia/2d/rigidregister.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/transformfactory.hh>
#include <gsl++/multimin.hh>



NS_MIA_BEGIN

using namespace gsl;
using namespace std;

struct C2DRigidRegisterImpl {

	C2DRigidRegisterImpl(P2DImageCost cost, EMinimizers minimizer,
			     const string& transform_type,
			     const C2DInterpolatorFactory& ipf);

	P2DTransformation run(P2DImage src, P2DImage ref,  size_t mg_levels) const;
private:

	void apply(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
		   const gsl_multimin_fdfminimizer_type *optimizer)const ;


	void apply(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
		   const gsl_multimin_fminimizer_type *optimizer)const ;


	P2DImageCost _M_cost;
	EMinimizers _M_minimizer;
	C2DInterpolatorFactory _M_ipf;
	string _M_transform_type;
};

class C2DRegGradientProblem: public gsl::CFDFMinimizer::Problem {
public:
	C2DRegGradientProblem(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
			 const C2DImageCost& _M_cost, const C2DInterpolatorFactory& _M_ipf);
private:
	double  do_f(const DoubleVector& x);
	void    do_df(const DoubleVector& x, DoubleVector&  g);
	double  do_fdf(const DoubleVector& x, DoubleVector&  g);

	P2DImage apply(const DoubleVector& x);
	const C2DImage& _M_model;
	const C2DImage& _M_reference;
	C2DTransformation& _M_transf;
	const C2DImageCost& _M_cost;
	const C2DInterpolatorFactory& _M_ipf;
};
typedef shared_ptr<C2DRegGradientProblem> P2DGradientProblem;

class C2DRegProblem: public gsl::CFMinimizer::Problem {
public:
	C2DRegProblem(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
			 const C2DImageCost& _M_cost, const C2DInterpolatorFactory& _M_ipf);
private:
	double  do_f(const DoubleVector& x);
	const C2DImage& _M_model;
	const C2DImage& _M_reference;
	C2DTransformation& _M_transf;
	const C2DImageCost& _M_cost;
	const C2DInterpolatorFactory& _M_ipf;
};
typedef shared_ptr<C2DRegProblem> P2DRegProblem;


C2DRigidRegister::C2DRigidRegister(P2DImageCost cost, EMinimizers minimizer,
				   const string& transform_type,
				   const C2DInterpolatorFactory& ipf):
	impl(new C2DRigidRegisterImpl( cost, minimizer, transform_type, ipf))
{
}


C2DRigidRegister::~C2DRigidRegister()
{
	delete impl;
}

P2DTransformation C2DRigidRegister::run(P2DImage src, P2DImage ref,  size_t mg_levels) const
{
	return impl->run(src, ref, mg_levels);
}

C2DRigidRegisterImpl::C2DRigidRegisterImpl(P2DImageCost cost, EMinimizers minimizer,
					   const string& transform_type, const C2DInterpolatorFactory& ipf):
	_M_cost(cost),
	_M_minimizer(minimizer),
	_M_ipf(ipf),
	_M_transform_type(transform_type)
{
}

static bool minimizer_need_gradient(EMinimizers m)
{
	switch (m) {
	case min_nmsimplex: return false;
	case min_undefined: throw invalid_argument("Try to use minimizer 'undefined'");
	default: return true;
	}
}

struct  UMinimzer{
		const gsl_multimin_fminimizer_type *fmin;
		const gsl_multimin_fdfminimizer_type *fdfmin;
};

UMinimzer minimizers[min_undefined] = {
	{ gsl_multimin_fminimizer_nmsimplex2, NULL },
	{ NULL, gsl_multimin_fdfminimizer_conjugate_fr },
	{ NULL, gsl_multimin_fdfminimizer_conjugate_pr },
	{ NULL, gsl_multimin_fdfminimizer_vector_bfgs },
	{ NULL, gsl_multimin_fdfminimizer_vector_bfgs2 },
	{ NULL, gsl_multimin_fdfminimizer_steepest_descent }
};


void C2DRigidRegisterImpl::apply(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
				 const gsl_multimin_fdfminimizer_type *optimizer)const
{
	if (!_M_cost->has(property_gradient))
		throw invalid_argument("requested optimizer needs gradient, but cost functions doesn't prvide one");
	P2DGradientProblem gp(new C2DRegGradientProblem(model, reference, transf, *_M_cost, _M_ipf));
	CFDFMinimizer minimizer(gp, optimizer );

	auto x = transf.get_parameters();
	minimizer.run(x);
	transf.set_parameters(x);
}


void C2DRigidRegisterImpl::apply(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
				 const gsl_multimin_fminimizer_type *optimizer) const
{
	P2DRegProblem gp(new C2DRegProblem(model, reference, transf, *_M_cost, _M_ipf));
	CFMinimizer minimizer(gp, optimizer );

	auto x = transf.get_parameters();
	minimizer.run(x);
	transf.set_parameters(x);

}



P2DTransformation C2DRigidRegisterImpl::run(P2DImage src, P2DImage ref,  size_t mg_levels) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	auto tr_creator = C2DTransformCreatorHandler::instance().produce(_M_transform_type);

	P2DTransformation transform;

	C2DBounds global_size = src->get_size();

	int x_shift = mg_levels + 1;
	int y_shift = mg_levels + 1;

	while (x_shift && y_shift) {
		if (x_shift)
			x_shift--;

		if (y_shift)
			y_shift--;

		C2DBounds BlockSize(1 << x_shift, 1 << y_shift);
		cvinfo() << "Blocksize = " << BlockSize.x << "x"<< BlockSize.y << "\n";

		stringstream downscale_descr;
		downscale_descr << "downscale:bx=" << BlockSize.x << ",by=" << BlockSize.y;
		C2DFilterPlugin::ProductPtr downscaler =
			C2DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());

		P2DImage src_scaled = x_shift && y_shift ? downscaler->filter(*src) : src;
		P2DImage ref_scaled = x_shift && y_shift ? downscaler->filter(*ref) : ref;

		if (transform)
			transform = transform->upscale(src_scaled->get_size());
		else
			transform = tr_creator->create(src_scaled->get_size());

		cvmsg() << "register at " << src_scaled->get_size() << "\n";

		if (minimizer_need_gradient(_M_minimizer))
			apply(*src_scaled, *ref_scaled, *transform, minimizers[_M_minimizer].fdfmin);

		else
			apply(*src_scaled, *ref_scaled, *transform, minimizers[_M_minimizer].fmin);

		auto params = transform->get_parameters(); 
		cvmsg() << "\nParams:";
		for (auto i = params.begin(); i != params.end(); ++i) 
			cverb << *i << " "; 
		cverb << "\n"; 
	}
	return transform;
}

C2DRegGradientProblem::C2DRegGradientProblem(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
				   const C2DImageCost& cost, const C2DInterpolatorFactory& ipf):
	gsl::CFDFMinimizer::Problem(transf.degrees_of_freedom()),
	_M_model(model),
	_M_reference(reference),
	_M_transf(transf),
	_M_cost(cost),
	_M_ipf(ipf)
{

}

P2DImage C2DRegGradientProblem::apply(const DoubleVector& x)
{
	_M_transf.set_parameters(x);
	return _M_transf(_M_model, _M_ipf);
}

double  C2DRegGradientProblem::do_f(const DoubleVector& x)
{
	P2DImage temp = apply(x);
	const double value = _M_cost.value(*temp, _M_reference);
	cvmsg() << "\rCost = " << value;
	return value;
}

void    C2DRegGradientProblem::do_df(const DoubleVector& x, DoubleVector&  g)
{
	P2DImage temp = apply(x);

	C2DFVectorfield gradient(_M_model.get_size());
	_M_cost.evaluate_force(*temp, _M_reference, -1.0, gradient);
	_M_transf.translate(gradient, g);
}

double  C2DRegGradientProblem::do_fdf(const DoubleVector& x, DoubleVector&  g)
{
	P2DImage temp = apply(x);
	C2DFVectorfield gradient(_M_model.get_size());
	_M_cost.evaluate_force(*temp, _M_reference, -1.0, gradient);
	_M_transf.translate(gradient, g);
	return _M_cost.value(*temp, _M_reference);
}


C2DRegProblem::C2DRegProblem(const C2DImage& model, const C2DImage& reference, C2DTransformation& transf,
	    const C2DImageCost& cost, const C2DInterpolatorFactory& ipf):
	gsl::CFMinimizer::Problem(transf.degrees_of_freedom()),
	_M_model(model),
	_M_reference(reference),
	_M_transf(transf),
	_M_cost(cost),
	_M_ipf(ipf)
{

}


double  C2DRegProblem::do_f(const DoubleVector& x)
{
	_M_transf.set_parameters(x);
	P2DImage test =  _M_transf(_M_model, _M_ipf);
	const double value = _M_cost.value(*test, _M_reference);
	cvmsg() << "\rCost = " << value;
	return value;
}



NS_MIA_END
