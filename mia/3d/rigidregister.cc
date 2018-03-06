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

#define VSTREAM_DOMAIN "rigidreg"

#include <mia/3d/rigidregister.hh>
#include <mia/3d/filter.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/core/filter.hh>

NS_MIA_BEGIN

using namespace std;

struct C3DRigidRegisterImpl {

       C3DRigidRegisterImpl(P3DImageCost cost, PMinimizer minimizer,
                            P3DTransformationFactory transform_creator,
                            size_t mg_levels);

       P3DTransformation run(P3DImage src, P3DImage ref) const;
private:


       P3DImageCost m_cost;
       PMinimizer m_minimizer;
       P3DTransformationFactory m_transform_creator;
       size_t m_mg_levels;
};

class C3DRegGradientProblem: public CMinimizer::Problem
{
public:
       C3DRegGradientProblem(const C3DImage& model, C3DTransformation& transf, const C3DImageCost& m_cost);
private:
       void    do_df(const CDoubleVector& x, CDoubleVector&  g);
       double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
protected:
       double  do_f(const CDoubleVector& x);
       size_t do_size() const;
       P3DImage apply(const CDoubleVector& x);
       const C3DImage& m_model;
       C3DTransformation& m_transf;
       const C3DImageCost& m_cost;

       size_t m_geval;
       size_t m_feval;

};
typedef shared_ptr<C3DRegGradientProblem> P3DGradientProblem;


class C3DRegFakeGradientProblem: public C3DRegGradientProblem
{
public:
       C3DRegFakeGradientProblem(const C3DImage& model, C3DTransformation& transf,
                                 const C3DImageCost& m_cost);
private:
       void    do_df(const CDoubleVector& x, CDoubleVector&  g);
       double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);

};

class C3DRegProblem: public CMinimizer::Problem
{
public:
       C3DRegProblem(const C3DImage& model, C3DTransformation& transf,
                     const C3DImageCost& m_cost);
private:
       double  do_f(const CDoubleVector& x);
       void    do_df(const CDoubleVector& x, CDoubleVector&  g);
       double  do_fdf(const CDoubleVector& x, CDoubleVector&  g);
       size_t do_size() const;
       const C3DImage& m_model;
       C3DTransformation& m_transf;
       const C3DImageCost& m_cost;
};
typedef shared_ptr<C3DRegProblem> P3DRegProblem;


C3DRigidRegister::C3DRigidRegister(P3DImageCost cost, PMinimizer minimizer,
                                   P3DTransformationFactory transform_creator, size_t mg_levels):
       impl(new C3DRigidRegisterImpl( cost, minimizer, transform_creator, mg_levels))
{
}


C3DRigidRegister::~C3DRigidRegister()
{
       delete impl;
}

P3DTransformation C3DRigidRegister::run(P3DImage src, P3DImage ref) const
{
       return impl->run(src, ref);
}

C3DRigidRegisterImpl::C3DRigidRegisterImpl(P3DImageCost cost, PMinimizer minimizer,
              P3DTransformationFactory transform_creator,
              size_t mg_levels):
       m_cost(cost),
       m_minimizer(minimizer),
       m_transform_creator(transform_creator),
       m_mg_levels(mg_levels)
{
}

P3DTransformation C3DRigidRegisterImpl::run(P3DImage src, P3DImage ref) const
{
       assert(src);
       assert(ref);
       P3DTransformation transform;
       int x_shift = m_mg_levels + 1;
       int y_shift = m_mg_levels + 1;
       int z_shift = m_mg_levels + 1;

       while (x_shift && y_shift && z_shift) {
              if (x_shift)
                     x_shift--;

              if (y_shift)
                     y_shift--;

              if (z_shift)
                     z_shift--;

              C3DBounds BlockSize(1 << x_shift, 1 << y_shift, 1 << z_shift);
              cvinfo() << "Blocksize = " << BlockSize << "\n";
              stringstream downscale_descr;
              downscale_descr << "downscale:bx=" << BlockSize.x
                              << ",by=" << BlockSize.y
                              << ",bz=" << BlockSize.z;
              auto downscaler = C3DFilterPluginHandler::instance().produce(downscale_descr.str().c_str());
              auto src_scaled = x_shift && y_shift ? downscaler->filter(*src) : src;
              auto ref_scaled = x_shift && y_shift ? downscaler->filter(*ref) : ref;

              if (transform)
                     transform = transform->upscale(ref_scaled->get_size());
              else {
                     transform = m_transform_creator->create(ref_scaled->get_size());
                     // set initial scale
              }

              cvmsg() << "register at " << ref_scaled->get_size() << "\n";
              m_cost->set_reference(*ref_scaled);
              CMinimizer::PProblem gp = m_minimizer->has(property_gradient) ?
                                        CMinimizer::PProblem(new C3DRegFakeGradientProblem(*src_scaled, *transform, *m_cost)) :
                                        CMinimizer::PProblem(new C3DRegProblem(*src_scaled, *transform, *m_cost));
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

C3DRegGradientProblem::C3DRegGradientProblem(const C3DImage& model, C3DTransformation& transf,
              const C3DImageCost& cost):
       m_model(model),
       m_transf(transf),
       m_cost(cost),
       m_geval(0),
       m_feval(0)

{
       add(property_gradient);
}

size_t C3DRegGradientProblem::do_size() const
{
       return m_transf.degrees_of_freedom();
}

P3DImage C3DRegGradientProblem::apply(const CDoubleVector& x)
{
       cvinfo() << "\nParams:";

       for (auto i = x.begin(); i != x.end(); ++i)
              cverb << *i << " ";

       cverb << "\n";
       m_transf.set_parameters(x);
       return m_transf(m_model);
}

double  C3DRegGradientProblem::do_f(const CDoubleVector& x)
{
       ++m_feval;
       P3DImage temp = apply(x);
       const double value = m_cost.value(*temp);
       cvmsg() << "Cost(f=" << m_feval << ",g=" << m_geval << ") = " << value << "\n";
       return value;
}

void    C3DRegGradientProblem::do_df(const CDoubleVector& x, CDoubleVector&  g)
{
       ++m_geval;
       P3DImage temp = apply(x);
       C3DFVectorfield gradient(m_transf.get_size());
       m_cost.evaluate_force(*temp, gradient);
       m_transf.translate(gradient, g);
}

double  C3DRegGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
       ++m_geval;
       ++m_feval;
       P3DImage temp = apply(x);
       C3DFVectorfield gradient(m_transf.get_size());
       const double value = m_cost.evaluate_force(*temp, gradient);
       m_transf.translate(gradient, g);
       cvmsg() << "Cost(f=" << m_feval << ",g=" << m_geval << ") = " << value << "\n";
       return value;
}

C3DRegFakeGradientProblem::C3DRegFakeGradientProblem(const C3DImage& model,
              C3DTransformation& transf, const C3DImageCost& m_cost):
       C3DRegGradientProblem(model, transf,  m_cost)
{
}

void    C3DRegFakeGradientProblem::do_df(const CDoubleVector& x, CDoubleVector&  g)
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

double  C3DRegFakeGradientProblem::do_fdf(const CDoubleVector& x, CDoubleVector&  g)
{
       double cost_value = do_f(x);
       do_df(x, g);
       return cost_value;
}

C3DRegProblem::C3DRegProblem(const C3DImage& model, C3DTransformation& transf,
                             const C3DImageCost& cost):
       m_model(model),
       m_transf(transf),
       m_cost(cost)
{
}

double  C3DRegProblem::do_f(const CDoubleVector& x)
{
       cvinfo() << "\nParams:";

       for (auto i = x.begin(); i != x.end(); ++i)
              cverb << *i << " ";

       cverb << "\n";
       m_transf.set_parameters(x);
       P3DImage test =  m_transf(m_model);
       const double value = m_cost.value(*test);
       cvmsg() << "Cost = " << value << "\r";
       return value;
}

void   C3DRegProblem::do_df(const CDoubleVector&, CDoubleVector&  )
{
       DEBUG_ASSERT_RELEASE_THROW(false, "C3DRegProblem doesn't provide a gradient.");
}

double  C3DRegProblem::do_fdf(const CDoubleVector&, CDoubleVector&  )
{
       DEBUG_ASSERT_RELEASE_THROW(false, "C3DRegProblem doesn't provide a gradient.");
}

size_t C3DRegProblem::do_size() const
{
       return m_transf.degrees_of_freedom();
}


NS_MIA_END
