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
#include <mia/core/filter.hh>


NS_MIA_BEGIN

using namespace std;

struct C2DRigidRegisterImpl {

	C2DRigidRegisterImpl(P2DImageCost cost, PMinimizer minimizer,
			     P2DTransformationFactory transform_creator,
			     const C2DInterpolatorFactory& ipf,  size_t mg_levels);

	P2DTransformation run(P2DImage src, P2DImage ref) const;
private:

	P2DImageCost _M_cost;
	PMinimizer _M_minimizer;
	C2DInterpolatorFactory _M_ipf;
	P2DTransformationFactory _M_transform_creator; 
	size_t _M_mg_levels; 
};

class C2DRegGradientProblem: public CMinimizer::Problem {
public:
	C2DRegGradientProblem(const C2DImage& model, C2DTransformation& transf,
			 const C2DImageCost& _M_cost, const C2DInterpolatorFactory& _M_ipf);
private:
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
protected: 
	P2DImage apply(const CDoubleVector& x); 
	double  do_f(const CDoubleVector& x);
	size_t do_size() const; 

	const C2DImage& _M_model;
	C2DTransformation& _M_transf;
	const C2DImageCost& _M_cost;
	const C2DInterpolatorFactory& _M_ipf;
};
typedef shared_ptr<C2DRegGradientProblem> P2DGradientProblem;


class C2DRegFakeGradientProblem: public C2DRegGradientProblem {
public:
	C2DRegFakeGradientProblem(const C2DImage& model, C2DTransformation& transf,
			 const C2DImageCost& _M_cost, const C2DInterpolatorFactory& _M_ipf);
private:
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);

};

class C2DRegProblem: public CMinimizer::Problem {
public:
	C2DRegProblem(const C2DImage& model, C2DTransformation& transf,
			 const C2DImageCost& _M_cost, const C2DInterpolatorFactory& _M_ipf);
private:
	double  do_f(const CDoubleVector& x);
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
	size_t do_size() const; 

	const C2DImage& _M_model;
	C2DTransformation& _M_transf;
	const C2DImageCost& _M_cost;
	const C2DInterpolatorFactory& _M_ipf;
};
typedef shared_ptr<C2DRegProblem> P2DRegProblem;


C2DRigidRegister::C2DRigidRegister(P2DImageCost cost, PMinimizer minimizer,
				   P2DTransformationFactory transform_creator,
				   const C2DInterpolatorFactory& ipf, size_t mg_levels):
	impl(new C2DRigidRegisterImpl( cost, minimizer, transform_creator, ipf, mg_levels))
{
}


C2DRigidRegister::~C2DRigidRegister()
{
	delete impl;
}

P2DTransformation C2DRigidRegister::run(P2DImage src, P2DImage ref) const
{
	return impl->run(src, ref);
}

C2DRigidRegisterImpl::C2DRigidRegisterImpl(P2DImageCost cost, PMinimizer minimizer,
					   P2DTransformationFactory transform_creator,
					   const C2DInterpolatorFactory& ipf,  
					   size_t mg_levels):
	_M_cost(cost),
	_M_minimizer(minimizer),
	_M_ipf(ipf),
	_M_transform_creator(transform_creator), 
	_M_mg_levels(mg_levels)
{
}

P2DTransformation C2DRigidRegisterImpl::run(P2DImage src, P2DImage ref) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	P2DTransformation transform;

	C2DBounds global_size = src->get_size();

	int x_shift = _M_mg_levels + 1;
	int y_shift = _M_mg_levels + 1;

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

		_M_cost->set_reference(*ref_scaled); 
		if (transform)
			transform = transform->upscale(src_scaled->get_size());
		else
			transform = _M_transform_creator->create(src_scaled->get_size());

		cvmsg() << "register at " << src_scaled->get_size() << "\n";


		CMinimizer::PProblem gp = _M_minimizer->has(property_gradient)? 
			CMinimizer::PProblem(new C2DRegFakeGradientProblem(*src_scaled, 
									   *transform, *_M_cost, _M_ipf)):
			CMinimizer::PProblem(new C2DRegProblem(*src_scaled, *transform, 
							       *_M_cost, _M_ipf)); 

		_M_minimizer->set_problem(gp); 

		auto x = transform->get_parameters();
		_M_minimizer->run(x);
		transform->set_parameters(x);

		auto params = transform->get_parameters(); 
		cvinfo() << "\nParams:";
		for (auto i = params.begin(); i != params.end(); ++i) 
			cverb << *i << " "; 
		cverb << "\n"; 
	}
	return transform;
}

C2DRegGradientProblem::C2DRegGradientProblem(const C2DImage& model, C2DTransformation& transf,
				   const C2DImageCost& cost, const C2DInterpolatorFactory& ipf):
	_M_model(model),
	_M_transf(transf),
	_M_cost(cost),
	_M_ipf(ipf)
{
	add(property_gradient); 
}

size_t C2DRegGradientProblem::do_size() const
{
	return _M_transf.degrees_of_freedom(); 
}

P2DImage C2DRegGradientProblem::apply(const CDoubleVector& x)
{
	cvinfo() << "\nParams:";
	for (auto i = x.begin(); i != x.end(); ++i) 
		cverb << *i << " "; 
	cverb << "\n"; 

	_M_transf.set_parameters(x);
	return _M_transf(_M_model, _M_ipf);
}

double  C2DRegGradientProblem::do_f(const CDoubleVector& x)
{
	P2DImage temp = apply(x);
	const double value = _M_cost.value(*temp);
	cvmsg() << "\rCost = " << value;
	return value;
}

void    C2DRegGradientProblem::do_df(const CDoubleVector& x, CDoubleVector&  g)
{
	P2DImage temp = apply(x);

	C2DFVectorfield gradient(_M_model.get_size());
	_M_cost.evaluate_force(*temp, -1.0, gradient);
	_M_transf.translate(gradient, g);
}

double  C2DRegGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	P2DImage temp = apply(x);
	C2DFVectorfield gradient(_M_model.get_size());
	double result = _M_cost.evaluate_force(*temp, -1.0, gradient);
	_M_transf.translate(gradient, g);
	return result;
}

C2DRegFakeGradientProblem::C2DRegFakeGradientProblem(const C2DImage& model,
						     C2DTransformation& transf, const C2DImageCost& _M_cost, 
						     const C2DInterpolatorFactory& _M_ipf):
	C2DRegGradientProblem(model, transf,  _M_cost,  _M_ipf)
{
	add(property_gradient); 
}

void    C2DRegFakeGradientProblem::do_df(const CDoubleVector& x, CDoubleVector&  g)
{
	CDoubleVector x_tmp(x.size());
	copy(x.begin(), x.end(), x_tmp.begin()); 
	for (size_t i = 0; i < g.size(); ++i) {
		x_tmp[i] += 0.01; 
		double cost_p = do_f(x_tmp); 
		x_tmp[i] -= 0.02; 
		double cost_m = do_f(x_tmp); 
		x_tmp[i] += 0.01;
		g[i] = (cost_p - cost_m) * 50.0; 
		cvinfo() << "g[" << i << "] = " << g[i] << "\n"; 
	}
}

double  C2DRegFakeGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	double cost_value = do_f(x); 
	do_df(x,g); 
	return cost_value; 
}

C2DRegProblem::C2DRegProblem(const C2DImage& model, C2DTransformation& transf,
	    const C2DImageCost& cost, const C2DInterpolatorFactory& ipf):
	_M_model(model),
	_M_transf(transf),
	_M_cost(cost),
	_M_ipf(ipf)
{
}

double  C2DRegProblem::do_f(const CDoubleVector& x)
{
	cvinfo() << "\nParams:";
	for (auto i = x.begin(); i != x.end(); ++i) 
		cverb << *i << " "; 
	cverb << "\n"; 

	_M_transf.set_parameters(x);
	P2DImage test =  _M_transf(_M_model, _M_ipf);

	const double value = _M_cost.value(*test);
	cvmsg() << "Cost = " << value << "\n";
	return value;
}

void   C2DRegProblem::do_df(const CDoubleVector& , CDoubleVector&  )
{
	assert(0 && "C2DRegProblem::do_df should not be called from a gradient free minimizer"); 
}

double  C2DRegProblem::do_fdf(const CDoubleVector& , CDoubleVector&  )
{
	assert(0 && "C2DRegProblem::do_fdf should not be called from a gradient free minimizer"); 
}

size_t C2DRegProblem::do_size() const
{
	return _M_transf.degrees_of_freedom(); 
}


NS_MIA_END
