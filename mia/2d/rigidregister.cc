/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 1999-2012 Gert Wollny
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

#define VSTREAM_DOMAIN "rigidreg"

#include <mia/2d/rigidregister.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/filter.hh>


NS_MIA_BEGIN

using namespace std;

struct C2DRigidRegisterImpl {

	C2DRigidRegisterImpl(P2DImageCost cost, PMinimizer minimizer,
			     P2DTransformationFactory transform_creator,  size_t mg_levels);

	P2DTransformation run(P2DImage src, P2DImage ref) const;
private:

	P2DImageCost m_cost;
	PMinimizer m_minimizer;
	P2DTransformationFactory m_transform_creator; 
	size_t m_mg_levels; 
};

class C2DRegGradientProblem: public CMinimizer::Problem {
public:
	C2DRegGradientProblem(const C2DImage& model, C2DTransformation& transf, const C2DImageCost& m_cost);
private:
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
protected: 
	P2DImage apply(const CDoubleVector& x); 
	double  do_f(const CDoubleVector& x);
	size_t do_size() const; 

	const C2DImage& m_model;
	C2DTransformation& m_transf;
	const C2DImageCost& m_cost;
};
typedef shared_ptr<C2DRegGradientProblem> P2DGradientProblem;


class C2DRegFakeGradientProblem: public C2DRegGradientProblem {
public:
	C2DRegFakeGradientProblem(const C2DImage& model, C2DTransformation& transf,
			 const C2DImageCost& m_cost);
private:
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);

};

class C2DRegProblem: public CMinimizer::Problem {
public:
	C2DRegProblem(const C2DImage& model, C2DTransformation& transf,
			 const C2DImageCost& m_cost);
private:
	double  do_f(const CDoubleVector& x);
	void    do_df(const CDoubleVector& x, CDoubleVector&  g);
	double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
	size_t do_size() const; 

	const C2DImage& m_model;
	C2DTransformation& m_transf;
	const C2DImageCost& m_cost;
};
typedef shared_ptr<C2DRegProblem> P2DRegProblem;


C2DRigidRegister::C2DRigidRegister(P2DImageCost cost, PMinimizer minimizer,
				   P2DTransformationFactory transform_creator,size_t mg_levels):
	impl(new C2DRigidRegisterImpl( cost, minimizer, transform_creator, mg_levels))
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
					   size_t mg_levels):
	m_cost(cost),
	m_minimizer(minimizer),
	m_transform_creator(transform_creator), 
	m_mg_levels(mg_levels)
{
}

P2DTransformation C2DRigidRegisterImpl::run(P2DImage src, P2DImage ref) const
{
	assert(src);
	assert(ref);
	assert(src->get_size() == ref->get_size());

	P2DTransformation transform;

	int x_shift = m_mg_levels + 1;
	int y_shift = m_mg_levels + 1;

	while (x_shift && y_shift) {
		if (x_shift)
			x_shift--;

		if (y_shift)
			y_shift--;

		C2DBounds BlockSize(1 << x_shift, 1 << y_shift);
		cvinfo() << "Blocksize = " << BlockSize.x << "x"<< BlockSize.y << "\n";

		stringstream downscale_descr;
		downscale_descr << "downscale:bx=" << BlockSize.x << ",by=" << BlockSize.y;
		auto downscaler = C2DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());

		P2DImage src_scaled = x_shift && y_shift ? downscaler->filter(*src) : src;
		P2DImage ref_scaled = x_shift && y_shift ? downscaler->filter(*ref) : ref;

		m_cost->set_reference(*ref_scaled); 
		if (transform)
			transform = transform->upscale(src_scaled->get_size());
		else
			transform = m_transform_creator->create(src_scaled->get_size());

		cvmsg() << "register at " << src_scaled->get_size() << "\n";


		CMinimizer::PProblem gp = m_minimizer->has(property_gradient)? 
			CMinimizer::PProblem(new C2DRegFakeGradientProblem(*src_scaled, *transform, *m_cost)):
			CMinimizer::PProblem(new C2DRegProblem(*src_scaled, *transform, *m_cost)); 

		m_minimizer->set_problem(gp); 

		auto x = transform->get_parameters();
		m_minimizer->run(x);
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
				   const C2DImageCost& cost):
	m_model(model),
	m_transf(transf),
	m_cost(cost)
{
	add(property_gradient); 
}

size_t C2DRegGradientProblem::do_size() const
{
	return m_transf.degrees_of_freedom(); 
}

P2DImage C2DRegGradientProblem::apply(const CDoubleVector& x)
{
	cvinfo() << "\nParams:";
	for (auto i = x.begin(); i != x.end(); ++i) 
		cverb << *i << " "; 
	cverb << "\n"; 

	m_transf.set_parameters(x);
	return m_transf(m_model);
}

double  C2DRegGradientProblem::do_f(const CDoubleVector& x)
{
	P2DImage temp = apply(x);
	const double value = m_cost.value(*temp);
	cvmsg() << "\rCost = " << value;
	return value;
}

void    C2DRegGradientProblem::do_df(const CDoubleVector& x, CDoubleVector&  g)
{
	P2DImage temp = apply(x);

	C2DFVectorfield gradient(m_model.get_size());
	m_cost.evaluate_force(*temp, -1.0, gradient);
	m_transf.translate(gradient, g);
}

double  C2DRegGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	P2DImage temp = apply(x);
	C2DFVectorfield gradient(m_model.get_size());
	double result = m_cost.evaluate_force(*temp, -1.0, gradient);
	m_transf.translate(gradient, g);
	return result;
}

C2DRegFakeGradientProblem::C2DRegFakeGradientProblem(const C2DImage& model,
						     C2DTransformation& transf, const C2DImageCost& m_cost):
	C2DRegGradientProblem(model, transf,  m_cost)
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
		cvdebug() << "g[" << i << "] = " << g[i] << "\n"; 
	}
}

double  C2DRegFakeGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
	double cost_value = do_f(x); 
	do_df(x,g); 
	return cost_value; 
}

C2DRegProblem::C2DRegProblem(const C2DImage& model, C2DTransformation& transf,
	    const C2DImageCost& cost):
	m_model(model),
	m_transf(transf),
	m_cost(cost)
{
}

double  C2DRegProblem::do_f(const CDoubleVector& x)
{
	cvinfo() << "\nParams:";
	for (auto i = x.begin(); i != x.end(); ++i) 
		cverb << *i << " "; 
	cverb << "\n"; 

	m_transf.set_parameters(x);
	P2DImage test =  m_transf(m_model);

	const double value = m_cost.value(*test);
	cvmsg() << "Cost = " << value << "\n";
	return value;
}

void   C2DRegProblem::do_df(const CDoubleVector& , CDoubleVector&  )
{
	assert(0 && "C2DRegProblem::do_df must not be called from a gradient free minimizer"); 
}

double  C2DRegProblem::do_fdf(const CDoubleVector& , CDoubleVector&  )
{
	assert(0 && "C2DRegProblem::do_fdf must not be called from a gradient free minimizer"); 
}

size_t C2DRegProblem::do_size() const
{
	return m_transf.degrees_of_freedom(); 
}


NS_MIA_END
