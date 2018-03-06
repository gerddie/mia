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


#include <mia/2d/cost/gncc.hh>
#include <mia/core/parallel.hh>

namespace mia_2d_gncc
{


CGNCC2DImageCost::CGNCC2DImageCost():
{
       m_convert_to_float = produce_2dimage_filter("convert:repn=float,map=copy");
       m_sobel_x = produce_2dimage_filter("sobel:dir=x");
       m_sobel_y = produce_2dimage_filter("sobel:dir=y");
}


class SumNccPart
{
       SumNccPart(const C2DFImage& mov, const C2DFImage& ref);
       NCCSums operator ()(const C1DParallelRange& range, const NCCSums& sumacc) const;
private:
       const C2DFImage& m_mov;
       const C2DFImage& m_ref;
};

SumNccPart::SumNccPart(const C2DFImage& mov, const C2DFImage& ref):
       m_mov(mov),
       m_ref(ref)
{
}


NCCSums SumNccPart::operator ()(const C1DParallelRange& range, const NCCSums& sumacc) const
{
       for (auto y = range.begin(); y != range.end(); ++y) {
              auto iref = m_ref.begin_at(0, y);
              auto eref = m_ref.begin_at(0, y + 1);
              auto imov = m_mov.begin_at(0, y);

              while (iref != eref) {
                     nccsum.add(*iref, *imov);
                     ++imov, ++iref;
              }

              return sum + sumacc;
       }
}

double CGNCC2DImageCost::do_value(const mia::C2DImage& a,
                                  const mia::C2DImage& MIA_PARAM_UNUSED(b)) const
{
       auto float_image = m_convert_to_float->filter(a);
       auto agx =  m_sobel_x->filter(*float_image);
       auto agy =  m_sobel_y->filter(*float_image);
       auto& nagx = dynamic_cast<const C2DFImage&>(*agx);
       auto& nagy = dynamic_cast<const C2DFImage&>(*agy);
       SumNccPart sum_part_x(nagx, m_grad_x);
       SumNccPart sum_part_y(nagy, m_grad_y);
       NCCSums sum;
       sum = preduce(C1DParallelRange(0, mov.get_size().y, 1), sum, sum_part_x,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       sum = preduce(C1DParallelRange(0, mov.get_size().y, 1), sum, sum_part_y,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       return sum.value();
}

double CGNCC2DImageCost::do_evaluate_force(const mia::C2DImage& a,
              const mia::C2DImage& MIA_PARAM_UNUSED(b),
              mia::C2DFVectorfield& force) const
{
       auto float_image = m_convert_to_float->filter(a);
       auto agx =  m_sobel_x->filter(*float_image);
       auto agy =  m_sobel_y->filter(*float_image);
       auto& nagx = dynamic_cast<const C2DFImage&>(*agx);
       auto& nagy = dynamic_cast<const C2DFImage&>(*agy);
       SumNccPart sum_part_x(nagx, m_grad_x);
       SumNccPart sum_part_y(nagy, m_grad_y);
       NCCSums sum_x;
       sum_x = preduce(C1DParallelRange(0, mov.get_size().y, 1), sum_x, sum_part_x,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       NCCSums sum_y;
       sum_y = preduce(C1DParallelRange(0, mov.get_size().y, 1), sum_y, sum_part_y,
       [](const NCCSums & x, const NCCSums & y) {
              return x + y;
       });
       NCCSums sum = sum_x + sum_y;
       double retval = sum.value();
       // from here on we evaluate the gradinet
       auto x_grad_helper = sum_x.get_grad_helper();
       auto y_grad_helper = sum_y.get_grad_helper();
       auto ddx = get_gradient(nagx);
       auto ddy = get_gradient(nagy);
       auto eval_force = [this, &x_grad_helper, &y_grad_helper, &nagx, & nagy, &ddx, &ddy, &force](const C1DParallelRange & range) -> void {
              for (auto y = range.begin(); y != range.end(); ++y)
              {
                     int offs = nagx.get_size().x * y;

                     for (auto x = 0; x < nagx.get_size().x; ++x, ++offs) {
                            auto g = x_grad_helper.second.get_gradient_scale(nagx[offs], m_grad_x[offs]) * ddx[offs];
                            g += y_grad_helper.second.get_gradient_scale(nagy[offs], m_grad_y[offs]) * ddy[offs];
                            force[offs]  = g;
                     }
              }
       }
       pfor(C1DParallelRange(0, a.get_size().y, 1), eval_force);
       return retval;
}

void CGNCC2DImageCost::post_set_reference(const mia::C2DImage& ref)
{
       auto float_image = m_convert_to_float->filter(ref);
       auto agx =  m_sobel_x->filter(*float_image);
       auto agy =  m_sobel_y->filter(*float_image);
       m_grad_x = dynamic_cast<const C2DFImage&>(*agx);
       m_grad_y = dynamic_cast<const C2DFImage&>(*agy);
}

CGNCC2DImageCostPlugin::CGNCC2DImageCostPlugin():
       mia::C2DImageCostPlugin("gncc")
{
}

mia::C2DImageCost *CGNCC2DImageCostPlugin::do_create() const
{
       return new CGNCC2DImageCost();
}


const std::string CGNCC2DImageCostPlugin::do_get_descr() const
{
       return "Implementation of the 2D normalized gradient correlation image similarity measures";
}


NS_END(NS)
