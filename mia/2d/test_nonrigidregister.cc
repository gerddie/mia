/* -*- mia-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004 - 2010
 * Max-Planck-Institute for Human Cognitive and Brain Science
 * Max-Planck-Institute for Evolutionary Anthropology
 * BIT, ETSI Telecomunicacion, UPM
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#define VSTREAM_DOMAIN "NR-TEST"

#include <cmath>
#include <sstream>
#include <mia/internal/autotest.hh>
#include <mia/core/datapool.hh>
#include <mia/core/spacial_kernel.hh>
#include <mia/2d/nonrigidregister.hh>
#include <mia/2d/2dimageio.hh>
#include <mia/2d/2dfilter.hh>
#include <mia/2d/cost.hh>

NS_MIA_USE
using namespace std;
namespace bfs=boost::filesystem;

struct NonRigidRegistrationFixture {
	NonRigidRegistrationFixture();

	void Init(const char* model, const char *timestep, const char* transform);

	string create_image(const char *id, const C2DBounds & size, vector<float>& data);


	size_t start_size;
	size_t max_iter;
	float outer_epsilon;
	auto_ptr<C2DMultiImageNonrigidRegister> registration;
	C2DBounds size;

};



BOOST_FIXTURE_TEST_CASE( test_nonrigid_registration, NonRigidRegistrationFixture )
{
	Init("navier", "fluid", "vf");

	vector<float> src_data(size.x * size.y);
	vector<float> ref_data(size.x * size.y);
	vector<float>::iterator i = src_data.begin();
	vector<float>::iterator r = ref_data.begin();
	for (size_t y = 0; y < size.y; ++y) {
		for (size_t x = 0; x < size.x; ++x, ++i, ++r) {
			float fx = x;
			float fy = y;
			*i = (fx - size.x/2) * (fx - size.x/2) + (fy - size.y/2) * (fy - size.y/2);
			*r = (fx - size.x/2 - 2) * (fx - size.x/2 - 2) + (fy - size.y/2 + 2) * (fy - size.y/2 - 2);
		}
	}
	// create images & cost function
	string src_name = create_image("src", size, src_data);
	string ref_name = create_image("ref", size, ref_data);
	stringstream coststr;
	coststr << "ssd:src=" << src_name << ",ref=" << ref_name;

	C2DImageFatCostList cost;
	cvmsg() << "Create cost from: '" << coststr.str() << "'\n";
	cost.push_back(C2DFatImageCostPluginHandler::instance().produce(coststr.str().c_str()));

	P2DTransformation result = (*registration)(cost);

	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(result->get_size(), size);


}

BOOST_FIXTURE_TEST_CASE( test_nonrigid_spline_registration, NonRigidRegistrationFixture )
{
	Init("navier", "fluid", "spline:rate=4");

	vector<float> src_data(size.x * size.y);
	vector<float> ref_data(size.x * size.y);
	vector<float>::iterator i = src_data.begin();
	vector<float>::iterator r = ref_data.begin();
	for (size_t y = 0; y < size.y; ++y) {
		for (size_t x = 0; x < size.x; ++x, ++i, ++r) {
			float fx = x;
			float fy = y;
			*i = (fx - size.x/2) * (fx - size.x/2) + (fy - size.y/2) * (fy - size.y/2);
			*r = (fx - size.x/2 - 2) * (fx - size.x/2 - 2) + (fy - size.y/2 + 2) * (fy - size.y/2 - 2);
		}
	}
	// create images & cost function
	string src_name = create_image("src", size, src_data);
	string ref_name = create_image("ref", size, ref_data);
	stringstream coststr;
	coststr << "ssd:src=" << src_name << ",ref=" << ref_name;

	C2DImageFatCostList cost;
	cvmsg() << "Create cost from: '" << coststr.str() << "'\n";
	cost.push_back(C2DFatImageCostPluginHandler::instance().produce(coststr.str().c_str()));

	P2DTransformation result = (*registration)(cost);

	BOOST_REQUIRE(result);
	BOOST_CHECK_EQUAL(result->get_size(), size);


}


template <typename Handler>
struct setup_path {
	static void apply(const char *spath)
	{
		list< bfs::path> searchpath;
		searchpath.push_back(bfs::path(spath));
		Handler::set_search_path(searchpath);

	}
};


NonRigidRegistrationFixture::NonRigidRegistrationFixture():
	start_size(16),
	max_iter(20),
	outer_epsilon(0.01),
	size(16,32)
{
	// set up the plugin path
	setup_path<C2DTransformCreatorHandler>::apply("transform");
	setup_path<C2DRegModelPluginHandler>::apply("model");
	setup_path<C2DRegTimeStepPluginHandler>::apply("timestep");
	setup_path<C2DFatImageCostPluginHandler>::apply("cost");
	setup_path<C2DImageCostPluginHandler>::apply("cost");
	setup_path<C2DFilterPluginHandler>::apply("filter");
	setup_path<C1DSpacialKernelPluginHandler>::apply("../core/spacialkernel");

	// standard elements
	// actually this should use some mock objects, but fot a first test
	// we use the real thing

}

void NonRigidRegistrationFixture::Init(const char* smodel, const char *stimestep, const char* stransform)
{
	P2DRegModel model = C2DRegModelPluginHandler::instance().produce(smodel);
	P2DRegTimeStep time_step = C2DRegTimeStepPluginHandler::instance().produce(stimestep);
	P2DTransformationFactory trans_factory =  C2DTransformCreatorHandler::instance().produce(stransform);

	registration.reset(new C2DMultiImageNonrigidRegister(start_size, max_iter,
							     model,
							     time_step,
							     trans_factory,
							     outer_epsilon));
}


string NonRigidRegistrationFixture::create_image(const char *id, const C2DBounds & size, vector<float>& data)
{
	P2DImage image(new C2DFImage(size, &data[0]));
	C2DImageIOPlugin::PData image_list(new C2DImageVector);
	image_list->push_back(image);
	stringstream key;
	key << id << ".datapool";
	CDatapool::Instance().add(key.str(), image_list);
	return key.str();
}
