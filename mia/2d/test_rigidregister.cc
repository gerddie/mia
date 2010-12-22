/*
** Copyright Madrid (c) 2010 BIT ETSIT UPM
**                      Gert Wollny <gw.fossdev @ gmail.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/


#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>


#include <mia/2d/rigidregister.hh>
#include <mia/2d/transformfactory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/2dfilter.hh>


NS_MIA_USE
namespace bfs=boost::filesystem;

class PluginPathInitFixture {
protected:
	PluginPathInitFixture() {
		list< bfs::path> costsearchpath;
		costsearchpath.push_back(bfs::path("cost"));
		C2DImageCostPluginHandler::set_search_path(costsearchpath);

		list< bfs::path> transsearchpath;
		transsearchpath.push_back(bfs::path("transform"));
		C2DTransformCreatorHandler::set_search_path(transsearchpath);

		list< bfs::path> kernelsearchpath;
		kernelsearchpath.push_back(bfs::path("..")/
					   bfs::path("core")/bfs::path("spacialkernel"));
		C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);

		list< bfs::path> filterpath;
		filterpath.push_back(bfs::path("filter"));
		C2DFilterPluginHandler::set_search_path(filterpath);


	}
};

class RigidRegisterFixture : public PluginPathInitFixture {
protected: 
	RigidRegisterFixture(); 
	void run(C2DTransformation& t, EMinimizers minimizer, double accuracy); 
	const C2DBounds size;
}; 

void RigidRegisterFixture::run(C2DTransformation& t, EMinimizers minimizer, double accuracy)
{
	P2DImageCost cost = C2DImageCostPluginHandler::instance().produce("ssd");
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));
	auto tr_creator = C2DTransformCreatorHandler::instance().produce(t.get_creator_string());

	C2DRigidRegister rr(cost, minimizer, tr_creator, *ipfactory, 1);


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
	P2DImage ref = t(*src, *ipfactory);

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
	size(10,10)
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

	run(*transformation, min_nmsimplex, 1.0); 
}


BOOST_FIXTURE_TEST_CASE( test_rigidreg_translate_gd, RigidRegisterFixture )
{
	auto tr_creator = C2DTransformCreatorHandler::instance().produce("translate");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] = 1.0;
	params[1] = 1.0;
	transformation->set_parameters(params); 

	run(*transformation, min_gd, 0.1); 
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

	run(*transformation, min_nmsimplex, 1.0); 
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

	run(*transformation, min_nmsimplex, 1.0); 
}

#ifdef THIS_TEST_USES_THE_TRANSLATE_CODE_THAT_IS_NOT_WORKING
BOOST_AUTO_TEST_CASE( test_rigidreg_affine_cost_gradient ) //, RigidRegisterFixture )
{
	C2DBounds size(10,10); 
	auto tr_creator = C2DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 

	P2DImageCost cost = C2DImageCostPluginHandler::instance().produce("ssd");
	unique_ptr<C2DInterpolatorFactory>   ipfactory(create_2dinterpolation_factory(ip_bspline3));

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

	gsl::DoubleVector gradient(transformation->degrees_of_freedom());
	
	cost->prepare_reference(*ref); 
	
	C2DFVectorfield force(size); 
	cost->evaluate_force(*src, *ref, 1.0, force);
	transformation->translate(force,gradient); 
	auto params = transformation->get_parameters(); 
	for (size_t i = 0; i < transformation->degrees_of_freedom(); ++i) {
		params[i] -= 0.01;  
		transformation->set_parameters(params); 
		P2DImage tmp = (*transformation)(*src, *ipfactory); 
		double cm = cost->value(*tmp, *ref); 
		
		params[i] += 0.02; 
		transformation->set_parameters(params); 
		tmp = (*transformation)(*src, *ipfactory); 
		double cp = cost->value(*tmp, *ref); 
		
		params[i] -= 0.01;
		cvinfo() << "[" << cm << ", " << cp << "] maxtrans=" << transformation->get_max_transform() <<"\n"; 
		cvinfo() << i << "\n"; 

		BOOST_CHECK_CLOSE(gradient[i], (cp - cm) / 0.02, 0.1); 
		
	}
}
#endif

BOOST_FIXTURE_TEST_CASE( test_rigidreg_rigid_gd, RigidRegisterFixture )
{
	auto tr_creator = C2DTransformCreatorHandler::instance().produce("rigid");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] = 1.0;
	params[1] = 1.0;
	params[2] = 0.5;
	transformation->set_parameters(params); 

	// this is a rather high tolerance, especially in light that the 
	// nm_simplex algorithm passes with a 0.1% tolerance 
	
	run(*transformation, min_cg_pr, 5); 
}

