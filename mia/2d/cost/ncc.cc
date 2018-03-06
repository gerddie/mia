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

#include <mia/2d/cost/ncc.hh>

#include <mia/core/threadedmsg.hh>
#include <mia/core/nccsum.hh>
#include <mia/core/parallel.hh>

NS_BEGIN(NS)

using namespace mia;


CNCC2DImageCost::CNCC2DImageCost()
{
       m_copy_to_double = produce_2dimage_filter("convert:repn=double,map=copy");
}

struct FEvaluateNCCSum {
       FEvaluateNCCSum(const C2DDImage& mov, const C2DDImage& ref);
       NCCSums operator ()(const C1DParallelRange& range, const NCCSums& sumacc) const;
private:
       const C2DDImage& m_mov;
       const C2DDImage& m_ref;
};


FEvaluateNCCSum::FEvaluateNCCSum(const C2DDImage& mov, const C2DDImage& ref):
       m_mov(mov), m_ref(ref)
{
}

NCCSums FEvaluateNCCSum::operator ()(const C1DParallelRange& range, const NCCSums& sumacc) const
{
       CThreadMsgStream msks;
       NCCSums sum;

       for (auto y = range.begin(); y != range.end(); ++y) {
              auto ia = m_mov.begin_at(0, y);
              auto ib = m_ref.begin_at(0, y);
              auto eb = m_ref.begin_at(0, y + 1);

              while (ib != eb) {
                     sum.add(*ia, *ib);
                     ++ia;
                     ++ib;
              }
       }

       return sum + sumacc;
};

double CNCC2DImageCost::do_value(const Data& a, const Data& b) const
{
       auto a_double_ptr = m_copy_to_double->filter(a);
       auto b_double_ptr = m_copy_to_double->filter(b);
       const C2DDImage& a_double = static_cast<const C2DDImage&>(*a_double_ptr);
       const C2DDImage& b_double = static_cast<const C2DDImage&>(*b_double_ptr);
       FEvaluateNCCSum ev(a_double, b_double);
       NCCSums sum;
       sum = preduce(C1DParallelRange(0, a_double.get_size().y, 1), sum, ev,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       return sum.value();
}

double CNCC2DImageCost::do_evaluate_force(const Data& a, const Data& b, Force& force) const
{
       auto a_double_ptr = m_copy_to_double->filter(a);
       auto b_double_ptr = m_copy_to_double->filter(b);
       const C2DDImage& a_double = static_cast<const C2DDImage&>(*a_double_ptr);
       const C2DDImage& b_double = static_cast<const C2DDImage&>(*b_double_ptr);
       NCCSums sum;
       FEvaluateNCCSum ev(a_double, b_double);
       sum = preduce(C1DParallelRange(0, a_double.get_size().y, 1), sum, ev,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       auto geval = sum.get_grad_helper();
       auto grad = get_gradient(a_double);
       auto grad_eval = [&force, &a_double, &b_double, &grad, &geval](const C1DParallelRange & range) {
              for (auto y = range.begin(); y != range.end(); ++y) {
                     auto ig = grad.begin_at(0, y);
                     auto iforce = force.begin_at(0, y);
                     auto ia = a_double.begin_at(0, y);
                     auto ib = b_double.begin_at(0, y);
                     auto eb = b_double.begin_at(0, y + 1);

                     while (ib != eb) {
                            *iforce = geval.second.get_gradient_scale(*ia, *ib) * *ig;
                            ++ig;
                            ++iforce;
                            ++ia;
                            ++ib;
                     }
              };
       };
       pfor(C1DParallelRange(0, a_double.get_size().y, 1), grad_eval);
       return geval.first;
}


CNCC2DImageCostPlugin::CNCC2DImageCostPlugin():
       C2DImageCostPlugin("ncc")
{
}

C2DImageCost *CNCC2DImageCostPlugin::do_create() const
{
       return new CNCC2DImageCost();
}

const std::string CNCC2DImageCostPlugin::do_get_descr() const
{
       return "normalized cross correlation.";
}

extern "C" EXPORT CPluginBase *get_plugin_interface()
{
       return new CNCC2DImageCostPlugin();
}

NS_END
