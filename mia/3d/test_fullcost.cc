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

#include <mia/3d/cost.hh>
#include <mia/3d/fullcost.hh>
#include <mia/3d/multicost.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/3d/transformmock.hh>
#include <mia/3d/imageio.hh>

#include <mia/internal/autotest.hh>

NS_MIA_USE
namespace bfs =::boost::filesystem;

class C3DFullCostMock: public C3DFullCost
{
public:
       C3DFullCostMock(double weight, double cost, double gx, double gy, double gz);
private:
       double do_evaluate(const C3DTransformation& t, CDoubleVector& gradient) const;
       double do_value(const C3DTransformation& t) const;
       double do_value() const;
       void do_set_size();
       void do_reinit();
       C3DFMatrix derivative_at(int x, int y, int z) const;
       double m_cost;
       double m_gx;
       double m_gy;
       double m_gz;
};

C3DFullCostMock::C3DFullCostMock(double weight, double cost, double gx, double gy, double gz):
       C3DFullCost(weight),
       m_cost(cost),
       m_gx(gx),
       m_gy(gy),
       m_gz(gz)

{
}

double C3DFullCostMock::do_evaluate(const C3DTransformation&, CDoubleVector& gradient) const
{
       for (auto g = gradient.begin(); g != gradient.end(); g += 3) {
              g[0] = m_gx;
              g[1] = m_gy;
              g[2] = m_gz;
       }

       return m_cost;
}

double C3DFullCostMock::do_value(const C3DTransformation& /*t*/) const
{
       return m_cost;
}

double C3DFullCostMock::do_value() const
{
       return m_cost;
}
void C3DFullCostMock::do_reinit()
{
}

void C3DFullCostMock::do_set_size()
{
}

BOOST_AUTO_TEST_CASE( test_fullcost )
{
       const double mcost = 10.0;
       const double mgx   = 2.0;
       const double mgy   = 3.0;
       const double mgz   = 4.0;
       C3DFullCostMock c(0.5, mcost, mgx, mgy, mgz);
       C3DTransformMock t(C3DBounds(2, 1, 1), C3DInterpolatorFactory("bspline:d=3", "mirror"));
       CDoubleVector gradient(t.degrees_of_freedom());
       c.set_size(t.get_size());
       BOOST_CHECK_CLOSE(c.evaluate(t, gradient), 0.5 * mcost, 0.01);
       BOOST_CHECK_CLOSE(gradient[0], 0.5 * mgx, 0.01);
       BOOST_CHECK_CLOSE(gradient[1], 0.5 * mgy, 0.01);
       BOOST_CHECK_CLOSE(gradient[2], 0.5 * mgz, 0.01);
}

BOOST_AUTO_TEST_CASE( test_multicost )
{
       const double mcost1 = 10.0;
       const double mgx1   = 2.0;
       const double mgy1   = 3.0;
       const double mgz1   = 5.0;
       const double mcost2 = 2.0;
       const double mgx2   = 4.0;
       const double mgy2   = 9.0;
       const double mgz2   = 7.0;
       P3DFullCost c1(new C3DFullCostMock(0.5, mcost1, mgx1, mgy1, mgz1));
       P3DFullCost c2(new C3DFullCostMock(0.2, mcost2, mgx2, mgy2, mgz2));
       C3DFullCostList costs;
       costs.push(c1);
       costs.push(c2);
       C3DTransformMock t(C3DBounds(2, 1, 1), C3DInterpolatorFactory("bspline:d=3", "mirror"));
       CDoubleVector gradient(t.degrees_of_freedom());
       costs.set_size(t.get_size());
       BOOST_CHECK_CLOSE(costs.evaluate(t, gradient), 0.5 * mcost1 + 0.2 * mcost2, 0.1);
       BOOST_CHECK_CLOSE(gradient[0], 0.5 * mgx1 + 0.2 * mgx2, 0.1);
       BOOST_CHECK_CLOSE(gradient[1], 0.5 * mgy1 + 0.2 * mgy2, 0.1);
       BOOST_CHECK_CLOSE(gradient[2], 0.5 * mgz1 + 0.2 * mgz2, 0.1);
}


BOOST_AUTO_TEST_CASE( test_multicost2 )
{
       const double mcost1 = 10.0;
       const double mgx1   = 2.0;
       const double mgy1   = 3.0;
       const double mgz1   = 15.0;
       const double mcost2 = 2.0;
       const double mgx2   = 4.0;
       const double mgy2   = 9.0;
       const double mgz2   = -2.0;
       P3DFullCost c1(new C3DFullCostMock(0.5, mcost1, mgx1, mgy1, mgz1));
       P3DFullCost c2(new C3DFullCostMock(0.2, mcost2, mgx2, mgy2, mgz2));
       C3DFullCostList costs;
       costs.push(c2);
       costs.push(c1);
       C3DTransformMock t(C3DBounds(2, 1, 1), C3DInterpolatorFactory("bspline:d=3", "mirror"));
       CDoubleVector gradient(t.degrees_of_freedom(), true);
       costs.set_size(t.get_size());
       BOOST_CHECK_CLOSE(costs.evaluate(t, gradient), 0.5 * mcost1 + 0.2 * mcost2, 0.1);
       BOOST_CHECK_CLOSE(gradient[0], 0.5 * mgx1 + 0.2 * mgx2, 0.1);
       BOOST_CHECK_CLOSE(gradient[1], 0.5 * mgy1 + 0.2 * mgy2, 0.1);
       BOOST_CHECK_CLOSE(gradient[2], 0.5 * mgz1 + 0.2 * mgz2, 0.1);
}


BOOST_AUTO_TEST_CASE( test_multicost_property )
{
       const double mcost1 = 10.0;
       const double mgx1   = 2.0;
       const double mgy1   = 3.0;
       const double mgz1   = 15.0;
       const double mcost2 = 2.0;
       const double mgx2   = 4.0;
       const double mgy2   = 9.0;
       const double mgz2   = -2.0;
       P3DFullCost c1(new C3DFullCostMock(0.5, mcost1, mgx1, mgy1, mgz1));
       P3DFullCost c2(new C3DFullCostMock(0.2, mcost2, mgx2, mgy2, mgz2));
       const char *test_prop = "test_prop";
       C3DFullCostList costs;
       BOOST_CHECK(!costs.has(test_prop));
       costs.push(c1);
       costs.push(c2);
       BOOST_CHECK(!costs.has(test_prop));
       c1->add(test_prop);
       BOOST_CHECK(!costs.has(test_prop));
       c2->add(test_prop);
       BOOST_CHECK(costs.has(test_prop));
}

class PrepareFullcostTests
{
public:
       static const PrepareFullcostTests& instance();

       const C3DFullCostPluginHandler::Instance& fullcost_handler()const;
       const C3DTransformCreatorHandler::Instance& transform_handler()const;
private:
       PrepareFullcostTests();
};

#if 0
BOOST_AUTO_TEST_CASE( test_load_plugins )
{
       const C3DFullCostPluginHandler::Instance& handler = PrepareFullcostTests::instance().fullcost_handler();
       BOOST_CHECK_EQUAL(handler.size(), 2u);
       BOOST_CHECK_EQUAL(handler.get_plugin_names(), "divcurl image ");
}
#endif

#if 0
BOOST_AUTO_TEST_CASE ( test_ssd_cost_spline_rate_3 )
{
       auto cost_ssd = PrepareFullcostTests::instance().fullcost_handler().produce("image:cost=ssd");
       auto transform_factory = PrepareFullcostTests::instance().transform_handler().produce("spline:rate=3");
       C3DBounds size(16, 16);
       const float src_init[ 16 * 16] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 20, 20, 20, 10, 10, 10, 10, 10, 20, 10, 10, 10, 0,
              0, 10, 10, 20, 30, 30, 30, 30, 30, 30, 30, 30, 20, 10, 0,
              0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 20, 20, 20, 10, 10, 10, 10, 10, 20, 10, 10, 10, 0,
              0, 10, 10, 20, 30, 30, 30, 30, 30, 30, 30, 30, 20, 10, 0,
              0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 20, 20, 20, 10, 10, 10, 10, 10, 20, 10, 10, 10, 0,
              0, 10, 10, 20, 30, 30, 30, 30, 30, 30, 30, 30, 20, 10, 0,
              0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

       };
       const float ref_init[ 16 * 16] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 30, 30, 40, 20, 20, 30, 30, 40, 40, 20, 30, 10, 0,
              0, 10, 20, 10, 20, 20, 20, 30, 40, 50, 40, 40, 30, 10, 0,
              0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 20, 20, 20, 10, 10, 10, 10, 10, 20, 10, 10, 10, 0,
              0, 10, 30, 30, 20, 20, 32, 20, 30, 40, 50, 30, 20, 10, 0,
              0, 0, 30, 30, 30, 30, 30, 30, 20, 20, 50, 20, 20, 10, 0,
              0, 10, 30.10, 30, 30, 30, 30, 30, 20, 20, 20, 10, 10, 0,
              0, 10, 30, 10, 30, 30, 40, 43, 20, 20, 30, 30, 10, 10, 0,
              0, 10, 10, 20, 30, 30, 20, 20, 20, 20, 20, 30, 20, 10, 0,
              0, 0, 20, 20, 20, 30, 30, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

       };
       P3DImage src(new C3DFImage(size, src_init));
       save_image3d("src.@", src);
       P3DImage ref(new C3DFImage(size, ref_init));
       save_image3d("ref.@", ref);
       cost_ssd->reinit();
       cost_ssd->set_size(size);
       P3DTransformation t = transform_factory->create(size);
       auto params = t->get_parameters();
       CDoubleVector gradient(params.size());
       double cost = cost_ssd->evaluate(*t, gradient);
//	BOOST_CHECK_CLOSE(cost, 55*55 / 1024.0, 0.1);

       for (size_t i = 0; i < params.size(); ++i) {
              params[i] -= 0.0001;
              t->set_parameters(params);
              double costm = cost_ssd->cost_value(*t);
              params[i] += 0.0002;
              t->set_parameters(params);
              double costp = cost_ssd->cost_value(*t);
              params[i] -= 0.0001;
              double cgrad = (costp - costm) / 0.0002;
              cvinfo()  << gradient[i] << " vs " << cgrad << " "
                        << gradient[i] / cgrad << "\n";

              if (fabs(cgrad) > 0.001)
                     BOOST_CHECK_CLOSE(gradient[i], cgrad, 0.1);
              else
                     BOOST_CHECK_CLOSE(1.0 + gradient[i], 1.0, 0.1);
       }
}
#endif

#if 0
BOOST_AUTO_TEST_CASE ( test_ssd_cost_vf )
{
       auto cost_ssd = PrepareFullcostTests::instance().fullcost_handler().produce("image:cost=ssd");
       auto transform_factory = PrepareFullcostTests::instance().transform_handler().produce("vf");
       C3DBounds size(16, 16);
       const float src_init[ 16 * 16] = {
              0, 10, 20, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 20, 30, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 30, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 40, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 50, 60, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 60, 70, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 70, 80, 90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 80, 90, 00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 70, 80, 90, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 60, 70, 80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 50, 60, 70, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 40, 50, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 30, 40, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 20, 30, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 10, 20, 30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 10, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

       };
       const float ref_init[ 16 * 16] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 30, 30, 40, 20, 20, 30, 30, 40, 40, 20, 30, 10, 0,
              0, 10, 20, 10, 20, 20, 20, 30, 40, 50, 40, 40, 30, 10, 0,
              0, 0, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 10, 10, 0,
              0, 10, 20, 20, 20, 10, 10, 10, 10, 10, 20, 10, 10, 10, 0,
              0, 10, 30, 30, 20, 20, 32, 20, 30, 40, 50, 30, 20, 10, 0,
              0, 0, 30, 30, 30, 30, 30, 30, 20, 20, 50, 20, 20, 10, 0,
              0, 10, 30.10, 30, 30, 30, 30, 30, 20, 20, 20, 10, 10, 0,
              0, 10, 30, 10, 30, 30, 40, 43, 20, 20, 30, 30, 10, 10, 0,
              0, 10, 10, 20, 30, 30, 20, 20, 20, 20, 20, 30, 20, 10, 0,
              0, 0, 20, 20, 20, 30, 30, 20, 20, 20, 20, 20, 20, 10, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

       };
       P3DImage src(new C3DFImage(size, src_init));
       save_image3d("src.@", src);
       P3DImage ref(new C3DFImage(size, ref_init));
       save_image3d("ref.@", ref);
       cost_ssd->reinit();
       cost_ssd->set_size(size);
       P3DTransformation t = transform_factory->create(size);
       auto params = t->get_parameters();
       CDoubleVector gradient(params.size());
       double cost = cost_ssd->evaluate(*t, gradient);
//	BOOST_CHECK_CLOSE(cost, 55*55 / 1024.0, 0.1);

       for (size_t i = 0; i < params.size(); ++i) {
              params[i] -= 0.0001;
              t->set_parameters(params);
              double costm = cost_ssd->cost_value(*t);
              params[i] += 0.0002;
              t->set_parameters(params);
              double costp = cost_ssd->cost_value(*t);
              params[i] -= 0.0001;
              double cgrad = (costp - costm) / 0.0002;
              cvinfo()  << gradient[i] << " vs " << cgrad << " "
                        << gradient[i] / cgrad << "\n";

              if (fabs(cgrad) > 0.01)
                     BOOST_CHECK_CLOSE(gradient[i], cgrad, 0.1);
              else
                     BOOST_CHECK_CLOSE(1.0 + gradient[i], 1.0, 0.1);
       }
}
#endif

const PrepareFullcostTests& PrepareFullcostTests::instance()
{
       const static PrepareFullcostTests  me;
       return me;
}

const C3DFullCostPluginHandler::Instance& PrepareFullcostTests::fullcost_handler() const
{
       return C3DFullCostPluginHandler::instance();
}

const C3DTransformCreatorHandler::Instance& PrepareFullcostTests::transform_handler()const
{
       return C3DTransformCreatorHandler::instance();
}

PrepareFullcostTests::PrepareFullcostTests()
{
}

