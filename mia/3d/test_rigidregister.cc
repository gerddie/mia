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

#define VSTREAM_DOMAIN "test-rigidregister"
#include <stdexcept>
#include <climits>

#include <mia/internal/autotest.hh>


#include <mia/3d/rigidregister.hh>
#include <mia/3d/transformfactory.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/3d/3dfilter.hh>
#include <mia/3d/3dimageio.hh>

const TDictMap<EMinimizers>::Table g_minimizer_table[] = {
	{"simplex", min_nmsimplex},
	{"cg-fr", min_cg_fr},
	{"cg-pr", min_cg_pr},
	{"bfgs", min_bfgs},
	{"bfgs2", min_bfgs2},
	{"gd", min_gd},
	{NULL, min_undefined}
};

const TDictMap<EMinimizers> minimizers(g_minimizer_table); 

NS_MIA_USE
namespace bfs=boost::filesystem;

class PluginPathInitFixture {
protected:
	PluginPathInitFixture() {
		list< bfs::path> costsearchpath;
		costsearchpath.push_back(bfs::path("cost"));
		C3DImageCostPluginHandler::set_search_path(costsearchpath);

		list< bfs::path> transsearchpath;
		transsearchpath.push_back(bfs::path("transform"));
		C3DTransformCreatorHandler::set_search_path(transsearchpath);

		list< bfs::path> kernelsearchpath;
		kernelsearchpath.push_back(bfs::path("..")/
					   bfs::path("core")/bfs::path("spacialkernel"));
		C1DSpacialKernelPluginHandler::set_search_path(kernelsearchpath);

		list< bfs::path> filterpath;
		filterpath.push_back(bfs::path("filter"));
		C3DFilterPluginHandler::set_search_path(filterpath);


	}
};

class RigidRegisterFixture : public PluginPathInitFixture {
protected: 
	RigidRegisterFixture(); 
	void run(C3DTransformation& t, EMinimizers minimizer, double accuracy); 
	const C3DBounds size;
}; 

void RigidRegisterFixture::run(C3DTransformation& t, EMinimizers minimizer, double accuracy)
{
	P3DImageCost cost = C3DImageCostPluginHandler::instance().produce("ssd");
	unique_ptr<C3DInterpolatorFactory>   ipfactory(create_3dinterpolation_factory(ip_bspline3));
	auto tr_creator = C3DTransformCreatorHandler::instance().produce(t.get_creator_string());

	C3DRigidRegister rr(cost, minimizer, tr_creator, *ipfactory, 1);


	C3DFImage *psrc = new C3DFImage(size); 
	auto is = psrc->begin(); 
	const float hx = size.x/2.0;  
	const float hy = size.y/2.0;  
	const float hz = size.z/2.0;  
	for(size_t z = 0; z < size.z; ++z) {
		double fz = z - hz; 
		for(size_t y = 0; y < size.y; ++y) {
			double fy = 1.3 * y - size.y/2.0; 
			for(size_t x = 0; x < size.x; ++x,++is) {
				double fx = 1.5 * x - size.x/2.0; 
				*is = exp(-(fx*fx/(hx*hx) + fy*fy/(hy*hy) + fz*fz/(hz*hz))); 
			}
		}
	}
	P3DImage src(psrc);
	P3DImage ref = t(*src, *ipfactory);

	const C3DFImage& rsrc = dynamic_cast<const C3DFImage&>(*src); 
	const C3DFImage& rref = dynamic_cast<const C3DFImage&>(*ref); 
	
	auto iss = rsrc.begin(); 
	auto irs = rref.begin(); 
	
	for(size_t z = 0; z < size.z; ++z) {
		for(size_t y = 0; y < size.y; ++y) {
			cvdebug() << "src:"; 
			for(size_t x = 0; x < size.x; ++x,++iss)
				cverb << setw(9) << *iss << " "; 
			cverb << "\n"; 
			cvdebug() << "ref:"; 
			for(size_t x = 0; x < size.x; ++x,++irs)
				cverb << setw(9) << *irs << " "; 
			cverb << "\n"; 
		}
		cverb << "\n"; 
	}
	
			

	P3DTransformation transform = rr.run(src, ref);
	auto params = transform->get_parameters();
	auto orig_params = t.get_parameters();

	BOOST_CHECK_EQUAL(params.size(), orig_params.size());

	for (size_t i = 0; i < params.size(); ++i) 
		if (fabs(orig_params[i]) > 0.01) 
			BOOST_CHECK_CLOSE(params[i], orig_params[i], accuracy);
		else
			BOOST_CHECK_CLOSE(1.0 + params[i], 1.0 + orig_params[i], 2*accuracy);

	if ( cverb.get_level() <= vstream::ml_info ) {
		save_image
("src.hdr", src);
		save_image
("ref.hdr", ref);
		P3DImage reg = (*transform)(*src, *ipfactory); 
		stringstream out_name; 
		out_name << "reg-" << t.get_creator_string()
			 << "-" << minimizers.get_name(minimizer) << ".hdr"; 
		save_image
(out_name.str(), ref);
	}
}

RigidRegisterFixture::RigidRegisterFixture():
	size(20,30,30)
{
	
}

BOOST_FIXTURE_TEST_CASE( test_rigidreg_translate_nmsimplex, RigidRegisterFixture )
{

	auto tr_creator = C3DTransformCreatorHandler::instance().produce("translate");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] = 1.0;
	params[1] = 1.0;
	params[2] = 2.0;
	transformation->set_parameters(params); 

	run(*transformation, min_nmsimplex, 1.0); 
}


BOOST_FIXTURE_TEST_CASE( test_rigidreg_translate_gd, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("translate");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] = 1.0;
	params[1] = 1.0;
	params[2] = 2.0;
	transformation->set_parameters(params); 

	run(*transformation, min_gd, 0.4); 
}

BOOST_FIXTURE_TEST_CASE( test_rigid_simplex, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("rigid");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  1.0;
	params[1] =  3.0;
	params[2] =  1.0;
	params[3] =  0.3;
	params[4] =  0.3;
	params[5] = -0.2;
	
	transformation->set_parameters(params); 

	run(*transformation, min_nmsimplex, 16.0); 
}


BOOST_FIXTURE_TEST_CASE( test_rigid_gd, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("rigid");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  0.0;
	params[1] =  0.0;
	params[2] =  0.0;
	params[3] = -0.3;
	params[4] =  0.3;
	params[5] =  0.2;
	
	transformation->set_parameters(params); 

	run(*transformation, min_gd, 8.0); 
}

#if 0 
// the problem with tese tests is, that the images to be registered are 
// too ambigious, so that more then just one affine transformation 
// can result in a good registration - which means the parameters 
// will not be recovered. 
BOOST_FIXTURE_TEST_CASE( test_affine_simplex, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  1.0;
	params[1] =  0.4;
	params[2] =  0.7;
	params[3] = -13.0;

	params[4] = -0.1;
	params[5] =  1.02;
	params[6] =  0.4;
	params[7] = -9.1;

	params[8] =  0.9;
	params[9] = -0.4;
	params[10] = 0.9;
	params[11] = -10.3;

	transformation->set_parameters(params); 

	run(*transformation, min_nmsimplex, 1.0); 
}

BOOST_FIXTURE_TEST_CASE( test_affine_gd, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  1.0;
	params[1] =  0.4;
	params[2] =  0.7;
	params[3] = -13.0;

	params[4] = -0.1;
	params[5] =  1.02;
	params[6] =  0.4;
	params[7] = -9.1;

	params[8] =  0.9;
	params[9] = -0.4;
	params[10] = 0.9;
	params[11] = -10.3;

	transformation->set_parameters(params); 

	run(*transformation, min_gd, 1.0); 
}

BOOST_FIXTURE_TEST_CASE( test_affine_bfgs, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 

	params[0] =  1.0;
	params[1] =  0.4;
	params[2] =  0.7;
	params[3] = -13.0;

	params[4] = -0.1;
	params[5] =  1.02;
	params[6] =  0.4;
	params[7] = -9.1;

	params[8] =  0.9;
	params[9] = -0.4;
	params[10] = 0.9;
	params[11] = -10.3;

	transformation->set_parameters(params); 

	run(*transformation, min_bfgs, 1.0); 
}

BOOST_FIXTURE_TEST_CASE( test_affine_cg_fr, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  1.0;
	params[1] =  0.4;
	params[2] =  0.7;
	params[3] = -13.0;

	params[4] = -0.1;
	params[5] =  1.02;
	params[6] =  0.4;
	params[7] = -9.1;

	params[8] =  0.9;
	params[9] = -0.4;
	params[10] = 0.9;
	params[11] = -10.3;

	transformation->set_parameters(params); 

	run(*transformation, min_cg_fr, 1.0); 
}

BOOST_FIXTURE_TEST_CASE( test_affine_cg_pr, RigidRegisterFixture )
{
	auto tr_creator = C3DTransformCreatorHandler::instance().produce("affine");
	auto transformation = tr_creator->create(size); 
	auto params = transformation->get_parameters(); 
	params[0] =  1.0;
	params[1] =  0.4;
	params[2] =  0.7;
	params[3] = -13.0;

	params[4] = -0.1;
	params[5] =  1.02;
	params[6] =  0.4;
	params[7] = -9.1;

	params[8] =  0.9;
	params[9] = -0.4;
	params[10] = 0.9;
	params[11] = -10.3;

	transformation->set_parameters(params); 

	run(*transformation, min_cg_pr, 1.0); 
}

#endif