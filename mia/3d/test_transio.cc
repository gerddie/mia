/* -*- mona-c++  -*-
 *
 * Copyright (c) Leipzig, Madrid 2004-2011
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

#define VSTREAM_DOMAIN "test-transio"
#include <mia/internal/autotest.hh>
#include <mia/3d/transformio.hh>
#include <mia/3d/transformfactory.hh>
#include <boost/static_assert.hpp>

NS_MIA_USE; 
namespace bfs=::boost::filesystem; 
class PrepareTransIOTests {
public: 
	static const PrepareTransIOTests& instance(); 
	
	const C3DTransformationIOPluginHandler::Instance& transformio_handler()const; 
	const C3DTransformCreatorHandler::Instance& transform_handler()const; 
private:
	PrepareTransIOTests(); 
}; 

BOOST_AUTO_TEST_CASE(test_transform_io) 
{
	C3DBounds size( 20, 22, 23); 
	
	const char *transforms[] = {
		"translate",
		"affine", 
		"rigid"
#if 0
		, 
		"spline:rate=3", 
		"spline:rate=6", 
		"vf"
#endif
	}; 
	const size_t n_transforms = sizeof(transforms)/sizeof(char*); 

	const char *io[] = {
		"x3dt",
		"bbs"
	}; 
	const size_t n_io = sizeof(io)/sizeof(char*); 

	const PrepareTransIOTests& prep = PrepareTransIOTests::instance(); 

	for (size_t t = 0; t < n_transforms; ++t) {
		auto tc = prep.transform_handler().produce(transforms[t]); 
		auto tr = tc->create(size); 
		auto params = tr->get_parameters(); 
		for (size_t k = 0; k < params.size(); ++k) 
			params[k] = k + 1; 
		tr->set_parameters(params);
		
		for (size_t i = 0; i < n_io; ++i) {
			stringstream fname; 
			fname << transforms[t] << "." << io[i]; 
			cvdebug() << "Test:'" << fname.str() << "'\n"; 
			BOOST_CHECK(prep.transformio_handler().save("", fname.str(), *tr)); 
			
			auto t_loaded = prep.transformio_handler().load(fname.str()); 
			BOOST_CHECK(t_loaded); 
			BOOST_CHECK_EQUAL(t_loaded->get_size(), size); 
			BOOST_CHECK_EQUAL(t_loaded->get_creator_string(), transforms[t]);
			
			auto lparams = t_loaded->get_parameters();
			BOOST_CHECK_EQUAL(lparams.size(), params.size()); 
			for (size_t k = 0; k < lparams.size(); ++k) 
				BOOST_CHECK_EQUAL(lparams[k], k + 1); 
			unlink( fname.str().c_str()); 

		}
	}
}



const PrepareTransIOTests& PrepareTransIOTests::instance()
{
	const static PrepareTransIOTests  me; 
	return me; 
}

const C3DTransformationIOPluginHandler::Instance& PrepareTransIOTests::transformio_handler() const
{
	return C3DTransformationIOPluginHandler::instance(); 
}

const C3DTransformCreatorHandler::Instance& PrepareTransIOTests::transform_handler()const
{
	return C3DTransformCreatorHandler::instance(); 
}

PrepareTransIOTests::PrepareTransIOTests()
{
	list< bfs::path> transformio_plugpath;
	transformio_plugpath.push_back(bfs::path("transio"));
	C3DTransformationIOPluginHandler::set_search_path(transformio_plugpath);
	
	list< bfs::path> transform_searchpath;
	transform_searchpath.push_back(bfs::path("transform"));
	C3DTransformCreatorHandler::set_search_path(transform_searchpath);

}


