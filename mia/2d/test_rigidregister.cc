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

#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>


#include <mia/2d/rigidregister.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/core/minimizer.hh>
#include <mia/2d/filter.hh>


using namespace mia;
using namespace std;
namespace bfs = boost::filesystem;

class RigidRegisterFixture
{
protected:
       RigidRegisterFixture();
       void run(C2DTransformation& t, const string& minimizer_descr,  double accuracy);
       const C2DBounds size;
};

void RigidRegisterFixture::run(C2DTransformation& t, const string& minimizer_descr, double accuracy)
{
       auto minimizer = CMinimizerPluginHandler::instance().produce(minimizer_descr);
       P2DImageCost cost = C2DImageCostPluginHandler::instance().produce("ssd");
       C2DInterpolatorFactory  ipfactory(C2DInterpolatorFactory("bspline:d=3", "mirror"));
       auto tr_creator = C2DTransformCreatorHandler::instance().produce(t.get_creator_string());
       C2DRigidRegister rr(cost, minimizer, tr_creator, 1);
       float src_image_init[10 * 10] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 1, 2, 2, 2, 2, 1, 0, 0,
              0, 0, 1, 2, 7, 7, 2, 1, 0, 0,
              0, 0, 1, 2, 7, 7, 2, 1, 0, 0,
              0, 0, 1, 2, 2, 2, 2, 1, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       P2DImage src(new C2DFImage(size, src_image_init));
       P2DImage ref = t(*src);
       P2DTransformation transform = rr.run(src, ref);
       auto params = transform->get_parameters();
       auto orig_params = t.get_parameters();
       BOOST_CHECK_EQUAL(params.size(), orig_params.size());

       for (size_t i = 0; i < params.size(); ++i)
              if (fabs(orig_params[i]) > 0.1)
                     BOOST_CHECK_CLOSE(params[i], orig_params[i], accuracy);
              else
                     BOOST_CHECK_CLOSE(1.0 + params[i], 1.0 + orig_params[i], accuracy);
}

RigidRegisterFixture::RigidRegisterFixture():
       size(10, 10)
{
}

BOOST_FIXTURE_TEST_CASE( test_rigidreg_translate_nmsimplex, RigidRegisterFixture )
{
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
       auto transformation = tr_creator->create(size);
       auto params = transformation->get_parameters();
       params[0] = 1.0;
       params[1] = 1.0;
       transformation->set_parameters(params);
       run(*transformation, "gsl:opt=simplex,step=1.0", 1.0);
}


BOOST_FIXTURE_TEST_CASE( test_rigidreg_rigid_simplex, RigidRegisterFixture )
{
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("rigid");
       auto transformation = tr_creator->create(size);
       auto params = transformation->get_parameters();
       params[0] = 1.0;
       params[1] = 1.0;
       params[2] = 0.5;
       transformation->set_parameters(params);
       run(*transformation, "gsl:opt=simplex,step=1.0", 1.0);
}

BOOST_FIXTURE_TEST_CASE( test_rigidreg_affine_simplex, RigidRegisterFixture )
{
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("affine");
       auto transformation = tr_creator->create(size);
       auto params = transformation->get_parameters();
       params[0] =  1.0;
       params[1] =  0.0;
       params[2] =  0.0;
       params[3] =  0.0;
       params[4] =  1.0;
       params[5] =  2.0;
       transformation->set_parameters(params);
       run(*transformation, "gsl:opt=simplex,step=1.0,eps=0.0001", 1.0);
}

#ifdef HAVE_NLOPT

BOOST_FIXTURE_TEST_CASE( test_rigidreg_translate_gd, RigidRegisterFixture )
{
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
       auto transformation = tr_creator->create(size);
       auto params = transformation->get_parameters();
       params[0] = 1.0;
       params[1] = 1.0;
       transformation->set_parameters(params);
       run(*transformation, "nlopt:opt=ld-var1,xtolr=0.001,ftolr=0.001", 0.1);
}
#endif

#ifdef THIS_TEST_USES_THE_TRANSLATE_CODE_THAT_IS_NOT_WORKING
BOOST_AUTO_TEST_CASE( test_rigidreg_affine_cost_gradient ) //, RigidRegisterFixture )
{
       C2DBounds size(10, 10);
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("affine");
       auto transformation = tr_creator->create(size);
       P2DImageCost cost = C2DImageCostPluginHandler::instance().produce("ssd");
       C2DInterpolatorFactory   ipfactory("bspline:d=3", "mirror"));
       float src_image_init[10 * 10] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 1, 2, 2, 2, 2, 1, 0, 0,
              0, 0, 1, 2, 7, 7, 2, 1, 0, 0,
              0, 0, 1, 2, 7, 7, 2, 1, 0, 0,
              0, 0, 1, 2, 2, 2, 2, 1, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       float ref_image_init[10 * 10] = {
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 1, 3, 3, 3, 3, 1, 0, 0,
              0, 0, 1, 3, 5, 5, 3, 1, 0, 0,
              0, 0, 1, 3, 5, 5, 3, 1, 0, 0,
              0, 0, 1, 3, 3, 3, 3, 1, 0, 0,
              0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0
       };
       P2DImage ref(new C2DFImage(size, ref_image_init));
       P2DImage src(new C2DFImage(size, src_image_init));
       CDoubleVector gradient(transformation->degrees_of_freedom());
       cost->prepare_reference(*ref);
       C2DFVectorfield force(size);
       cost->evaluate_force(*src, *ref, 1.0, force);
       transformation->translate(force, gradient);
       auto params = transformation->get_parameters();

       for (size_t i = 0; i < transformation->degrees_of_freedom(); ++i) {
              params[i] -= 0.01;
              transformation->set_parameters(params);
              P2DImage tmp = (*transformation)(*src, ipfactory);
              double cm = cost->value(*tmp, *ref);
              params[i] += 0.02;
              transformation->set_parameters(params);
              tmp = (*transformation)(*src, ipfactory);
              double cp = cost->value(*tmp, *ref);
              params[i] -= 0.01;
              cvinfo() << "[" << cm << ", " << cp << "] maxtrans=" << transformation->get_max_transform() << "\n";
              cvinfo() << i << "\n";
              BOOST_CHECK_CLOSE(gradient[i], (cp - cm) / 0.02, 0.1);
       }
}
#endif

BOOST_FIXTURE_TEST_CASE( test_rigidreg_rigid_gd, RigidRegisterFixture )
{
       auto tr_creator = C2DTransformCreatorHandler::instance().produce("rigid:imgboundary=zero");
       auto transformation = tr_creator->create(size);
       auto params = transformation->get_parameters();
       params[0] = 1.0;
       params[1] = 1.0;
       params[2] = 0.5;
       transformation->set_parameters(params);
       // this is a rather high tolerance, especially in light that the
       // nm_simplex algorithm passes with a 0.1% tolerance
       run(*transformation, "gsl:opt=gd,iter=1000,tol=0.1", 9);
}

